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

  const uint32_t now = millis();
  const int16_t difference = target_ - actual_;

  if (difference > 0) {  // Target is higher, we need to increase actual_
    // If limit is zero OR period is zero for increase, snap immediately.
    // This also handles the default uninitialized state.
    if (max_increase_ == 0 || period_increase_ms_ == 0) {
      actual_ = target_;
      last_update_ms_ = now;
      return;
    }

    if (now - last_update_ms_ < period_increase_ms_) {
      return;
    }

    // Enough time has passed for an increase step
    if (difference <
        max_increase_) {  // Remaining difference is less than one step
      actual_ = target_;
    } else {
      actual_ += max_increase_;
    }
    last_update_ms_ = now;  // Update time after a successful step
  } else {  // difference < 0 (or 0, but caught by initial check). Target is
            // lower, we need to decrease actual_
    // If limit is zero OR period is zero for decrease, snap immediately.
    // This also handles the default uninitialized state.
    if (max_decrease_ == 0 || period_decrease_ms_ == 0) {
      actual_ = target_;
      last_update_ms_ = now;
      return;
    }

    if (now - last_update_ms_ < period_decrease_ms_) {
      return;
    }

    // Enough time has passed for a decrease step
    // difference is negative, max_decrease_ is positive magnitude (abs value)
    if (-difference <
        max_decrease_) {  // Absolute remaining difference is less than one step
      actual_ = target_;
    } else {
      actual_ -= max_decrease_;
    }
    last_update_ms_ = now;  // Update time after a successful step
  }
}
