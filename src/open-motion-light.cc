#include <Arduino.h>

// USB pins
constexpr int kPinCc1 = PA0;
constexpr int kPinCc2 = PA1;
constexpr int kPinSerialTx = PA9;
constexpr int kPinSerialRx = PA10;

// UI pins
constexpr int kPinBatteryLed1 = PA2;
constexpr int kPinBatteryLed2 = PA3;
constexpr int kPinBatteryLed3 = PA5;
constexpr int kPinSw1 = PB3;
constexpr int kPinWhiteLed = PA15;

// Sensors
constexpr int kPinMotionSensor = PA8;
constexpr int kPinLightSensorInterrupt = PA7;

// I2C - used for light sensor
constexpr int kPinScl = PB6;
constexpr int kPinSda = PB7;

// Power switch
constexpr int kPinPowerAuto = PA4;
constexpr int kPinPowerOn = PA6;

// Battery management
constexpr int kPinBatteryCharge = PC14;
constexpr int kPinBatteryDone = PC15;
constexpr int kPinChargeHighCurrentEnable = PB1;

void setup() {
  // USB pins
  pinMode(kPinCc1, INPUT_ANALOG);
  pinMode(kPinCc2, INPUT_ANALOG);
  pinMode(kPinSerialTx, OUTPUT);
  pinMode(kPinSerialRx, INPUT);

  // UI pins
  pinMode(kPinBatteryLed1, OUTPUT);
  pinMode(kPinBatteryLed2, OUTPUT);
  pinMode(kPinBatteryLed3, OUTPUT);
  pinMode(kPinSw1, INPUT_PULLUP);
  pinMode(kPinWhiteLed, OUTPUT);

  // Sensors
  pinMode(kPinMotionSensor, INPUT);
  pinMode(kPinLightSensorInterrupt, INPUT);

  // I2C - used for light sensor

  // Power switch
  pinMode(kPinPowerAuto, INPUT);
  pinMode(kPinPowerOn, INPUT);

  // Battery management
  pinMode(kPinBatteryCharge, INPUT);
  pinMode(kPinBatteryDone, INPUT);
  pinMode(kPinChargeHighCurrentEnable, OUTPUT);
}

void loop() {}
