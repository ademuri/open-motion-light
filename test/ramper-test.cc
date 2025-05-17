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

#include "ramper.h"

#include <gtest/gtest.h>

namespace {

TEST(Ramper, AllowsUnlimitedChangeByDefault) {
  setMillis(0);
  Ramper ramper;

  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.SetTarget(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 1);
  EXPECT_EQ(ramper.GetActual(), 1);

  ramper.SetTarget(10000);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 10000);
  EXPECT_EQ(ramper.GetActual(), 10000);

  ramper.SetTarget(-10000);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), -10000);
  EXPECT_EQ(ramper.GetActual(), -10000);
}

TEST(Ramper, LimitsChange) {
  setMillis(0);
  Ramper ramper;
  ramper.SetMaxIncrease(1, 1);  // Max increase of 1 per 1ms
  ramper.SetMaxDecrease(1, 1);  // Max decrease of 1 per 1ms

  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.SetTarget(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 1);
  EXPECT_EQ(ramper.GetActual(), 0);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 1);
  EXPECT_EQ(ramper.GetActual(), 1);

  ramper.SetTarget(10);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 10);
  EXPECT_EQ(ramper.GetActual(), 1);

  for (uint32_t n = 2; n <= 10; ++n) {
    advanceMillis(1);
    ramper.Step();
    EXPECT_EQ(ramper.GetTarget(), 10) << n;
    EXPECT_EQ(ramper.GetActual(), n) << n;
  }

  ramper.SetTarget(-10);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), -10);
  EXPECT_EQ(ramper.GetActual(), 10);

  for (int32_t n = 9; n >= -10; --n) {
    advanceMillis(1);
    ramper.Step();
    EXPECT_EQ(ramper.GetTarget(), -10) << n;
    EXPECT_EQ(ramper.GetActual(), n) << n;
  }
}

TEST(Ramper, LimitsChangeAsymmetrically) {
  setMillis(0);
  Ramper ramper;
  ramper.SetMaxIncrease(2, 1);  // Increase by 2 per 1ms, decrease by 1 per 1ms.
  ramper.SetMaxDecrease(1, 1);  // This sets period_ms_ to 1

  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  // Test increasing
  ramper.SetTarget(5);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 0);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 2);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 4);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 5);

  // Test decreasing
  ramper.SetTarget(0);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 5);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 4);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 3);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 2);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 1);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 0);
}

TEST(Ramper, HandlesZeroStepLimit) {
  setMillis(0);
  Ramper ramper;
  ramper.SetMaxIncrease(0, 100);  // Increase is immediate
  ramper.SetMaxDecrease(5, 100);  // Decrease is limited

  ramper.SetTarget(100);
  advanceMillis(100);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 100);

  ramper.SetTarget(90);
  advanceMillis(100);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 95);

  advanceMillis(100);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 90);

  ramper.SetMaxDecrease(0, 100);
  ramper.SetTarget(50);
  advanceMillis(100);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 50);
}

TEST(Ramper, IncreaseMultipleChangesBelowThreshold) {
  setMillis(0);
  Ramper ramper;

  ramper.SetMaxIncrease(10, 10);
  ramper.SetMaxDecrease(10, 10);

  ramper.SetTarget(5);
  advanceMillis(10);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 5);

  ramper.SetTarget(10);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 5);

  advanceMillis(9);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 5);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), 10);
}

TEST(Ramper, DecreaseMultipleChangesBelowThreshold) {
  setMillis(0);
  Ramper ramper;

  ramper.SetMaxIncrease(10, 10);
  ramper.SetMaxDecrease(10, 10);

  ramper.SetTarget(-5);
  advanceMillis(10);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), -5);

  ramper.SetTarget(-10);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), -5);

  advanceMillis(9);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), -5);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetActual(), -10);
}

};  // namespace
