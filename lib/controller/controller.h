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

enum class PowerStatus {
  kBattery,
  kCharging,
  kCharged,
  kChargeError,
  kLowBatteryCutoff,
  // The battery is low enough that we should operate in low-power mode, but
  // we're charging, so we should display that on the charge status LEDs.
  kLowBatteryCutoffCharging,
};

enum class USBStatus {
  kNoConnection,
  // "Standard" current is available. This typically means that the device is
  // plugged into a USB A port via an A-to-C cable. Technically, this requires
  // negotiation with the host to determine the current available. However, most
  // USB A port sources (e.g. cheap chargers) support currents >=1A, and do no
  // typically support current negotiation. So, from a user's perspective, the
  // "correct" behavior is for us to charge from one of these sources - and to
  // be conservative, we'll use the lower-current charging option, to keep draw
  // below 500mA.
  kStandardUsb,
  // 1.5A is available.
  kUSB1_5,
  // 3A is available. This device shouldn't draw more than about 1.5A, so we
  // don't need to distinguish this from the 1.5A case.
  kUSB3_0,
};

class Controller {
 public:
  Controller(VCNL4010* vcnl4010) : vcnl4010_(vcnl4010) {}

  // Initializes this object. Returns whether this was successful.
  bool Init();

  void Step();

  PowerMode GetPowerMode() { return power_mode_; }
  PowerStatus GetPowerStatus() { return power_status_; }
  USBStatus GetUSBStatus() { return usb_status_; }

  // Returns the battery voltage, filtered for stability.
  uint16_t GetFilteredBatteryMillivolts() {
    return battery_average_filter_.GetFilteredValue();
  }

  // Reads the raw (unfiltered) battery voltage, in millivolts. Visible for
  // testing.
  static uint16_t ReadRawBatteryMillivolts();

  static uint16_t ReadAnalogVoltageMillivolts(uint32_t pin,
                                              uint16_t battery_millivolts);

  uint16_t ReadProximity() { return vcnl4010_->ReadProximity(); }
  uint16_t ReadAmbientLight() { return vcnl4010_->ReadAmbient(); }

  // Configuration values. TODO: store these in EEPROM and add an interface for
  // changing them.

  // How long the light should be on for after motion is detected. Visible for
  // testing.
  uint32_t GetMotionTimeoutSeconds() { return 128; }

  // The brightness of the white LEDs when they're on.
  uint32_t GetLedDutyCycle() { return 128; }

  // This is considered to be the "empty" point for the battery. Below this
  // voltage, the device goes into a lower-power mode to minimize battery drain.
  uint32_t GetLowBatteryCutoffMillivolts() { return 3000; }

  // Tuning constants - visible for testing
  static constexpr uint8_t kBatteryFilterAlpha = 64;
  static constexpr uint8_t kBatteryMedianFilterSize = 5;
  static constexpr uint32_t kBatteryFilterRunIntervalMillis = 10;

  static constexpr uint32_t kBatteryLevelDisplayTimeSeconds = 10;

  // The V_DDA value that the reference calibration was measured using.
  static constexpr uint16_t kReferenceSupplyMillivolts = 3000;
  // The max value for the ADC during the reference measurement, which uses the
  // full 12 bits of the ADC.
  static constexpr uint32_t kAdcMaxCount = 4096;
  // The current configured max value for the ADC, which is 2^<number of bits>.
  static constexpr uint32_t kAdcConfiguredMaxCount = 1 << 10;

  static constexpr uint16_t kUsbNoConnectionMillivolts = 200;
  static constexpr uint16_t kUsbStandardMillivolts = 660;
  static constexpr uint16_t kUsb1_5Millivolts = 1230;

  // The duty cycle for LEDs when they're not active, but are lit up for
  // clarity.
  static constexpr uint16_t kBatteryLedPlaceholderBrightness = 16;

  // Battery levels. This is non-linear, since LiFePO4 batteries have a ~cubic
  // discharge curve.
  static inline constexpr uint16_t kBatteryVoltage1 = 3400;
  static inline constexpr uint16_t kBatteryVoltage0 = 3150;

 private:
  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};

  PowerStatus power_status_ = PowerStatus::kBattery;
  USBStatus usb_status_ = USBStatus::kNoConnection;

  CountUpTimer motion_timer_;

  CountDownTimer battery_level_timer_{kBatteryLevelDisplayTimeSeconds * 1000};

  MedianFilter<uint16_t, uint16_t, kBatteryMedianFilterSize>
      battery_median_filter_{Controller::ReadRawBatteryMillivolts};
  ExponentialMovingAverageFilter<uint16_t> battery_average_filter_{
      [this]() { return battery_median_filter_.GetFilteredValue(); },
      kBatteryFilterAlpha};

  VCNL4010* vcnl4010_;
};
