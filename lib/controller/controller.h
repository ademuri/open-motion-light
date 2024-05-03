#pragma once

// clang-format off
#include <types.h>
// clang-format on

#include <arduino-timer.h>

class Controller {
 public:
  enum class PowerMode {
    kOff,
    kAuto,
    kOn,
  };

  // Initializes this object. Returns whether this was successful.
  bool Init();

  PowerMode GetPowerMode() { return power_mode_; };

  void Step();

 private:
  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};
};
