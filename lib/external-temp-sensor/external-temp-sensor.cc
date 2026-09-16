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

#include "external-temp-sensor.h"

#include <cmath>
#include <limits>

ExternalTempSensor::ExternalTempSensor(
    MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>&
        battery_median_filter,
    uint32_t pin)
    : battery_median_filter_(battery_median_filter), pin_(pin) {}

bool ExternalTempSensor::Begin() {
  pinMode(pin_, INPUT_ANALOG);
  return true;
}

float ExternalTempSensor::CalculateResistance(float pin_value,
                                              float battery_mv) {
  if (pin_value <= 0.0f || battery_mv <= 0.0f) {
    return 0.0f;
  }

  // The BQ25170DSGR bias current is 38uA.
  // Voltage at pin (in mV) = (pin_value / kAdcMaxCount) * battery_mv
  const float voltage_mv = (pin_value * battery_mv) / kAdcMaxCount;

  // R = E / I
  // R (Ohms) = (voltage_mv / 1000) / (kBiasCurrentMicroamps / 1e6)
  //          = (voltage_mv * 1000) / kBiasCurrentMicroamps
  return (voltage_mv * 1000.0f) / kBiasCurrentMicroamps;
}

float ExternalTempSensor::CalculateTemperatureCelsius(float resistance_ohm) {
  if (resistance_ohm <= 0.0f) {
    return std::numeric_limits<float>::quiet_NaN();
  }

  // The Steinhart-Hart B-parameter equation:
  // 1 / T = 1 / T0 + (1 / B) * ln(R / R0)
  // where T is the temperature in Kelvin, R is the resistance, and R0 and T0
  // are the calibration values (10k Ohm at 25C).
  const float steinhart = (1.0f / kNominalTemperatureKelvin) +
                          (1.0f / kBetaCoefficient) *
                              std::log(resistance_ohm / kNominalResistanceOhm);

  if (steinhart <= 0.0f) {
    return std::numeric_limits<float>::infinity();
  }

  const float temperature_kelvin = 1.0f / steinhart;
  return temperature_kelvin - 273.15f;
}

int16_t ExternalTempSensor::ReadTemperature() {
  uint16_t battery_mv = battery_median_filter_.GetFilteredValue();
  if (battery_mv == 0) {
    battery_mv = battery_median_filter_.GetRawValue();
  }
  if (battery_mv == 0) {
    return std::numeric_limits<int16_t>::min();
  }

  const int pin_value = analogRead(pin_);
  if (pin_value <= 0) {
    return std::numeric_limits<int16_t>::min();
  }

  const float resistance_ohm = CalculateResistance(
      static_cast<float>(pin_value), static_cast<float>(battery_mv));
  const float temperature_celsius = CalculateTemperatureCelsius(resistance_ohm);

  if (std::isnan(temperature_celsius) || std::isinf(temperature_celsius)) {
    return std::numeric_limits<int16_t>::min();
  }

  const float rounded = std::round(temperature_celsius);
  if (rounded > static_cast<float>(std::numeric_limits<int16_t>::max())) {
    return std::numeric_limits<int16_t>::max();
  }
  if (rounded < static_cast<float>(std::numeric_limits<int16_t>::min())) {
    return std::numeric_limits<int16_t>::min();
  }

  return static_cast<int16_t>(rounded);
}
