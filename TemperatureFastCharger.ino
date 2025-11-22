/*
  TemperatureFastCharger.ino
  Firmware for NodeMCU ESP8266 (Arduino) - Temperature-responsive fast charger

  Features:
  - Reads DHT11 sensor (D5)
  - Controls Fast Charge relay and Fan relay with temperature thresholds
  - Displays temperature & mode on I2C LCD
  - Connects to Wi-Fi and serves a simple HTTP JSON status endpoint
  - Non-blocking timing using millis()

  Pins (NodeMCU naming):
  - D5 (GPIO14) : DHT11 data
  - D0 (GPIO16) : FAST_CHARGE_RELAY (active LOW to enable)
  - D4 (GPIO2)  : FAN_RELAY (active LOW to enable)
  - I2C SDA/SCL : LCD (default NodeMCU pins: D2/D1 are typical; depends on your wiring)

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char SSID[] = "Microworld";        
const char PASSWORD[] = "Microworld123"; 

// Temperature thresholds 
const float FAN_ON_TEMP = 32.0;   
const float SLOW_CHARGE_TEMP = 35.0; 

// Sampling interval (ms)
const unsigned long SENSOR_INTERVAL_MS = 2000UL;
const unsigned long LCD_REFRESH_MS = 1500UL;

// Pins (NodeMCU pin names)
#define DHTPIN D5            // DHT11 data
#define DHTTYPE DHT11

#define FAST_RELAY_PIN D0    // Relay that enables fast charging (active LOW to enable)
#define FAN_RELAY_PIN D4     // Relay that controls fan (active LOW to enable)

// I2C LCD address (common addresses are 0x27 or 0x3F)
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// ----------------- Global objects -----------------
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
ESP8266WebServer server(80);

// State variables
float temperatureC = NAN;
float humidityPerc = NAN;
unsigned long lastSensorMillis = 0;
unsigned long lastLcdMillis = 0;
String chargingMode = "Unknown";
bool fanOn = false;

// ----------------- Helper functions -----------------

void updateRelaysAndMode(float temp) {
  // Fan logic
  if (!isnan(temp) && temp >= FAN_ON_TEMP) {
    // Activate fan (active LOW)
    digitalWrite(FAN_RELAY_PIN, LOW);
    fanOn = true;
  } else {
    digitalWrite(FAN_RELAY_PIN, HIGH);
    fanOn = false;
  }

  // Charging mode logic
  if (!isnan(temp) && temp >= SLOW_CHARGE_TEMP) {
    // Switch to slow charging - disable fast relay (active LOW)
    digitalWrite(FAST_RELAY_PIN, LOW); 
    chargingMode = "Slow Charging";
  } else {
    digitalWrite(FAST_RELAY_PIN, HIGH); 
    chargingMode = "Fast Charging";
  }
}

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius

  if (isnan(h) || isnan(t)) {
    Serial.println("[WARN] Failed to read from DHT sensor");
 
    return;
  }

  humidityPerc = h;
  temperatureC = t;
  Serial.printf("[SENSOR] Temp: %.2f C, Humidity: %.2f %%\n", temperatureC, humidityPerc);

  updateRelaysAndMode(temperatureC);
}

void handleRoot() {
 
  server.sendHeader("Location", "/status");
  server.send(302, "text/plain", "");
}

void handleStatus() {
  String json = "{";
  json += "\"temperature_C\":" + String(isnan(temperatureC) ? -999.0 : temperatureC, 2) + ",";
  json += "\"humidity_percent\":" + String(isnan(humidityPerc) ? -999.0 : humidityPerc, 2) + ",";
  json += "\"charging_mode\":\"" + chargingMode + "\",";
  json += "\"fan_on\":" + String(fanOn ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.onNotFound([](){ server.send(404, "text/plain", "Not Found"); });
  server.begin();
  Serial.println("[INFO] HTTP server started");
}

void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature base");
  lcd.setCursor(0,1);
  lcd.print("Mobile Fast");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Charger &");
  lcd.setCursor(0,1);
  lcd.print("Mobile Cooler");
  delay(1500);
  lcd.clear();
}

void setup() {
  Serial.begin(115200);
  delay(100); // Allow USB-Serial to initialize

 
  pinMode(FAST_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  
  digitalWrite(FAST_RELAY_PIN, HIGH);
  digitalWrite(FAN_RELAY_PIN, HIGH);

  // Initialize DHT and LCD
  dht.begin();
  Wire.begin();        
  lcd.init();
  lcd.backlight();

  displayWelcome();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("[INFO] Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000UL) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.printf("[INFO] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println();
    Serial.println("[WARN] WiFi connect timed out. Continuing without network.");
  }

  setupServer();

  // Initial sensor read
  readSensor();
  lastSensorMillis = millis();
  lastLcdMillis = millis();
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isnan(temperatureC)) {
    lcd.print("Temp = --.- C");
  } else {
    lcd.print("Temp = ");
    lcd.print(String(temperatureC, 1));
    lcd.print(" C");
  }
  lcd.setCursor(0, 1);
  lcd.print(chargingMode);
 
  if (fanOn) {
    lcd.setCursor(13, 1);
    lcd.print("F");
  }
}

void loop() {
  unsigned long now = millis();

  server.handleClient();


  if (now - lastSensorMillis >= SENSOR_INTERVAL_MS) {
    readSensor();
    lastSensorMillis = now;
  }
  if (now - lastLcdMillis >= LCD_REFRESH_MS) {
    updateLCD();
    lastLcdMillis = now;
  }
}
