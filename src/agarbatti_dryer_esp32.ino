/*
  Smart Solar-Powered Agarbatti Drying & Packaging System
  Controller: ESP32 Dev Module

  Sensor:
    SHT31 temperature + humidity sensor
    SDA -> GPIO 21
    SCL -> GPIO 22

  Other sensor:
    MQ-2 AO -> GPIO 34

  Actuators:
    Heater relay IN -> GPIO 16
    Fan MOSFET gate -> GPIO 17

  Required libraries:
    Adafruit SHT31
    Adafruit BusIO
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

// ---------------- Pin definitions ----------------
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t MQ2_PIN = 34;
constexpr uint8_t HEATER_RELAY_PIN = 16;
constexpr uint8_t FAN_MOSFET_PIN = 17;

// ---------------- Configuration ----------------
constexpr bool RELAY_ACTIVE_LOW = true;

constexpr float LOWER_TEMP_C = 40.0;
constexpr float UPPER_TEMP_C = 50.0;
constexpr float MAX_SAFE_TEMP_C = 60.0;

constexpr int MQ2_ALARM_THRESHOLD = 1000;

// Bench-test values. Change only after real chamber validation.
constexpr unsigned long DRYING_TIME_MS = 10UL * 60UL * 1000UL;
constexpr unsigned long COOLDOWN_TIME_MS = 2UL * 60UL * 1000UL;

constexpr unsigned long SENSOR_INTERVAL_MS = 2000;
constexpr unsigned long STATUS_INTERVAL_MS = 5000;

// ---------------- Objects ----------------
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ---------------- State ----------------
enum SystemState {
  IDLE,
  DRYING,
  COOLDOWN,
  FAULT
};

SystemState state = IDLE;
unsigned long stateStartedAt = 0;
unsigned long lastSensorRead = 0;
unsigned long lastStatusPrint = 0;

float humidity = NAN;
float temperatureC = NAN;
int mq2Value = 0;
String faultReason = "None";

// ---------------- Helpers ----------------
void setHeater(bool on) {
  digitalWrite(
    HEATER_RELAY_PIN,
    RELAY_ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW)
  );
}

void setFan(bool on) {
  digitalWrite(FAN_MOSFET_PIN, on ? HIGH : LOW);
}

void enterState(SystemState newState, const String &reason = "") {
  state = newState;
  stateStartedAt = millis();

  if (reason.length() > 0) {
    faultReason = reason;
  }

  if (newState == FAULT) {
    setHeater(false);
    setFan(true);
  } else if (newState == IDLE) {
    setHeater(false);
    setFan(false);
  } else if (newState == DRYING) {
    setFan(true);
  } else if (newState == COOLDOWN) {
    setHeater(false);
    setFan(true);
  }
}

bool sensorValuesValid() {
  return !isnan(temperatureC) &&
         !isnan(humidity) &&
         temperatureC > -40.0 &&
         temperatureC < 125.0 &&
         humidity >= 0.0 &&
         humidity <= 100.0;
}

void readSensors() {
  temperatureC = sht31.readTemperature();
  humidity = sht31.readHumidity();
  mq2Value = analogRead(MQ2_PIN);
}

void printStatus() {
  Serial.println(F("\n----- AGARBATTI DRYER STATUS -----"));
  Serial.print(F("State: "));

  switch (state) {
    case IDLE: Serial.println(F("IDLE")); break;
    case DRYING: Serial.println(F("DRYING")); break;
    case COOLDOWN: Serial.println(F("COOLDOWN")); break;
    case FAULT: Serial.println(F("FAULT")); break;
  }

  Serial.print(F("SHT31 temperature: "));
  Serial.print(temperatureC);
  Serial.println(F(" C"));

  Serial.print(F("SHT31 humidity: "));
  Serial.print(humidity);
  Serial.println(F(" %"));

  Serial.print(F("MQ-2 ADC: "));
  Serial.println(mq2Value);

  Serial.print(F("Fault: "));
  Serial.println(faultReason);
  Serial.println(F("----------------------------------"));
}

void safetyCheck() {
  if (!sensorValuesValid()) {
    enterState(FAULT, "SHT31 sensor failure");
    return;
  }

  if (temperatureC >= MAX_SAFE_TEMP_C) {
    enterState(FAULT, "Over-temperature");
    return;
  }

  if (mq2Value >= MQ2_ALARM_THRESHOLD) {
    enterState(FAULT, "MQ-2 threshold exceeded");
    return;
  }
}

void controlDrying() {
  if (temperatureC < LOWER_TEMP_C) {
    setHeater(true);
  } else if (temperatureC >= UPPER_TEMP_C) {
    setHeater(false);
  }

  setFan(true);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(HEATER_RELAY_PIN, OUTPUT);
  pinMode(FAN_MOSFET_PIN, OUTPUT);
  pinMode(MQ2_PIN, INPUT);

  setHeater(false);
  setFan(false);

  analogReadResolution(12);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!sht31.begin(0x44)) {
    Serial.println(F("ERROR: SHT31 not found at address 0x44."));
    enterState(FAULT, "SHT31 not detected");
  } else {
    Serial.println(F("SHT31 detected successfully."));
    enterState(IDLE);
  }

  Serial.println(F("Smart Solar-Powered Agarbatti Dryer started."));
  Serial.println(F("Send 's' for status, 'd' to start drying, 'x' to stop, 'r' to reset."));
}

void loop() {
  unsigned long now = millis();

  if (Serial.available()) {
    char command = Serial.read();

    if (command == 's' || command == 'S') {
      printStatus();
    } else if (command == 'd' || command == 'D') {
      if (state != FAULT) {
        faultReason = "None";
        enterState(DRYING);
        Serial.println(F("Drying cycle started."));
      } else {
        Serial.println(F("Reset fault first using 'r'."));
      }
    } else if (command == 'x' || command == 'X') {
      enterState(IDLE);
      Serial.println(F("System stopped."));
    } else if (command == 'r' || command == 'R') {
      faultReason = "None";
      enterState(IDLE);
      Serial.println(F("Fault reset. System is idle."));
    }
  }

  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    readSensors();
    safetyCheck();
  }

  if (state == DRYING) {
    if (now - stateStartedAt >= DRYING_TIME_MS) {
      enterState(COOLDOWN);
      Serial.println(F("Drying complete. Cooldown started."));
    } else if (state != FAULT) {
      controlDrying();
    }
  } else if (state == COOLDOWN) {
    setHeater(false);
    setFan(true);

    if (now - stateStartedAt >= COOLDOWN_TIME_MS) {
      enterState(IDLE);
      Serial.println(F("Cooldown complete. Cycle finished."));
    }
  } else if (state == IDLE) {
    setHeater(false);
    setFan(false);
  } else if (state == FAULT) {
    setHeater(false);
    setFan(true);
  }

  if (now - lastStatusPrint >= STATUS_INTERVAL_MS) {
    lastStatusPrint = now;
    printStatus();
  }
}
