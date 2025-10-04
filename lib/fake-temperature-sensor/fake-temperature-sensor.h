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
#include <types.h>

#include "temperature-sensor.h"

// Fake implementation of the temperature sensor for testing.
class FakeTemperatureSensor : public TemperatureSensor {
 public:
  bool Begin() override {
    initialized_ = true;
    return true;
  }

  int16_t ReadTemperature() override {
    EXPECT_TRUE(initialized_)
        << "FakeTemperatureSensor::Begin() not yet called";
    return temperature_;
  }

  void SetTemperature(int16_t temperature_celsius) {
    temperature_ = temperature_celsius;
  }

 private:
  bool initialized_ = false;
  int16_t temperature_;
};
