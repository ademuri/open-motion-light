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

#include "vcnl4020.h"

// Implementation of the VCNL4020 HAL on the hardware device.
class ArduinoVCNL4020 : public VCNL4020 {
 public:
  // Initializes the sensor.
  bool Begin() override;

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  void SetLEDCurrent(uint8_t mA) override;

  void SetPeriodicAmbient(bool enable) override;

  void SetPeriodicProximity(bool enable) override;

  // Returns true when the ambient measurement is ready. Reset by calls to
  // ReadAmbient.
  bool AmbientReady() override;

  // Reads the 16-bit ambient light value.
  uint16_t ReadAmbient() override;

  bool ProximityReady() override;

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  uint16_t ReadProximity() override;

  uint8_t ReadStatus();

 private:
  uint8_t ReadByte(uint8_t register_address);
  uint8_t WriteByte(uint8_t register_address, uint8_t data);

  uint8_t command_ = 0;

  // Continuous conversion mode disabled
  // ALS measurement rate 1 sample/second
  // Auto offset compensation enabled
  // Average function set to 4 measurements
  static constexpr uint8_t als_parameter_ = 0b00001010;

  static constexpr uint8_t kDeviceAddress = 0x13;

  static constexpr uint8_t kRegCommand = 0x80;
  static constexpr uint8_t kRegProductId = 0x81;
  static constexpr uint8_t kRegProxRate = 0x82;
  static constexpr uint8_t kRegProxCurrent = 0x83;
  static constexpr uint8_t kRegAlsParameter = 0x84;
  static constexpr uint8_t kRegAlsResultHigh = 0x85;
  static constexpr uint8_t kRegAlsResultLow = 0x86;
  static constexpr uint8_t kRegProxResultHigh = 0x87;
  static constexpr uint8_t kRegProxResultLow = 0x88;

  static constexpr uint8_t kCommandAlsDataReady = 0b1000000;
  static constexpr uint8_t kCommandProxDataReady = 0b100000;
  static constexpr uint8_t kCommandAlsOnDemand = 0b10000;
  static constexpr uint8_t kCommandProxOnDemand = 0b1000;
  static constexpr uint8_t kCommandAlsEnable = 0b100;
  static constexpr uint8_t kCommandProxEnable = 0b10;
  static constexpr uint8_t kCommandSelfTimedEnable = 0b1;
};
