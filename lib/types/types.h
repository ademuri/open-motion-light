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

#include <api/Common.h>

#ifdef ARDUINO

#include <Arduino.h>
#include <EEPROM.h>
#include <stm32l0xx_ll_adc.h>

#else  // ifdef ARDUINO

// Types needed for native code - i.e. tests

#include <cstdint>

#include "fake-eeprom.h"
#include "fake-pins.h"

PinStatus digitalRead(uint32_t ulPin);
void setDigitalRead(uint32_t ulPin, bool value);
void setDigitalRead(uint32_t ulPin, PinStatus value);

void digitalWrite(uint32_t ulPin, bool val);
void digitalWrite(uint32_t ulPin, uint32_t ulVal, bool checkPinMode);
bool getDigitalWrite(uint32_t ulPin);

void analogWrite(uint32_t ulPin, uint32_t ulValue, bool checkPinMode);
uint32_t getAnalogWrite(uint32_t ulPin);

void pinMode(uint32_t pin, uint32_t mode);
uint32_t getPinMode(uint32_t pin);

int analogRead(uint32_t ulPin);
void setAnalogRead(uint32_t ulPin, int value);

unsigned long millis();
void setMillis(unsigned long millis);
void delay(unsigned long millis);

void advanceMillis(unsigned long millis);

#endif  // ifdef ARDUINO