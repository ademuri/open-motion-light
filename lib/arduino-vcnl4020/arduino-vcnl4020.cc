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

#include "arduino-vcnl4020.h"

#include <Wire.h>

bool ArduinoVCNL4020::Begin() {
  // The light sensor isn't on the default I2C pins, but
  // `open-motion-light.cc::setup()` configures the default `Wire` to use the
  // correct pins.

  // Check whether we can detect the sensor.
  Wire.beginTransmission(kDeviceAddress);
  uint8_t detected = Wire.endTransmission() == 0;

  if (!detected) {
    Serial1.println("Failed to detect VCNL4020");
    return false;
  }

  uint8_t revision = ReadByte(kRegProductId);
  if (revision != 0x21) {
    Serial1.printf(
        "Got incorrect product ID from light sensor. Expected '0x21', got "
        "'0x%X'.\n",
        revision);
    return false;
  }

  // Initialize defaults - sensor standby (1.5uA power consumption)
  WriteByte(kRegCommand, command_);

  WriteByte(kRegAlsParameter, als_parameter_);

  return true;
}

void ArduinoVCNL4020::SetLEDCurrent(uint8_t mA) {
  if (mA > 200) {
    mA = 200;
  }
  // TODO: implement this
}

uint16_t ArduinoVCNL4020::ReadProximity() {
  // TODO: implement this
  return 0;
}

void ArduinoVCNL4020::SetPeriodicAmbient(bool enable) {
  // Note: self-timed enable is required for periodic ALS or prox measurements.
  // TODO: when this library supports the prox sensor, this code will need to
  // leave kCommandSelfTimedEnable active if prox is still enabled.
  if (enable) {
    command_ = command_ | kCommandAlsEnable | kCommandSelfTimedEnable;
  } else {
    command_ = command_ & (0xFF ^ kCommandAlsEnable ^ kCommandSelfTimedEnable);
  }
  WriteByte(kRegCommand, command_);
}

bool ArduinoVCNL4020::AmbientReady() {
  return ReadByte(kRegCommand) & kCommandAlsDataReady;
}

uint16_t ArduinoVCNL4020::ReadAmbient() {
  uint16_t result = ReadByte(kRegAlsResultLow);
  result |= ReadByte(kRegAlsResultHigh) << 8;
  return result;
}

uint8_t ArduinoVCNL4020::ReadByte(uint8_t register_address) {
  Wire.beginTransmission(kDeviceAddress);
  Wire.write(register_address);
  Wire.endTransmission(false);
  Wire.requestFrom(kDeviceAddress, (uint8_t)1);
  if (Wire.available() == 0) {
    Serial1.printf("Error: got no data from light sensor for register: 0x%X\n",
                   register_address);
    return 0;
  }
  return Wire.read();
}

uint8_t ArduinoVCNL4020::WriteByte(uint8_t register_address, uint8_t data) {
  Wire.beginTransmission(kDeviceAddress);
  Wire.write(register_address);
  Wire.write(data);
  return Wire.endTransmission();
}

uint8_t ArduinoVCNL4020::ReadStatus() { return ReadByte(kRegCommand); }
