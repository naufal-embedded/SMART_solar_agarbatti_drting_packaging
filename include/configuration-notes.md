# Configuration Notes

## Final sensor

This repository uses the SHT31 sensor.

- VIN/VCC -> ESP32 3.3 V
- GND -> ESP32 GND
- SDA -> GPIO 21
- SCL -> GPIO 22
- Common I2C address: 0x44
- Some boards use 0x45; change `sht31.begin(0x44)` if required.

## Before connecting the heater

- Test the firmware with an LED or low-power DC load in place of the heater.
- Confirm relay ON/OFF polarity.
- Confirm the fan MOSFET gate switches correctly.
- Confirm SHT31 readings in Serial Monitor.
- Calibrate the MQ-2 threshold after its warm-up period.
- Set the drying timer only after measuring the real chamber temperature and humidity.
- Use a separate hardware thermostat and thermal fuse for the heater.
