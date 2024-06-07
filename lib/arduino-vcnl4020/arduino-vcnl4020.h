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

#include <Adafruit_VCNL4020.h>
#include <types.h>

#include "vcnl4020.h"

// Implementation of the VCNL4020 HAL on the hardware device.
class ArduinoVCNL4020 : public VCNL4020 {
 public:
  // Initializes the sensor.
  bool Begin() override;

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  void SetLEDCurrent(uint8_t mA) override;

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  uint16_t ReadProximity() override;

  // Reads the 16-bit ambient light value.
  uint16_t ReadAmbient() override;

 private:
  // TODO: modify this library to allow lower-power usage, or write a new one.
  // Additionally, the readProximity and readAmbient calls are blocking, which
  // isn't what we want.
  Adafruit_VCNL4020 sensor_;
};
