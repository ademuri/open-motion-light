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
  Ramper ramper;
  ramper.SetMaxChange(1, 1);

  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 0);
  EXPECT_EQ(ramper.GetActual(), 0);

  ramper.SetTarget(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 1);
  EXPECT_EQ(ramper.GetActual(), 1);

  ramper.SetTarget(2);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 2);
  EXPECT_EQ(ramper.GetActual(), 1);

  advanceMillis(1);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 2);
  EXPECT_EQ(ramper.GetActual(), 2);

  ramper.SetTarget(10);
  ramper.Step();
  EXPECT_EQ(ramper.GetTarget(), 10);
  EXPECT_EQ(ramper.GetActual(), 2);

  for (uint32_t n = 3; n <= 10; ++n) {
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

};  // namespace
