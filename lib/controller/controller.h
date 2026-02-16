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

#include <arduino-timer.h>
#include <exponential-moving-average-filter.h>
#include <median-filter.h>

#include "pb.h"  // Needed to trigger inclusion of the Nanopb-generated files
#include "power-controller.h"
#include "ramper.h"
#include "serial.pb.h"
#include "temperature-sensor.h"
#include "vcnl4020.h"

enum class PowerMode {
  kOff,
  kAuto,
  kOn,
  kToggled,
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

// Defaults if the config hasn't been programmed. Keep in sync with the UI
// defaults in open-motion-light-manager:src/components/DeviceConfig/index.tsx
static constexpr ConfigPb kDefaultConfig = ConfigPb{
  version : 1,
  brightnessMode : BrightnessMode::BRIGHTNESS_MODE_DISABLED,
  autoBrightnessThreshold : 4 * 30,
  proximity_mode : ProximityMode::PROXIMITY_MODE_DISABLED,
  proximity_toggle_timeout_seconds : 10 * 60,
  proximity_threshold : 300,
  motion_timeout_seconds : 10,
  led_duty_cycle : 255,
  low_battery_cutoff_millivolts : 3000,
  low_battery_hysteresis_threshold_millivolts : 3200,
  motion_sensitivity :
      MotionSensitivity::MotionSensitivity_MOTION_SENSITIVITY_ONE,
};

class Controller {
 public:
  Controller(TemperatureSensor* temperature_sensor, VCNL4020* vcnl4020,
             PowerController* power_controller)
      : temperature_sensor_(temperature_sensor),
        vcnl4020_(vcnl4020),
        power_controller_(power_controller) {}

  // Initializes this object. Returns whether this was successful.
  bool Init();

  void Step();

  PowerMode GetPowerMode() const { return power_mode_; }
  PowerStatus GetPowerStatus() const { return power_status_; }
  USBStatus GetUSBStatus() const { return usb_status_; }

  // Returns the battery voltage, filtered for stability.
  uint16_t GetFilteredBatteryMillivolts() const {
    return battery_average_filter_.GetFilteredValue();
  }

  // Reads the raw (unfiltered) battery voltage, in millivolts. Visible for
  // testing.
  static uint16_t ReadRawBatteryMillivolts();

  static uint16_t ReadAnalogVoltageMillivolts(uint32_t pin,
                                              uint16_t battery_millivolts);

  uint16_t ReadProximity() { return vcnl4020_->ReadProximity(); }
  uint16_t ReadAmbientLight() { return vcnl4020_->ReadAmbient(); }

  // Reads the MCU temperature, in degrees Celsius.
  int16_t ReadTemperature() { return temperature_sensor_->ReadTemperature(); }

  // Configuration values. TODO: store these in EEPROM and add an interface for
  // changing them.

  // How long the light should be on for after motion is detected. Visible for
  // testing.
  uint32_t GetMotionTimeoutSeconds() const {
    return config_.motion_timeout_seconds;
  }

  // The brightness of the white LEDs when they're on.
  // Note: with a 20kHz output duty cycle, minimum value is 3.
  uint32_t GetLedDutyCycle() const { return config_.led_duty_cycle; }

  // This is considered to be the "empty" point for the battery. Below this
  // voltage, the device goes into a lower-power mode to minimize battery drain.
  uint32_t GetLowBatteryCutoffMillivolts() const {
    return config_.low_battery_cutoff_millivolts;
  }

  // Once the battery is low, it must exceed this voltage before the device will
  // turn on again.
  uint32_t GetLowBatteryHysteresisThresholdMillivolts() const {
    return config_.low_battery_hysteresis_threshold_millivolts;
  }

  uint32_t GetSleepInterval() const { return 15 * 60 * 1000; }

  ConfigPb const* GetConfig() const { return &config_; };
  void SetConfig(const ConfigPb& config);

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

  // How long the motion sensor signal is "active" for after detecting motion.
  static constexpr uint32_t kMotionPulseLengthMs = 0;

  // How long to turn on the proximity sensor for after detection motion, when
  // prox sensing is enabled.
  static constexpr uint32_t kMotionProximityPeriodMs = 3500;

  // In auto mode with brightness detection, ignore the light sensor for this
  // long after the LED is on. The light sensor integrates over 1 second, so
  // ignore 2 seconds to ensure there are no samples included with the light on.
  static constexpr uint32_t kBrightnessIgnorePeriodMs = 2000;

  static constexpr uint16_t kUsbNoConnectionMillivolts = 200;
  static constexpr uint16_t kUsbStandardMillivolts = 660;
  static constexpr uint16_t kUsb1_5Millivolts = 1230;

  // The duty cycle for LEDs when they're not active, but are lit up for
  // clarity.
  static constexpr uint16_t kBatteryLedPlaceholderBrightness = 1;

  // The duty cycle for LEDs when they're not active, but are lit up for
  // clarity.
  static constexpr uint16_t kBatteryLedActiveBrightness = 255;

  // Battery levels. This is non-linear, since LiFePO4 batteries have a ~cubic
  // discharge curve.
  static inline constexpr uint16_t kBatteryVoltage1 = 3350;
  static inline constexpr uint16_t kBatteryVoltage0 = 3150;

  static constexpr uint16_t kSleepLockoutMs = 1000;

 private:
  // Handles an updated config.
  void ConfigUpdated();

  // Reads the state of the power mode switch.
  static PowerMode ReadPowerMode();

  PowerMode power_mode_ = PowerMode::kOff;
  // Used to debounce reading the power mode switch.
  CountDownTimer power_mode_read_timer_{10};

  // After anything changes, wait this long before going to sleep. This prevents
  // missing mode changes if the switch bounces.
  CountDownTimer sleep_lockout_timer{kSleepLockoutMs};

  PowerStatus power_status_ = PowerStatus::kBattery;
  USBStatus usb_status_ = USBStatus::kNoConnection;
  USBStatus prev_usb_status_ = USBStatus::kNoConnection;

  CountUpTimer motion_timer_;

  CountDownTimer battery_level_timer_{kBatteryLevelDisplayTimeSeconds * 1000};
  CountDownTimer led_change_motion_timeout_{kMotionPulseLengthMs};
  CountDownTimer led_on_brightness_timeout_{kBrightnessIgnorePeriodMs};
  CountDownTimer motion_proximity_timeout_{kMotionProximityPeriodMs};

  bool led_on_ = false;

  MedianFilter<uint16_t, uint16_t, kBatteryMedianFilterSize>
      battery_median_filter_{Controller::ReadRawBatteryMillivolts};
  ExponentialMovingAverageFilter<uint16_t> battery_average_filter_{
      [this]() { return battery_median_filter_.GetFilteredValue(); },
      kBatteryFilterAlpha};

  VCNL4020* const vcnl4020_;
  PowerController* const power_controller_;
  TemperatureSensor* const temperature_sensor_;

  Ramper led_ramper_;

  int32_t prev_proximity_ = 0;

  ConfigPb config_ = kDefaultConfig;
};
