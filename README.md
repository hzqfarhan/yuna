<p align="center">
  <a href="https://seladevs.my">
    <img src="https://www.seladevs.my/branding/seladevs-logo.png" width="110" alt="SELADEVS Logo" />
  </a>
</p>

<h1 align="center">🐾 YUNA — Quadruped Robot</h1>

<p align="center">
  <strong>ESP32-Powered 8-Servo Quadruped Robot with WiFi Web Controller</strong><br>
  Developed & Customized by <a href="https://seladevs.my"><strong>SELADEVS</strong></a> · Collective of Developers at UTHM
</p>

<p align="center">
  <img src="https://img.shields.io/badge/SELADEVS-Collective_09-ff2a2a?style=for-the-badge&logo=codeforces&logoColor=white" alt="SELADEVS" />
  <img src="https://img.shields.io/badge/Platform-ESP32-red?style=for-the-badge&logo=espressif" alt="ESP32" />
  <img src="https://img.shields.io/badge/Skills-20%20Actions-critical?style=for-the-badge" alt="Skills" />
  <img src="https://img.shields.io/badge/License-MIT-blue?style=for-the-badge" alt="License" />
</p>

---

## ⚡ Overview

**Yuna** is an 8-degree-of-freedom (8-DOF) quadruped robotic dog built on an **ESP32** microcontroller. It hosts an embedded, mobile-responsive Web Server running a custom **Cyberpunk Red** dashboard that allows real-time wireless control over WiFi without needing any mobile app installed.

With 20 pre-programmed animated skills and gait algorithms based on sinusoidal oscillators and linear interpolation, Yuna can walk, dance, bow, jump, shake hands, and perform expressive lifelike movements.

---

## ✨ Features

- 🎮 **20 Animated Skills & Poses**:
  - **Locomotion**: Forward, Backward, Turn Left, Turn Right, Stand/Home, OmniWalk, Moonwalk Left, Moonwalk Right.
  - **Tricks & Expressions**: Hello, Wave Hand, Shake Hand, Cheer, Boxing, Dance, Twist / Wiggle, UpDown, PushUp, FrontBack, Stretch / Bow, TipToe, Jump / Bounce, Sit, Scared, Pee, Hide / Sleep.
- 🔴 **Cyberpunk Red Web Dashboard**:
  - Integrated red & obsidian theme with SELADEVS branding.
  - Touch-friendly D-Pad navigation for mobile and desktop screens.
  - Live terminal telemetry showing executed commands in real time.
- 📡 **Dual WiFi Modes (`Config.h`)**:
  - **Access Point (AP) Mode** *(Default)*: Broadcasts a dedicated standalone hotspot `yuna` (`192.168.4.1`) — no external router needed.
  - **Station (STA) Mode**: Connects directly to your local 2.4 GHz WiFi network with fixed IP addressing.
- ⚙️ **Smooth Kinematics**:
  - Driven by `ESP32Servo` and `Octosnake` sinusoidal phase-offset oscillators for natural walking and fluid gait cycles.

---

## 📐 Pinout & Servo Configuration

Yuna utilizes **8 micro servos** (SG90 / MG90S) connected to the ESP32 GPIO pins:

```
                  前 (Head / Front)
              +-------+       +-------+
              |   2   |       |   3   |   (Front Knees / Feet)
              | GPIO25|       | GPIO18|
              +-------+-------+-------+
                  |   0   | |   1   |     (Front Hips)
                  | GPIO12| | GPIO16|
                  +-------+ +-------+
                  |   4   | |   5   |     (Rear Hips)
                  | GPIO13| | GPIO17|
              +-------+-------+-------+
              |   6   |       |   7   |   (Rear Knees / Feet)
              | GPIO26|       | GPIO19|
              +-------+       +-------+
                  后 (Tail / Rear)
```

| Servo ID | Position | Function | ESP32 GPIO |
|---|---|---|---|
| **0** | Front-Left (FLH) | Hip (Horizontal) | **GPIO 12** |
| **1** | Front-Right (FRH) | Hip (Horizontal) | **GPIO 16** |
| **2** | Front-Left (FLF) | Knee / Foot (Vertical) | **GPIO 25** |
| **3** | Front-Right (FRF) | Knee / Foot (Vertical) | **GPIO 18** |
| **4** | Back-Left (BLH) | Hip (Horizontal) | **GPIO 13** |
| **5** | Back-Right (BRH) | Hip (Horizontal) | **GPIO 17** |
| **6** | Back-Left (BLF) | Knee / Foot (Vertical) | **GPIO 26** |
| **7** | Back-Right (BRF) | Knee / Foot (Vertical) | **GPIO 19** |

> 📷 For a visual diagram, see [`images/quad_接线图.jpg`](images/quad_接线图.jpg).

---

## 🚀 Getting Started

### 1. Requirements

* **Hardware**:
  * ESP32 Development Board (e.g., ESP32 Dev Module, NodeMCU-32S, ESP-WROOM-32)
  * 8× SG90 or MG90S 9g Micro Servos
  * External 5V / 2A–3A Power Supply (e.g., 2S LiPo with buck regulator or 18650 battery pack)
  * USB cable for flashing
* **Software**:
  * [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.19 or 2.x)
  * **ESP32 Board Package** installed via Boards Manager (`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)

### 2. Install Arduino Libraries

Search and install the following libraries inside Arduino IDE (**Sketch > Include Library > Manage Libraries...**):
1. **ESP32Servo** (by Kevin Harrington)
2. **ArduinoJson** (by Benoît Blanchon)
3. **Bluepad32** (by Ricardo Quesada — required for PS5 controller support)

### 3. Flash Configuration

1. Connect your ESP32 via USB and open `yuna/yuna.ino`.
2. Select your board under **Tools > Board > ESP32 Arduino > ESP32 Dev Module**.
3. Select your **Port** (e.g. `COM3` on Windows).
4. Set **Tools > Partition Scheme** to **"Huge APP (3MB No OTA/1MB SPIFFS)"** *(Essential to ensure sufficient program memory)*.
5. Click **Upload**.

---

## 🌐 Network Configuration

Edit [`yuna/Config.h`](yuna/Config.h) before uploading if you wish to adjust network settings:

### Hotspot (AP) Mode (Default)
```cpp
const bool USE_AP_MODE = true;
const char *ap_ssid = "yuna";
const char *ap_password = "88889999";
IPAddress ap_local_ip(192, 168, 4, 1);
```
1. Connect your phone or computer to the WiFi hotspot named **`yuna`** (Password: `88889999`).
2. Open your browser and navigate to:
   ```
   http://192.168.4.1
   ```

### Router (STA) Mode
```cpp
const bool USE_AP_MODE = false;
const char *ssid = "Your_WiFi_SSID";
const char *password = "Your_WiFi_Password";
IPAddress local_ip(192, 168, 1, 192); // Set according to your subnet
```
1. Power on the ESP32; it will connect to your router.
2. Open your browser and visit the static IP configured (e.g. `http://192.168.1.192`).

---

## 📂 Project Structure

```
yuna/
├── yuna/
│   ├── yuna.ino          # Main Arduino firmware & HTTP web server
│   ├── Config.h          # WiFi credentials & PS5 controller configuration
│   ├── PS5Controller.h   # PS5 DualSense controller Bluetooth handler (Bluepad32)
│   ├── WiFiSetup.h       # AP & STA mode connection manager
│   ├── minikame.h        # Quadruped kinematic classes & skill definitions
│   ├── minikame.cpp      # Gait routines & 20 animated action implementations
│   ├── Octosnake.h       # Sinusoidal phase oscillator library
│   └── Octosnake.cpp     # Oscillator implementation
├── quad-arduino-wifi/    # Reference Arduino WiFi implementation
├── quad-mpy/             # Reference MicroPython implementation
├── index.html            # Red-themed web controller dashboard
├── convert_html.py       # Utility to compile index.html into Arduino C++ string
├── images/               # Wiring schematics and visual diagrams
├── README.md             # Documentation
└── LICENSE               # MIT License
```

---

## 👥 About SELADEVS

**SELADEVS** is a developer collective at **Universiti Tun Hussein Onn Malaysia (UTHM)**. We engineer innovative web applications, software as a service, hardware interfaces, and merchandise.

- 🌐 **Website**: [seladevs.my](https://seladevs.my)
- 💻 **GitHub**: [@hzqfarhan](https://github.com/hzqfarhan)
- 📍 **Location**: UTHM, Johor, Malaysia

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
