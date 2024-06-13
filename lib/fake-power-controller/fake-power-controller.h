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

#include "power-controller.h"

class FakePowerController : public PowerController {
 public:
  bool Begin() override {
    initialized_ = true;
    return true;
  }

  void AttachInterruptWakeup(uint32_t pin, uint32_t mode) override {
    ASSERT_TRUE(initialized_);
    ASSERT_LT(pin, kPinMax);
    interrupts_[pin] = mode;
  }

  uint32_t GetPinInterruptMode(uint32_t pin) {
    EXPECT_LT(pin, kPinMax);
    assert(pin < kPinMax);
    return interrupts_[pin];
  }

  void Sleep(uint32_t millis) {
    ASSERT_TRUE(initialized_);
    sleep_millis_ = millis;
  }

  uint32_t GetSleep() { return sleep_millis_; }

 private:
  bool initialized_ = false;
  uint32_t sleep_millis_ = 0;
  std::array<uint32_t, kPinMax> interrupts_;
};
