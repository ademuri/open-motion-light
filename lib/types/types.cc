// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "types.h"

#ifndef ARDUINO

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <vector>

static std::array<bool, kPinMax> digital_read_data;
static std::array<bool, kPinMax> digital_write_data;
static std::array<uint32_t, kPinMax> analog_write_data;
static std::array<uint32_t, kPinMax> analog_read_data;
static std::array<uint32_t, kPinMax> pin_mode_data;

static uint32_t millis_ = 0;

MATCHER(PinModeForDigitalRead,
        negation ? " a pin mode that doesn't support digitalRead"
                 : " a pin mode that supports digitalRead") {
  return arg == INPUT || arg == INPUT_PULLUP || arg == INPUT_FLOATING ||
         arg == INPUT_PULLDOWN;
}

MATCHER(PinModeForDigitalWrite,
        negation ? " a pin mode that doesn't support digitalWrite"
                 : " a pin mode that supports digitalWrite") {
  return arg == OUTPUT || arg == OUTPUT_OPEN_DRAIN;
}

MATCHER(PinModeForAnalogWrite,
        negation ? " a pin mode that doesn't support analogWrite"
                 : " a pin mode that supports analogWrite") {
  return arg == OUTPUT || arg == OUTPUT_OPEN_DRAIN;
}

MATCHER(PinModeForAnalogRead,
        negation ? " a pin mode that doesn't support analogRead"
                 : " a pin mode that supports analogRead") {
  return arg == INPUT || arg == INPUT_ANALOG;
}

MATCHER(ValidPinMode, "") {
  static const std::vector<uint32_t> valid_pin_modes = {
      INPUT,        INPUT_PULLUP, INPUT_FLOATING,   INPUT_PULLDOWN,
      INPUT_ANALOG, OUTPUT,       OUTPUT_OPEN_DRAIN};
  return std::find(valid_pin_modes.begin(), valid_pin_modes.end(), arg) !=
         valid_pin_modes.end();
}

int digitalRead(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  EXPECT_THAT(pin_mode_data[ulPin], PinModeForDigitalRead())
      << "for pin " << ulPin;
  return digital_read_data[ulPin];
}

void setDigitalRead(uint32_t ulPin, bool value) {
  ASSERT_LT(ulPin, kPinMax);
  // EXPECT_THAT(pin_mode_data[ulPin], PinModeForDigitalRead()) << "for pin " <<
  // ulPin;
  digital_read_data[ulPin] = value;
}

void digitalWrite(uint32_t ulPin, uint32_t ulVal, bool checkPinMode) {
  ASSERT_LT(ulPin, kPinMax);
  if (checkPinMode) {
    EXPECT_THAT(pin_mode_data[ulPin], PinModeForDigitalWrite())
        << "for pin " << ulPin;
  }
  digital_write_data[ulPin] = ulVal;
}

bool getDigitalWrite(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  // EXPECT_THAT(pin_mode_data[ulPin], PinModeForDigitalWrite()) << "for pin "
  // << ulPin;
  return digital_write_data[ulPin];
}

void analogWrite(uint32_t ulPin, uint32_t ulValue, bool checkPinMode) {
  ASSERT_LT(ulPin, kPinMax);
  if (checkPinMode) {
    EXPECT_THAT(pin_mode_data[ulPin], PinModeForAnalogWrite())
        << "for pin " << ulPin;
  }
  analog_write_data[ulPin] = ulValue;
}

uint32_t getAnalogWrite(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  return analog_write_data[ulPin];
}

void pinMode(uint32_t pin, uint32_t mode) {
  ASSERT_LT(pin, kPinMax);
  EXPECT_THAT(mode, ValidPinMode()) << "for pin " << pin;
  pin_mode_data[pin] = mode;
}

uint32_t getPinMode(uint32_t pin) {
  EXPECT_LT(pin, kPinMax);
  return pin_mode_data[pin];
}

uint32_t analogRead(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  EXPECT_THAT(pin_mode_data[ulPin], PinModeForAnalogRead())
      << "for pin " << ulPin;
  return analog_read_data[ulPin];
}

void setAnalogRead(uint32_t ulPin, uint32_t value) {
  ASSERT_LT(ulPin, kPinMax);
  analog_read_data[ulPin] = value;
}

uint32_t millis() { return millis_; }

void setMillis(uint32_t millis) { millis_ = millis; }

void delay(uint32_t millis) { advanceMillis(millis); }

void advanceMillis(uint32_t millis) { millis_ += millis; }

#endif  // ifndef ARDUINO