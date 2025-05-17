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

#pragma once

#include <types.h>

// Rate-limits changes to the held value (actual).
// TODO: template the type of actual and extract this into a library
class Ramper {
 public:
  void SetTarget(int16_t target) { target_ = target; }
  int16_t GetTarget() const { return target_; }
  int16_t GetActual() const { return actual_; }

  // Allow this much change over this period. Change will be quantized to happen
  // every `period_ms`.
  void SetMaxChange(int16_t value, uint32_t period_ms) {
    max_change_ = value;
    period_ms_ = period_ms;
  }

  void Step();

 private:
  // Config
  int16_t max_change_ = 0;
  uint32_t period_ms_ = 0;

  // Internal state
  int16_t target_ = 0;
  int16_t actual_ = 0;
  uint32_t last_update_ms_ = 0;
};
