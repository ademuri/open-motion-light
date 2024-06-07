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

bool ArduinoVCNL4020::Begin() {
  // The light sensor isn't on the default I2C pins, but
  // `open-motion-light.cc::setup()` configures the default `Wire` to use the
  // correct pins.
  return sensor_.begin();
}

void ArduinoVCNL4020::SetLEDCurrent(uint8_t mA) {
  if (mA > 200) {
    mA = 200;
  }
  sensor_.setProxLEDmA(mA);
}

uint16_t ArduinoVCNL4020::ReadProximity() { return sensor_.readProximity(); }

uint16_t ArduinoVCNL4020::ReadAmbient() { return sensor_.readAmbient(); }
