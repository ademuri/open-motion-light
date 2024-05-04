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
