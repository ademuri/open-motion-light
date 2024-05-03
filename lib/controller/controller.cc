#include "controller.h"

#include "pins.h"

using PowerMode = Controller::PowerMode;

bool Controller::Init() {
  // TODO: read power mode switch
  analogWrite(kPinWhiteLed, 0);
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

uint16_t Controller::ReadBatteryVoltageMillivolts() {
  // Read the factory-calibrated voltage of the reference.
  static const uint32_t vrefint_cal = *kAdcReferencePointer;
  // The supply (VDDA) voltage during the calibration measurement.
  static constexpr uint32_t kVrefMeasMillivolts = 3000;
  // The max value for the ADC, which is 2^(number of ADC bits).
  static constexpr uint32_t kMaxCount = 4096;
  // Read the current value of the reference
  uint32_t vrefint_raw = analogRead(kPinAdcReference);
  // Battery voltage (Vcc) is the max value - for the default resolution of 10 bits, this is 1023.
  return (vrefint_cal * kVrefMeasMillivolts) / vrefint_raw;
}

void Controller::Step() {
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
        if (!light_is_on_) {
          analogWrite(kPinWhiteLed, GetLedDutyCycle());
        }
        motion_timer_.Reset();
      } else if (motion_timer_.Running() &&
                 motion_timer_.Get() > GetMotionTimeoutSeconds() * 1000) {
        // TODO: check that this actually turns off the LED
        analogWrite(kPinWhiteLed, 0);
      }
    }
  }
}
