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

#include "vcnl4020.h"

// Fake implementation of the VCNL4020 for testing.
class FakeVCNL4020 : public VCNL4020 {
 public:
  // Initializes the sensor.
  bool Begin() override {
    initialized_ = true;
    return true;
  }

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  void SetLEDCurrent(uint8_t mA) override {
    ASSERT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    ASSERT_LE(mA, 200);
    ASSERT_EQ(mA % 10, 0) << "LED current has 10s precision";
    led_current_ma_ = mA;
  }

  uint8_t GetLEDCurrent() { return led_current_ma_; }

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  uint16_t ReadProximity() override {
    EXPECT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    return proximity_;
  }

  // Sets the value of the proximity for testing.
  void SetProximity(uint16_t val) { proximity_ = val; }

  void SetPeriodicAmbient(bool enable) override {
    ASSERT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    periodic_ambient_ = enable;
  }

  bool GetPeriodicAmbient() {
    EXPECT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    return periodic_ambient_;
  }

  bool AmbientReady() override {
    EXPECT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    return ambient_ready_;
  }

  void SetAmbientReady() {
    EXPECT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    ambient_ready_ = true;
  }

  // Reads the 16-bit ambient light value.
  uint16_t ReadAmbient() override {
    EXPECT_TRUE(initialized_) << "FakeVCNL4020::Begin() not yet called";
    ambient_ready_ = false;
    return ambient_;
  }

  // Sets the value of the ambient light for testing.
  void SetAmbient(uint16_t val) { ambient_ = val; }

 private:
  bool initialized_ = false;
  bool periodic_ambient_ = false;
  bool ambient_ready_ = false;
  uint8_t led_current_ma_;
  uint16_t proximity_;
  uint16_t ambient_;
};
