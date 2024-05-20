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

#include <gtest/gtest.h>

#include <iostream>

#include "controller.h"
#include "types.h"

std::ostream& operator<<(std::ostream& os, const PowerMode power_mode);

std::ostream& operator<<(std::ostream& os, const PowerStatus power_status);

std::ostream& operator<<(std::ostream& os, const USBStatus usb_status);

class LightTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset Arduino compatibility layer's static state
    setMillis(0);
    for (uint32_t pin = 0; pin < kPinMax; pin++) {
      setDigitalRead(pin, false);
      analogWrite(pin, 0);
    }
  }
};
