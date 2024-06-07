/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "types.h"

// USB pins
constexpr int kPinCc1 = PA6;
constexpr int kPinCc2 = PA7;
constexpr int kPinSerialTx = PA9;
constexpr int kPinSerialRx = PA10;

// UI pins
constexpr int kPinBatteryLed1 = PA1;
constexpr int kPinBatteryLed2 = PA2;
constexpr int kPinBatteryLed3 = PA3;
constexpr int kPinWhiteLed = PA0;

// Sensors
constexpr int kPinMotionSensor = PA11;
constexpr int kPinLightSensorInterrupt = PA8;

// I2C - used for light sensor
constexpr int kPinScl = PB6;
constexpr int kPinSda = PB7;

// Power switch
constexpr int kPinPowerAuto = PA4;
constexpr int kPinPowerOn = PA5;

// Battery management
constexpr int kPinBatteryCharge = PC14;
constexpr int kPinBatteryDone = PC15;
constexpr int kPinChargeHighCurrentEnable = PB4;
constexpr int kPin5vDetect = PA15;

// The output of the internal voltage reference for the ADC. Since this is a
// fixed voltage, we can use this to indirectly measure the battery voltage.
constexpr int kPinAdcReference = AVREF;

// This is a pointer to the factory-calibrated reference voltage value for the
// ADC. Since this points to memory, the value is not known at compile time, so
// it can't be constexpr. The typical reference voltage is 1.224v.
extern const uint16_t *const kAdcReferencePointer;
