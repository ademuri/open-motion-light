#pragma once

#include "types.h"

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

// The output of the internal voltage reference for the ADC. Since this is a
// fixed voltage, we can use this to indirectly measure the battery voltage.
constexpr int kPinAdcReference = AVREF;

// This is a pointer to the factory-calibrated reference voltage value for the
// ADC. Since this points to memory, the value is not known at compile time, so
// it can't be constexpr. The typical reference voltage is 1.224v.
extern const uint16_t *const kAdcReferencePointer;
