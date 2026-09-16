# Smart Solar-Powered Agarbatti Drying & Packaging System

SIH project: an ESP32-based smart drying chamber for home-based agarbatti manufacturing.

## Features

- Temperature and humidity monitoring using DHT22
- Independent temperature monitoring using DS18B20
- Smoke/gas warning using MQ-2 analog output
- Automatic PTC heater control through a 5 V relay module
- Automatic 5 V fan control through an N-channel MOSFET
- Drying temperature target: 40–50 °C
- Over-temperature protection
- Sensor-failure shutdown
- Serial-monitor status and fault messages
- Expandable for solar charging, battery monitoring, packaging and IoT dashboards

> **Safety:** This repository is a prototype control system. Use a correctly rated fuse, enclosure, wiring, flyback protection where required, common ground, and an appropriately rated relay/MOSFET. Do not connect mains AC directly to the ESP32 or breadboard. A qualified person must verify heater power wiring.

## Hardware

- ESP32 development board
- SHT31 temperature/humidity sensor
- DS18B20 temperature sensor
- 4.7 kΩ resistor for DS18B20 data pull-up
- MQ-2 sensor module
- 1-channel 5 V relay module
- 5 V DC fan
- Logic-level N-channel MOSFET, e.g. IRLZ44N
- 220 Ω gate resistor
- 10 kΩ gate pulldown resistor
- 12 V/24 V PTC heater according to the selected power system
- 12 V solar panel, charge controller and battery
- 5 V buck converter for ESP32 peripherals

## Pin mapping

| Device | Pin | ESP32 GPIO |
|---|---|---:|
| SHT31 | SDA | GPIO 21 |
| SHT31 | SCL | GPIO 22 |
| MQ-2 | AO | GPIO 34 |

| Heater relay | IN | GPIO 16 |
| Fan MOSFET | Gate | GPIO 17 |
| I2C SHT31 (optional future) | SDA | GPIO 21 |
| I2C SHT31 (optional future) | SCL | GPIO 22 |

The uploaded hand-drawn diagram contains a few ambiguous wire crossings. This repository uses the explicit pin table above as the source of truth. The final sensor choice here is SHT31.

## Control logic

1. Start with heater OFF and fan ON for ventilation.
2. Read DHT22, DS18B20 and MQ-2.
3. If a sensor fails, over-temperature occurs, or gas level exceeds the threshold:
   - Turn heater OFF
   - Turn fan ON
   - Report the fault
4. During normal drying:
   - Heater turns ON below the lower temperature limit.
   - Heater turns OFF at the upper temperature limit.
   - Fan remains ON for airflow.
5. After the configured drying time, heater turns OFF and the fan continues for the cooldown period.

## Arduino IDE setup

1. Install ESP32 board support in Arduino IDE.
2. Install these libraries:
   - Adafruit SHT31
   - Adafruit BusIO
3. Open `src/agarbatti_dryer_esp32.ino`.
4. Select your ESP32 board and port.
5. Upload and open Serial Monitor at **115200 baud**.

## Important configuration

Edit these constants in the `.ino` file:

- `LOWER_TEMP_C`
- `UPPER_TEMP_C`
- `MAX_SAFE_TEMP_C`
- `MQ2_ALARM_THRESHOLD`
- `DRYING_TIME_MS`
- `COOLDOWN_TIME_MS`

The MQ-2 threshold must be calibrated on the actual sensor and environment. The MQ-2 module is not a certified fire or gas safety device.

## Repository structure

```text
smart-solar-agarbatti-drying-packaging/
├── README.md
├── LICENSE
├── .gitignore
├── platformio.ini
├── src/
│   └── agarbatti_dryer_esp32.ino
├── docs/
│   ├── circuit-documentation.md
│   └── system-flow.md
├── hardware/
│   ├── pin-connections.csv
│   └── bill-of-materials.csv
└── include/
    └── configuration-notes.md
```

## Future extensions

- SHT31 is used as the final temperature/humidity sensor
- OLED/LCD display
- Battery voltage sensing
- Solar charging status
- Blynk/MQTT dashboard
- Door-open sensor
- Load-cell based packaging counter
- Automatic sealing and batch logging
