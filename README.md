# ⚡ Temperature-Responsive Fast Charging System  
### Intelligent Thermal-Aware Mobile Charging | NodeMCU ESP8266 | Arduino/C++ | Embedded Systems | IoT

This project implements a **temperature-controlled fast-charging system** using the **NodeMCU ESP8266**, designed to protect mobile batteries from thermal damage during fast charging.  
The system automatically switches between **Fast Charging**, **Slow Charging**, and **Cooling Mode** based on real-time temperature readings from a DHT11 sensor.

---

## 🚀 Features

- 🌡️ **Real-time temperature monitoring** using DHT11  
- ⚡ **Fast and slow charging modes** (relay-controlled)  
- ❄️ **Automatic cooling fan activation** above threshold temperature  
- 📟 **16x2 I2C LCD display** for temperature + charging status  
- 🌐 **Wi-Fi enabled (ESP8266)** with HTTP status endpoint  
- 🔁 **Non-blocking embedded control loop using `millis()`**  
- 🧩 Written entirely in **Arduino/C++ (Embedded C style)**  
- 🔌 Ideal for embedded systems, IoT, and hardware-software integration projects  

---

## 🛠️ Hardware Components

| Component | Description |
|----------|-------------|
| NodeMCU ESP8266 | Main microcontroller + WiFi |
| DHT11 Sensor | Temperature & humidity input |
| Relay Module (2-channel) | Controls fast charge & fan |
| Cooling Fan | Activated when device overheats |
| I2C LCD (16x2) | Displays temperature + mode |
| Buck-Boost Converter | Provides stable 5V supply |
| Mobile Charging Circuit | Fast/slow charging switching |




