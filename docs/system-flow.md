# System Flow

```text
POWER ON
   |
Initialize ESP32, sensors, relay and fan
   |
Read DHT22 + DS18B20 + MQ-2
   |
Safety check
   |
   +-- Sensor fault / over-temperature / gas threshold?
   |             |
   |             +---- YES ---> Heater OFF + Fan ON + FAULT
   |                              |
   |                              +-- Reset command ---> IDLE
   |
   +---- NO ---> IDLE
                    |
             Start drying command
                    |
             Fan ON + temperature control
                    |
       Temperature below 40 °C?
          |                 |
         YES               NO
          |                 |
      Heater ON       Temperature >= 50 °C?
                            |          |
                           YES        NO
                            |          |
                        Heater OFF   Keep state
                            |
                   Drying time completed?
                            |
                           YES
                            |
                     Heater OFF + Fan ON
                            |
                       Cooldown done
                            |
                       Cycle finished
