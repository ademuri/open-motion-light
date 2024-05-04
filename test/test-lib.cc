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
