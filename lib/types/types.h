#pragma once

#ifdef ARDUINO

#include <Arduino.h>

#else  // ifdef ARDUINO

// Types needed for native code - i.e. tests

#include <cstdint>

#include "fake-pins.h"

int digitalRead(uint32_t ulPin);
void setDigitalRead(uint32_t ulPin, bool value);

uint32_t millis();
void setMillis(uint32_t millis);
void advanceMillis(uint32_t millis);

#endif  // ifdef ARDUINO