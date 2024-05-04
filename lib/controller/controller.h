#pragma once

// clang-format off
#include <types.h>
// clang-format on

#include <arduino-timer.h>
#include <exponential-moving-average-filter.h>
#include <median-filter.h>

#include "vcnl4010.h"

enum class PowerMode {
  kOff,
  kAuto,
  kOn,
};

class Controller {
 public:
  Controller(VCNL4010* vcnl4010) : vcnl4010_(vcnl4010) {}

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
  uint16_t GetFilteredBatteryMillivolts() {
    return battery_average_filter_.GetFilteredValue();
  }

  // Reads the raw (unfiltered) battery voltage, in millivolts. Visible for
  // testing.
  static uint16_t ReadRawBatteryMillivolts();

  uint16_t ReadProximity() { return vcnl4010_->ReadProximity(); }
  uint16_t ReadAmbientLight() { return vcnl4010_->ReadAmbient(); }

  // Tuning constants - visible for testing
  static constexpr uint8_t kBatteryFilterAlpha = 64;
  static constexpr uint8_t kBatteryMedianFilterSize = 5;
  static constexpr uint32_t kBatteryFilterRunIntervalMillis = 10;

 private:
  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};

  CountUpTimer motion_timer_;

  MedianFilter<uint16_t, uint16_t, kBatteryMedianFilterSize>
      battery_median_filter_{Controller::ReadRawBatteryMillivolts};
  ExponentialMovingAverageFilter<uint16_t> battery_average_filter_{
      [this]() { return battery_median_filter_.GetFilteredValue(); },
      kBatteryFilterAlpha};

  VCNL4010* vcnl4010_;

  // The V_DDA value that the reference calibration was measured using.
  static constexpr uint16_t kReferenceSupplyMillivolts = 3000;
  // The max value for the ADC during the reference measurement, which uses the
  // full 12 bits of the ADC.
  static constexpr uint32_t kAdcMaxCount = 4096;
  // The current configured max value for the ADC, which is 2^<number of bits>.
  static constexpr uint32_t kAdcConfiguredMaxCount = 1 << 10;

  // TODO: make these configurable and store it in EEPROM
  static constexpr uint32_t kMotionTimeoutSeconds = 30;
  static constexpr uint32_t kLedDutyCycle = 128;
};
