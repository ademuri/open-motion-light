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

#include <types.h>

// HAL for the VCNL4020 ambient light and proximity sensor.
class VCNL4020 {
 public:
  // Initializes the sensor.
  virtual bool Begin() = 0;

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  virtual void SetLEDCurrent(uint8_t mA) = 0;

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  virtual uint16_t ReadProximity() = 0;

  // Enables or disables periodic ambient light measurements.
  virtual void SetPeriodicAmbient(bool enable);

  // Returns true when the ambient measurement is ready. Reset by calls to
  // ReadAmbient.
  virtual bool AmbientReady() = 0;

  // Reads the 16-bit ambient light value.
  virtual uint16_t ReadAmbient() = 0;
};
