/*
 * Copyright 2026 Google LLC
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

#include <median-filter.h>
#include <types.h>

#include "controller.h"
#include "pins.h"
#include "temperature-sensor.h"

// Implementation of the temperature sensor HAL using an external NTC
// thermistor biased by a constant-current source (such as the BQ25170DSGR
// charger TS pin).
class ExternalTempSensor : public TemperatureSensor {
 public:
  explicit ExternalTempSensor(
      MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>&
          battery_median_filter,
      uint32_t pin = kPinTempSensor);

  bool Begin() override;
  int16_t ReadTemperature() override;

  // Resistance in Ohms calculated from the analog pin reading and battery
  // voltage (in mV).
  static float CalculateResistance(float pin_value, float battery_mv);

  // Temperature in Celsius calculated from resistance using the B-parameter
  // Steinhart-Hart equation.
  static float CalculateTemperatureCelsius(float resistance_ohm);

 private:
  static constexpr float kNominalTemperatureKelvin = 273.15f + 25.0f;
  static constexpr float kNominalResistanceOhm = 10000.0f;
  static constexpr float kBetaCoefficient = 3950.0f;
  static constexpr float kBiasCurrentMicroamps = 38.0f;
  static constexpr float kAdcMaxCount = 1024.0f;

  MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>&
      battery_median_filter_;
  uint32_t pin_;
};
