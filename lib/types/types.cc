#include "types.h"

#ifndef ARDUINO

#include <gtest/gtest.h>

#include <array>

static std::array<bool, kPinMax> digital_read_data;
static std::array<uint32_t, kPinMax> analog_write_data;
static std::array<uint32_t, kPinMax> analog_read_data;

static uint32_t millis_ = 0;

int digitalRead(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  return digital_read_data[ulPin];
}

void setDigitalRead(uint32_t ulPin, bool value) {
  ASSERT_LT(ulPin, kPinMax);
  digital_read_data[ulPin] = value;
}

void analogWrite(uint32_t ulPin, uint32_t ulValue) {
  ASSERT_LT(ulPin, kPinMax);
  analog_write_data[ulPin] = ulValue;
}

uint32_t getAnalogWrite(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  return analog_write_data[ulPin];
}

uint32_t analogRead(uint32_t ulPin) {
  EXPECT_LT(ulPin, kPinMax);
  return analog_read_data[ulPin];

}

void setAnalogRead(uint32_t ulPin, uint32_t value) {
  ASSERT_LT(ulPin, kPinMax);
  analog_read_data[ulPin] = value;
}

uint32_t millis() { return millis_; }

void setMillis(uint32_t millis) { millis_ = millis; }

void advanceMillis(uint32_t millis) { millis_ += millis; }

#endif  // ifndef ARDUINO