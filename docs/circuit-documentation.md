# Circuit Documentation

## Power architecture

```text
12 V Solar Panel
       |
       v
Solar Charge Controller
       |
       +---- 12 V Battery ----> 12 V PTC Heater through relay contacts
       |
       +----> 5 V Buck Converter ----> ESP32 VIN/5 V, relay module, 5 V fan
```

## ESP32 signal connections

| Module | Module pin | ESP32 connection | Notes |
|---|---|---|---|
| SHT31 | VIN/VCC | 3.3 V | Use 3.3 V supply |
| SHT31 | SDA | GPIO 21 | I2C data |
| SHT31 | SCL | GPIO 22 | I2C clock |
| SHT31 | GND | GND | Common ground |
| MQ-2 | VCC | 5 V | Module heater normally needs 5 V |
| MQ-2 | AO | GPIO 34 | ADC-only input; ensure voltage does not exceed ESP32 ADC limit |
| MQ-2 | GND | GND | Common ground |
| Relay | VCC | 5 V | |
| Relay | GND | GND | |
| Relay | IN | GPIO 16 | Active-low assumed |
| Fan MOSFET | Gate | GPIO 17 through 220 Ω | Add 10 kΩ gate pulldown to GND |
| Fan MOSFET | Source | GND | |
| Fan MOSFET | Drain | Fan negative | |
| Fan positive | + | 5 V | |

## Relay power contacts

For a DC heater:

```text
Battery +  ---- fuse ---- COM (relay)
NO (relay) ---------- PTC heater +
PTC heater - -------- Battery -
```

Use `NO` so the heater is OFF when the relay is unpowered.

## Fan MOSFET wiring

```text
5 V supply + -------- Fan +
Fan - --------------- MOSFET Drain
MOSFET Source ------- GND
ESP32 GPIO17 --220 Ω-- MOSFET Gate
Gate -----------------10 kΩ---------------- GND
```

For a brushed DC fan or motor, add a suitable flyback diode across the fan:
- diode cathode to fan positive
- diode anode to fan negative

## Critical electrical checks

1. ESP32 GPIO pins are 3.3 V logic.
2. Never feed 5 V into an ESP32 GPIO.
3. GPIO34 is input-only and is used only for MQ-2 analog input.
4. MQ-2 analog output may exceed the safe ESP32 ADC voltage depending on the module. Use a voltage divider or suitable signal conditioning.
5. All low-voltage modules must share a common ground.
6. Use a fuse and appropriately rated wires for the heater and battery.
7. The relay contact rating must exceed the heater's voltage and current.
8. The PTC heater must have mechanical thermal protection independent of software.
