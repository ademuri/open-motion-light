#include "controller.h"

#include "pins.h"

bool Controller::Init() {
  // TODO: read power mode switch
  analogWrite(kPinWhiteLed, 0);

  // Initialize the battery filter
  for (uint32_t i = 0; i < kBatteryMedianFilterSize; i++) {
    battery_median_filter_.Run();
  }
  battery_average_filter_.Initialize(battery_median_filter_.GetFilteredValue());
  battery_average_filter_.Run();
  battery_median_filter_.SetMinRunInterval(kBatteryFilterRunIntervalMillis);
  battery_average_filter_.SetMinRunInterval(kBatteryFilterRunIntervalMillis);

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

void Controller::Step() {
#ifndef ARDUINO
  battery_median_filter_.SetMillis(millis());
  battery_average_filter_.SetMillis(millis());
#endif  // ifndef ARDUINO

  battery_median_filter_.Run();
  battery_average_filter_.Run();

  const PowerMode previous_power_mode = power_mode_;

  if (!power_mode_read_timer_.Running() || power_mode_read_timer_.Expired()) {
    power_mode_ = ReadPowerMode();
    if (previous_power_mode != power_mode_) {
      power_mode_read_timer_.Reset();
      if (power_mode_ == PowerMode::kOff) {
        analogWrite(kPinWhiteLed, 0);
      } else if (power_mode_ == PowerMode::kAuto) {
        analogWrite(kPinWhiteLed, GetLedDutyCycle());
        motion_timer_.Reset();
      } else if (power_mode_ == PowerMode::kOn) {
        analogWrite(kPinWhiteLed, GetLedDutyCycle());
      }
    }

    bool motion_detected = digitalRead(kPinMotionSensor);
    if (motion_detected) {
      motion_timer_.Reset();
    }

    if (power_mode_ == PowerMode::kAuto) {
      if (motion_detected) {
        // TODO: check whether calling this repeatedly when not necessary causes
        // issues.
        analogWrite(kPinWhiteLed, GetLedDutyCycle());
        motion_timer_.Reset();
      } else if (motion_timer_.Running() &&
                 motion_timer_.Get() > GetMotionTimeoutSeconds() * 1000) {
        // TODO: check that this actually turns off the LED
        analogWrite(kPinWhiteLed, 0);
      }
    }
  }
}
