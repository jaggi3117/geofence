# 📍 ESP8266 GPS Geofencing Dashboard

This project is a **WiFi-based GPS geofencing system** built using an **ESP8266 microcontroller**, **TinyGPS++**, and a **web-based dashboard**. It tracks a device's GPS location in real time, checks whether the device is within a specified radius from a target location (geofence), and displays live status and metadata via a hosted web page.

---

## 🚀 Features

* 📡 **GPS Tracking** using a GPS module connected via `SoftwareSerial`
* 🌐 **WiFi-based Web Dashboard** using ESP8266 WebServer
* 📍 **Geofencing Logic** using the Haversine formula
* 🔴 Real-time **device status** (inside/outside geofence, online/offline)
* 📊 **Satellite info**, **coordinates**, **timestamp**, and **cumulative angle**
* 🖥️ Web page with **auto-refreshing** dashboard (`/`, `/status`, `/values`)
* 💡 Lightweight and responsive frontend using plain HTML/CSS/JS

---

## 🧰 Hardware Requirements

* ✅ ESP8266 (NodeMCU or similar)
* ✅ GPS Module (e.g., Neo-6M)
* ✅ USB Cable and power source
* ✅ WiFi Network for connectivity

---

## 📁 Project Structure

```
├── main.ino                 # Main Arduino code (ESP8266 logic)
├── index.h                  # HTML Web Interface (served from flash memory)
├── README.md                # This file
```

---

## 📡 How It Works

### 📌 1. **Initialization**

* ESP8266 connects to a predefined WiFi network (SSID/password hardcoded).
* Initializes `TinyGPS++` to read GPS data from the GPS module.

### 📌 2. **Geofencing Logic**

* A geofence is defined by a **target latitude/longitude** and a fixed **radius** (34 meters).
* The system uses the **Haversine formula** to calculate real-world distance between current GPS location and the geofence center.

### 📌 3. **Web Server**

* The ESP8266 acts as an HTTP server (on port 80).
* Serves:

  * `/` → Web Dashboard
  * `/status` → Fence ID (for multiple fence support, currently hardcoded)
  * `/values` → Live GPS + status data (sent in `#`-delimited format)

### 📌 4. **Frontend (index.h)**

* Auto-refreshes every 2 seconds.
* Displays:

  * Satellite count
  * Latitude/Longitude
  * Date/Time
  * Device status (online/offline)
  * Target status (inside/outside geofence)
  * Cumulative angle (if applicable)
* Allows the user to select a geofence (via buttons).

---

## 🧠 Core Code Components

### 🛰️ `updateLatLon()`

* Decodes NMEA sentences from GPS
* Extracts:

  * Latitude & Longitude
  * Altitude, speed, satellite count
  * Date & Time

### 🧭 `haversine(lat1, lon1, lat2, lon2)`

* Calculates distance between current GPS position and geofence center.

### 🛡️ `pip(...)`

* Determines whether the current position is **inside or outside** the geofence.

### 🌍 `handleRoot()`, `fenceSelect()`, `gps_data()`

* Serve the main webpage and API endpoints.

---

## 🧪 Sample Output (Serial Monitor)

```
SATS: 7
LAT: 25.495148
LONG: 81.869165
ALT: 97.23
SPEED: 0.00
Date: 12/7/2025
Hour: 14:23:45
Target Status: 1
---------------------------
```

---

## ⚠️ Notes

* **WiFi credentials** are hardcoded. Update `connectWifi()` as needed.
* Currently only **one geofence is used** at a time (`fence = 1` is hardcoded).
* Page served is stored in **Flash Memory** using `PROGMEM` to conserve SRAM.

