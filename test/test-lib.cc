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

#include "test-lib.h"

#include <map>

std::ostream& operator<<(std::ostream& os, const PowerMode power_mode) {
  switch(power_mode) {
    case PowerMode::kOff:
      os << "kOff";
      break;

    case PowerMode::kAuto:
      os << "kAuto";
      break;

    case PowerMode::kOn:
      os << "kOn";
      break;

    case PowerMode::kToggled:
      os << "kToggled";
      break;

    default:
      os << static_cast<int>(power_mode);
      break;
  }
  return os;
}

static const std::map<PowerStatus, std::string> kPowerStatusMap = {
  {PowerStatus::kBattery, "battery"},
  {PowerStatus::kCharging, "charging"},
  {PowerStatus::kCharged, "charged"},
  {PowerStatus::kChargeError, "charge_error"},
  {PowerStatus::kLowBatteryCutoff, "low_battery_cutoff"},
};

std::ostream& operator<<(std::ostream& os, const PowerStatus power_status) {
  auto value = kPowerStatusMap.find(power_status);
  if (value == kPowerStatusMap.end()) {
    os << static_cast<int>(power_status);
  } else {
    os << value->second;
  }
  return os;
}


static const std::map<USBStatus, std::string> kUSBStatusMap = {
  {USBStatus::kNoConnection, "no_connection"},
  {USBStatus::kStandardUsb, "standard_usb"},
  {USBStatus::kUSB1_5, "usb_1.5A"},
  {USBStatus::kUSB3_0, "usb_3.0A"},
};

std::ostream& operator<<(std::ostream& os, const USBStatus usb_status) {
  auto value = kUSBStatusMap.find(usb_status);
  if (value == kUSBStatusMap.end()) {
    os << static_cast<int>(usb_status);
  } else {
    os << value->second;
  }
  return os;
}
