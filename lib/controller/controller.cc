#include "controller.h"

#include "pins.h"

using PowerMode = Controller::PowerMode;

bool Controller::Init() {
  // TODO: read power mode switch
  return true;
}

PowerMode Controller::ReadPowerMode() {
  // On takes precedence over auto. In practical usage, both the power on and
  // power auto pins should only be on at the same time briefly. The input pins
  // are inverted.
  if (!digitalRead(kPinPowerOn)) {
    return PowerMode::kOn;
  } else if (!digitalRead(kPinPowerAuto)) {
    return PowerMode::kAuto;
  }
  return PowerMode::kOff;
}

void Controller::Step() {
  const PowerMode previous_power_mode = power_mode_;

  if (!power_mode_read_timer_.Running() || power_mode_read_timer_.Expired()) {
    power_mode_ = ReadPowerMode();
    if (previous_power_mode != power_mode_) {
      power_mode_read_timer_.Reset();
    }
  }
}
