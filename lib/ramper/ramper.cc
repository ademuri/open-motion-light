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

void Ramper::Step() {
  if (target_ == actual_) {
    return;
  }

  if (max_change_ == 0 || period_ms_ == 0) {
    actual_ = target_;
    return;
  }

  const uint32_t now = millis();

  if (now - last_update_ms_ < period_ms_) {
    return;
  }

  const int16_t difference = target_ - actual_;
  const int16_t abs_difference = std::abs(difference);

  if (abs_difference < max_change_) {
    actual_ = target_;
  } else {
    if (difference > 0) {
      actual_ += max_change_;
    } else {
      actual_ -= max_change_;
    }
  }

  last_update_ms_ = now;
}
