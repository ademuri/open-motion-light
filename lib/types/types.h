#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include <stm32l0xx_ll_adc.h>

#else  // ifdef ARDUINO

// Types needed for native code - i.e. tests

#include <cstdint>

#include "fake-pins.h"

int digitalRead(uint32_t ulPin);
void setDigitalRead(uint32_t ulPin, bool value);

void analogWrite(uint32_t ulPin, uint32_t ulValue);
uint32_t getAnalogWrite(uint32_t ulPin);

uint32_t analogRead(uint32_t ulPin);
void setAnalogRead(uint32_t ulPin, uint32_t value);

uint32_t millis();
void setMillis(uint32_t millis);
void advanceMillis(uint32_t millis);

#endif  // ifdef ARDUINO