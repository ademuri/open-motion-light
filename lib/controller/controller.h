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

  // How long the light should be on for after motion is detected. Visible for
  // testing.
  uint32_t GetMotionTimeoutSeconds() { return kMotionTimeoutSeconds; }
  uint32_t GetLedDutyCycle() { return kLedDutyCycle; }

 private:
  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};

  CountUpTimer motion_timer_;
  bool light_is_on_ = false;

  // TODO: make these configurable and store it in EEPROM
  static constexpr uint32_t kMotionTimeoutSeconds = 30;
  static constexpr uint32_t kLedDutyCycle = 128;
};
