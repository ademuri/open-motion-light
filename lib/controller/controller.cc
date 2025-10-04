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

#include "controller.h"

#include "config-storage.h"
#include "pins.h"
#include "serial.pb.h"

// Useful for debugging, to display other values on the battery LEDs.
constexpr bool kShowBatteryStatus = true;

void SetSensitivityPins(const ConfigPb& config) {
  // To "disconnect" a pin from the voltage divider, we set it to a
  // high-impedance state.
  pinMode(kPinSensitivityLow, INPUT_ANALOG);
  pinMode(kPinSensitivityHigh1, INPUT_ANALOG);
  pinMode(kPinSensitivityHigh2, INPUT_ANALOG);

  switch (config.motion_sensitivity) {
    case MotionSensitivity::MotionSensitivity_MOTION_SENSITIVITY_UNSPECIFIED:
    case MotionSensitivity::MotionSensitivity_MOTION_SENSITIVITY_ONE:
      // All pins are high-impedance, so the voltage on the SENS pin is pulled
      // low by its internal pulldown resistor. This is the highest sensitivity.
      break;

    case MotionSensitivity::MotionSensitivity_MOTION_SENSITIVITY_TWO:
      pinMode(kPinSensitivityHigh1, OUTPUT);
      digitalWrite(kPinSensitivityHigh1, HIGH);
      break;

    case MotionSensitivity::MotionSensitivity_MOTION_SENSITIVITY_THREE:
      pinMode(kPinSensitivityLow, OUTPUT);
      digitalWrite(kPinSensitivityLow, HIGH);
      pinMode(kPinSensitivityHigh1, OUTPUT);
      digitalWrite(kPinSensitivityHigh1, HIGH);
      pinMode(kPinSensitivityHigh2, OUTPUT);
      digitalWrite(kPinSensitivityHigh2, HIGH);
      break;
  }
}

bool Controller::Init() {
  pinMode(kPinSensitivityLow, INPUT_ANALOG);
  pinMode(kPinSensitivityHigh1, INPUT_ANALOG);
  pinMode(kPinSensitivityHigh2, INPUT_ANALOG);
  // For some reason, this causes the LEDs to flash (likely something to do with
  // the STM32 Arduino implementation).
  // analogWrite(kPinWhiteLed, 0);
  led_on_ = false;

  // Initialize the battery filter
  for (uint32_t i = 0; i < kBatteryMedianFilterSize; i++) {
    battery_median_filter_.Run();
  }
  battery_average_filter_.Initialize(battery_median_filter_.GetFilteredValue());
  battery_average_filter_.Run();
  battery_median_filter_.SetMinRunInterval(kBatteryFilterRunIntervalMillis);
  battery_average_filter_.SetMinRunInterval(kBatteryFilterRunIntervalMillis);

  if (!vcnl4020_->Begin()) {
    return false;
  }
  // TODO: set this from config
  vcnl4020_->SetLEDCurrent(80);

  if (!power_controller_->Begin()) {
    return false;
  }

  // Note: there seems to be either an order dependence or a limited number. If
  // the interrupt for the 5V detect pin is first, it doesn't work.
  power_controller_->AttachInterruptWakeup(kPinPowerAuto, CHANGE);
  power_controller_->AttachInterruptWakeup(kPinMotionSensor, RISING);
  power_controller_->AttachInterruptWakeup(kPinPowerOn, CHANGE);
  power_controller_->AttachInterruptWakeup(kPin5vDetect, RISING);

  ConfigStorage::TryLoadConfig(&config_);
  ConfigUpdated();

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

uint16_t Controller::ReadRawBatteryMillivolts() {
  // Note: while the ADC (and cal) are 12-bit values, these use uint32_t to
  // avoid overflow.

  // Read the factory-calibrated voltage of the reference.
  static const uint32_t vrefint_cal = *kAdcReferencePointer;

  // Read the current value of the reference
  uint32_t vrefint_raw = analogRead(kPinAdcReference);

  // Avoid divide-by-zero
  if (vrefint_raw == 0) {
    return 0;
  }

  // VREF = CAL / kMaxCount * 3000mV
  // VCC = VREF / VAL * kConfiguredMaxCount
  // This is arranged to avoid losing precision.
  return ((vrefint_cal * kReferenceSupplyMillivolts) /
          (kAdcMaxCount / kAdcConfiguredMaxCount)) /
         vrefint_raw;
}

void Controller::SetConfig(const ConfigPb& config) {
  config_ = config;
  ConfigUpdated();
}

void Controller::ConfigUpdated() {
  if (config_.ramp_up_time_ms != 0) {
    if (config_.ramp_up_time_ms < GetLedDutyCycle()) {
      led_ramper_.SetMaxIncrease(GetLedDutyCycle() / config_.ramp_up_time_ms,
                                 1);
    } else {
      led_ramper_.SetMaxIncrease(1,
                                 config_.ramp_up_time_ms / GetLedDutyCycle());
    }
  }
  if (config_.ramp_down_time_ms != 0) {
    if (config_.ramp_down_time_ms < GetLedDutyCycle()) {
      led_ramper_.SetMaxDecrease(GetLedDutyCycle() / config_.ramp_down_time_ms,
                                 1);
    } else {
      led_ramper_.SetMaxDecrease(1,
                                 config_.ramp_down_time_ms / GetLedDutyCycle());
    }
  }
  SetSensitivityPins(config_);
}

uint16_t Controller::ReadAnalogVoltageMillivolts(
    const uint32_t pin, const uint16_t battery_millivolts) {
  uint32_t pin_value = analogRead(pin);
  return (pin_value * battery_millivolts) / kAdcConfiguredMaxCount;
}

void Controller::Step() {
#ifndef ARDUINO
  battery_median_filter_.SetMillis(millis());
  battery_average_filter_.SetMillis(millis());
#endif  // ifndef ARDUINO

  battery_median_filter_.Run();
  battery_average_filter_.Run();

  const PowerMode previous_power_mode = power_mode_;
  bool auto_triggered = false;

  if (!power_mode_read_timer_.Running() || power_mode_read_timer_.Expired()) {
    power_mode_ = ReadPowerMode();
    if (previous_power_mode == PowerMode::kToggled &&
        power_mode_ == PowerMode::kAuto) {
      power_mode_ = PowerMode::kToggled;
    } else if (previous_power_mode != power_mode_) {
      power_mode_read_timer_.Reset();
      sleep_lockout_timer.Reset();
      if (power_mode_ == PowerMode::kOff) {
        vcnl4020_->SetPeriodicAmbient(false);
        battery_level_timer_.Reset();
      } else if (power_mode_ == PowerMode::kAuto) {
        if (previous_power_mode == PowerMode::kToggled) {
        } else {
          vcnl4020_->SetPeriodicAmbient(
              config_.brightnessMode !=
              BrightnessMode::BRIGHTNESS_MODE_DISABLED);
          auto_triggered = true;
          motion_timer_.Reset();
          battery_level_timer_.Reset();
          motion_proximity_timeout_.Reset();
          if (config_.proximity_mode == ProximityMode::PROXIMITY_MODE_TOGGLE) {
            vcnl4020_->SetPeriodicProximity(true);
          }
        }
      } else if (power_mode_ == PowerMode::kOn) {
        vcnl4020_->SetPeriodicAmbient(false);
        battery_level_timer_.Reset();
      }
    }
  }

  if ((power_mode_ == PowerMode::kAuto || power_mode_ == PowerMode::kToggled) &&
      config_.proximity_mode == ProximityMode::PROXIMITY_MODE_TOGGLE &&
      vcnl4020_->ProximityReady()) {
    int32_t proximity = vcnl4020_->ReadProximity();

    if (prev_proximity_ != 0 &&
        std::abs(proximity - prev_proximity_) > config_.proximity_threshold) {
      if (power_mode_ == PowerMode::kAuto) {
        power_mode_ = PowerMode::kToggled;
        proximity = 0;
      } else {
        power_mode_ = PowerMode::kAuto;
        proximity = 0;
      }
    }

    prev_proximity_ = proximity;
  }

  // ADC readings are relative to the battery voltage.
  // TODO: possibly consolidate this read with the one inside the battery median
  // filter, to save power.
  const uint16_t battery_millivolts = ReadRawBatteryMillivolts();
  const uint16_t cc1_millivolts =
      ReadAnalogVoltageMillivolts(/*pin=*/kPinCc1, battery_millivolts);
  const uint16_t cc2_millivolts =
      ReadAnalogVoltageMillivolts(/*pin=*/kPinCc2, battery_millivolts);
  const uint16_t cc_millivolts = std::max(cc1_millivolts, cc2_millivolts);
  if (cc_millivolts < kUsbNoConnectionMillivolts) {
    usb_status_ = USBStatus::kNoConnection;
  } else if (cc_millivolts < kUsbStandardMillivolts) {
    usb_status_ = USBStatus::kStandardUsb;
  } else if (cc_millivolts < kUsb1_5Millivolts) {
    usb_status_ = USBStatus::kUSB1_5;
  } else {
    usb_status_ = USBStatus::kUSB3_0;
  }
  switch (usb_status_) {
    case USBStatus::kNoConnection:
    case USBStatus::kStandardUsb:
      digitalWrite(kPinChargeHighCurrentEnable, false);
      break;

    case USBStatus::kUSB1_5:
    case USBStatus::kUSB3_0:
      digitalWrite(kPinChargeHighCurrentEnable, true);
      break;
  }

  if (prev_usb_status_ != usb_status_) {
    if (usb_status_ == USBStatus::kNoConnection) {
      vcnl4020_->SetPeriodicAmbient(false);
      vcnl4020_->SetPeriodicProximity(false);
    } else {
      vcnl4020_->SetPeriodicAmbient(true);
      vcnl4020_->SetPeriodicProximity(true);
    }
  }

  prev_usb_status_ = usb_status_;

  {
    const bool charging_value = !digitalRead(kPinBatteryCharge);
    const bool done_value = !digitalRead(kPinBatteryDone);
    // TODO: possibly adjust the threshold based on estimated current
    // consumption when the white LEDs are on.
    static bool battery_low = false;
    if (battery_low) {
      battery_low = GetFilteredBatteryMillivolts() <
                    GetLowBatteryHysteresisThresholdMillivolts();
    } else {
      battery_low =
          GetFilteredBatteryMillivolts() < GetLowBatteryCutoffMillivolts();
    }

    if (battery_low && charging_value && !done_value) {
      power_status_ = PowerStatus::kLowBatteryCutoffCharging;
    } else if (battery_low) {
      power_status_ = PowerStatus::kLowBatteryCutoff;
    } else if (!charging_value && !done_value) {
      if (usb_status_ == USBStatus::kNoConnection) {
        power_status_ = PowerStatus::kBattery;
      } else {
        power_status_ = PowerStatus::kChargeError;
      }
    } else if (charging_value && done_value) {
      // This should't be possible, according to the CN3058E datasheet.
      power_status_ = PowerStatus::kChargeError;
    } else if (charging_value) {
      power_status_ = PowerStatus::kCharging;
    } else if (done_value) {
      power_status_ = PowerStatus::kCharged;
    }
  }

  if (power_status_ == PowerStatus::kLowBatteryCutoff) {
    if (led_on_) {
      analogWrite(kPinWhiteLed, 0);
      led_ramper_.SetActual(0);
      led_on_ = false;
    }
    analogWrite(kPinBatteryLed1, 0);
    analogWrite(kPinBatteryLed2, 0);
    analogWrite(kPinBatteryLed3, 0);
    power_controller_->Stop();
    return;
  }

  bool motion_detected = digitalRead(kPinMotionSensor);
  // If the LED was changed recently, then ignore the motion sensor, since the
  // LEDs shining on the lens can trigger the sensor.
  // https://marriedtotheseacomics.com/image/103884129802
  if (led_change_motion_timeout_.Active()) {
    motion_detected = false;
  }
  static bool prev_motion_detected;
  if (motion_detected && !prev_motion_detected) {
    motion_timer_.Reset();
    motion_proximity_timeout_.Reset();
    if (config_.proximity_mode == ProximityMode::PROXIMITY_MODE_TOGGLE) {
      vcnl4020_->SetPeriodicProximity(true);
    }
  }
  prev_motion_detected = motion_detected;

  if (led_on_) {
    led_on_brightness_timeout_.Reset();
  }

  if (power_status_ == PowerStatus::kLowBatteryCutoffCharging) {
    if (led_on_) {
      analogWrite(kPinWhiteLed, 0);
      led_ramper_.SetActual(0);
      led_on_ = false;
    }
  } else if (power_mode_ == PowerMode::kOff) {
    if (led_on_) {
      led_ramper_.SetTarget(0);
    }
  } else if (usb_status_ != USBStatus::kNoConnection) {
    if (led_on_) {
      led_ramper_.SetTarget(0);
    }
  } else if (power_mode_ == PowerMode::kOn) {
    if (!led_on_) {
      led_ramper_.SetTarget(GetLedDutyCycle());
    }
  } else if (power_mode_ == PowerMode::kAuto) {
    if (motion_detected || auto_triggered) {
      if (!led_on_ &&
          (config_.brightnessMode == BrightnessMode::BRIGHTNESS_MODE_DISABLED ||
           auto_triggered || led_on_brightness_timeout_.Active() ||
           vcnl4020_->ReadAmbient() < config_.autoBrightnessThreshold)) {
        led_ramper_.SetTarget(GetLedDutyCycle());
        led_change_motion_timeout_.Reset();
      }
      motion_timer_.Reset();
    } else if (motion_timer_.Running() &&
               motion_timer_.Get() > GetMotionTimeoutSeconds() * 1000) {
      if (led_on_) {
        led_ramper_.SetTarget(0);
        led_change_motion_timeout_.Reset();
      }
      led_on_ = false;
      motion_timer_.Stop();
    }
  }

  if (motion_proximity_timeout_.Expired() &&
      power_mode_ != PowerMode::kToggled &&
      usb_status_ == USBStatus::kNoConnection) {
    vcnl4020_->SetPeriodicProximity(false);
    motion_proximity_timeout_.Stop();
  }

  if (kShowBatteryStatus) {
    if (power_status_ == PowerStatus::kCharging ||
        power_status_ == PowerStatus::kLowBatteryCutoffCharging) {
      const uint16_t battery_millivolts =
          battery_average_filter_.GetFilteredValue();
      // Slow blink
      const uint8_t brightness = (millis() / 500) % 2 == 0
                                     ? kBatteryLedActiveBrightness
                                     : kBatteryLedPlaceholderBrightness;
      analogWrite(kPinBatteryLed1, battery_millivolts > kBatteryVoltage1
                                       ? brightness
                                       : kBatteryLedPlaceholderBrightness);
      analogWrite(kPinBatteryLed2, battery_millivolts > kBatteryVoltage0
                                       ? brightness
                                       : kBatteryLedPlaceholderBrightness);
      analogWrite(kPinBatteryLed3, brightness);
    } else if (power_status_ == PowerStatus::kCharged) {
      analogWrite(kPinBatteryLed1, kBatteryLedActiveBrightness);
      analogWrite(kPinBatteryLed2, kBatteryLedActiveBrightness);
      analogWrite(kPinBatteryLed3, kBatteryLedActiveBrightness);
    } else if (battery_level_timer_.Active()) {
      const uint16_t battery_millivolts =
          battery_average_filter_.GetFilteredValue();
      analogWrite(kPinBatteryLed1, battery_millivolts > kBatteryVoltage1
                                       ? kBatteryLedActiveBrightness
                                       : kBatteryLedPlaceholderBrightness);
      analogWrite(kPinBatteryLed2, battery_millivolts > kBatteryVoltage0
                                       ? kBatteryLedActiveBrightness
                                       : kBatteryLedPlaceholderBrightness);
      analogWrite(kPinBatteryLed3, kBatteryLedActiveBrightness);
    } else if (power_status_ == PowerStatus::kChargeError) {
      // Fast blink
      const uint8_t brightness = (millis() / 100) % 2 == 0
                                     ? kBatteryLedActiveBrightness
                                     : kBatteryLedPlaceholderBrightness;
      analogWrite(kPinBatteryLed1, brightness);
      analogWrite(kPinBatteryLed2, brightness);
      analogWrite(kPinBatteryLed3, brightness);
    } else {
      analogWrite(kPinBatteryLed1, 0);
      analogWrite(kPinBatteryLed2, 0);
      analogWrite(kPinBatteryLed3, 0);
    }
  }

  int16_t prev_actual = led_ramper_.GetActual();
  led_ramper_.Step();
  if (led_ramper_.GetActual() != prev_actual) {
    analogWrite(kPinWhiteLed, led_ramper_.GetActual());
  }
  led_on_ = led_ramper_.GetActual() > 0;

  const bool proximity_lockout =
      config_.proximity_mode != ProximityMode::PROXIMITY_MODE_DISABLED &&
      motion_proximity_timeout_.Active();
  if (!led_on_ && power_status_ != PowerStatus::kCharging &&
      !sleep_lockout_timer.Active() && !proximity_lockout &&
      !battery_level_timer_.Active()) {
    power_controller_->Sleep(GetSleepInterval());
  }
}
