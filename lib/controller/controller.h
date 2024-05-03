#pragma once

// clang-format off
#include <types.h>
// clang-format on

#include <arduino-timer.h>
#include <exponential-moving-average-filter.h>
#include <median-filter.h>

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

  // The brightness of the white LEDs when they're on.
  uint32_t GetLedDutyCycle() { return kLedDutyCycle; }

  // Returns the battery voltage, filtered for stability.
  uint16_t GetBatteryVoltage() {
    return battery_average_filter_.GetFilteredValue();
  }

  // Reads the raw (unfiltered) battery voltage, in millivolts. Visible for
  // testing.
  static uint16_t ReadBatteryVoltageMillivolts();

 private:
  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};

  CountUpTimer motion_timer_;
  bool light_is_on_ = false;

  static constexpr uint8_t kBatteryFilterAlpha = 8;
  static constexpr uint8_t kBatteryMedianFilterSize = 5;
  MedianFilter<uint16_t, uint16_t, kBatteryMedianFilterSize>
      battery_median_filter_{Controller::ReadBatteryVoltageMillivolts};
  ExponentialMovingAverageFilter<uint16_t> battery_average_filter_{
      [this]() { return battery_median_filter_.GetFilteredValue(); },
      kBatteryFilterAlpha};

  // The V_DDA value that the reference calibration was measured using.
  static constexpr uint16_t kReferenceSupplyVoltageMillivolts = 3000;

  // TODO: make these configurable and store it in EEPROM
  static constexpr uint32_t kMotionTimeoutSeconds = 30;
  static constexpr uint32_t kLedDutyCycle = 128;
};
