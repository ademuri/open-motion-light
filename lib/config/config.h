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

// clang-format off
#include <types.h>
// clang-format on

// Ambient light brightness sensing mode
enum class BrightnessMode {
  kDisabled,
  kOnWhenBelow,
};

enum class ProximityMode {
  kDisabled,
  kToggle,
};

// Configuration data
struct Config {
  // Version of this config data. Since this struct is read directly from flash,
  // updates need to only add fields to this struct, or risk breaking "parsing".
  // This version will allow for breaking changes, if needed.
  uint8_t version = 1;

  BrightnessMode brightnessMode = BrightnessMode::kOnWhenBelow;

  // This is units of 1/4 lux - a value of 4000 corresponds to 1000 lux.
  uint16_t autoBrightnessThreshold = 4 * 30;

  ProximityMode proximity_mode = ProximityMode::kDisabled;

  // Turn off the light if it's been on for this long in proximity toggle mode.
  uint16_t proximity_toggle_timeout_seconds = 10 * 60;

  // The proximity sensor must change by this much to toggle the mode.
  uint16_t proximity_threshold = 300;
};
