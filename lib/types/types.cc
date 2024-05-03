#include "types.h"

#ifndef ARDUINO

#include <gtest/gtest.h>

#include <array>

static constexpr uint32_t kDigitalReadMax = PC15;
static std::array<bool, kDigitalReadMax> digital_read_data;

static uint32_t millis_ = 0;

int digitalRead(uint32_t ulPin) {
  EXPECT_LE(ulPin, kDigitalReadMax);
  return digital_read_data[ulPin];
}

void setDigitalRead(uint32_t ulPin, bool value) {
  ASSERT_LE(ulPin, kDigitalReadMax);
  digital_read_data[ulPin] = value;
}

uint32_t millis() { return millis_; }

void setMillis(uint32_t millis) { millis_ = millis; }

void advanceMillis(uint32_t millis) { millis_ += millis; }

#endif  // ifndef ARDUINO