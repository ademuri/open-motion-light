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

#include <cstdint>
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

uint32_t ExternalTempSensor::CalculateResistance(int pin_value,
                                                 int battery_mv) {
  if (pin_value <= 0 || battery_mv <= 0) {
    return 0;
  }

  // R = (pin_value / kAdcMaxCount) * battery_mv * 1000 / kBiasCurrentMicroamps
  // R = (pin_value * battery_mv * 1000) / (1024 * 38)
  //   = (pin_value * battery_mv * 125) / (128 * 38)
  //   = (pin_value * battery_mv * 125) / 4864
  // Round to nearest integer Ohm (+ 2432):
  const int64_t product = static_cast<int64_t>(pin_value) * battery_mv * 125;
  return static_cast<uint32_t>((product + 2432) / 4864);
}

int32_t ExternalTempSensor::FixedLnQ16(int32_t x_q16) {
  if (x_q16 <= 0) {
    return std::numeric_limits<int32_t>::min();
  }

  // Find k such that x = m * 2^k where m is in [65536, 131071] (i.e. [1.0,
  // 2.0)).
  int32_t k = 0;
  int32_t val = x_q16;
  while (val >= 131072) {
    val >>= 1;
    k++;
  }
  while (val < 65536) {
    val <<= 1;
    k--;
  }

  // Let z = (m - 1) / (m + 1). In Q16:
  // val represents m * 65536, and 1.0 is 65536.
  const int64_t num = static_cast<int64_t>(val - 65536) << 16;
  const int32_t den = val + 65536;
  const int64_t z = num / den;  // z in Q16, z in [0, 1/3)

  // Series expansion: ln(m) = 2 * (z + z^3/3 + z^5/5)
  const int64_t z2 = (z * z) >> 16;
  const int64_t term3 = ((z * z2) >> 16) / 3;
  const int64_t z4 = (z2 * z2) >> 16;
  const int64_t term5 = ((z * z4) >> 16) / 5;
  const int32_t ln_m = static_cast<int32_t>(2 * (z + term3 + term5));

  // ln(x) = k * ln(2) + ln(m)
  // ln(2) in Q16 is round(ln(2) * 65536) = 45426.
  constexpr int32_t kLn2Q16 = 45426;
  return (k * kLn2Q16) + ln_m;
}

int32_t ExternalTempSensor::CalculateTemperatureCenticelsius(
    uint32_t resistance_ohm) {
  if (resistance_ohm == 0) {
    return std::numeric_limits<int32_t>::min();
  }

  // Ratio (R / R0) in Q16:
  // R0 = 10,000 Ohm.
  // ratio_q16 = (R * 65536) / 10000 = (R * 4096) / 625.
  const int64_t ratio_q16 = (static_cast<int64_t>(resistance_ohm) * 4096) / 625;
  if (ratio_q16 > std::numeric_limits<int32_t>::max()) {
    // Open circuit / near absolute zero resistance ratio.
    return -kKelvinOffsetCentikelvin;
  }

  const int32_t ln_q16 = FixedLnQ16(static_cast<int32_t>(ratio_q16));
  if (ln_q16 == std::numeric_limits<int32_t>::min()) {
    return std::numeric_limits<int32_t>::min();
  }

  // Steinhart-Hart B-parameter equation:
  // 1 / T = 1 / T0 + (1 / B) * ln(R / R0)
  // T = (B * T0) / (B + T0 * ln(R / R0))
  // With T0 in centikelvin (29815 cK):
  // num = B * T0_cK * 65536 = 3950 * 29815 * 65536 = 7,718,683,443,200
  // den = B * 65536 + (T0_cK * ln_q16) / 100
  constexpr int64_t kNumerator = static_cast<int64_t>(kBetaCoefficient) *
                                 kNominalTemperatureCentikelvin * 65536;
  const int64_t den =
      (static_cast<int64_t>(kBetaCoefficient) * 65536) +
      ((static_cast<int64_t>(kNominalTemperatureCentikelvin) * ln_q16 + 50) /
       100);

  if (den <= 0) {
    return std::numeric_limits<int32_t>::max();
  }

  const int64_t temp_ck = (kNumerator + (den / 2)) / den;
  return static_cast<int32_t>(temp_ck - kKelvinOffsetCentikelvin);
}

int16_t ExternalTempSensor::CalculateTemperatureCelsius(
    uint32_t resistance_ohm) {
  const int32_t centicelsius = CalculateTemperatureCenticelsius(resistance_ohm);
  if (centicelsius == std::numeric_limits<int32_t>::min()) {
    return std::numeric_limits<int16_t>::min();
  }
  if (centicelsius == std::numeric_limits<int32_t>::max()) {
    return std::numeric_limits<int16_t>::max();
  }

  int32_t rounded;
  if (centicelsius >= 0) {
    rounded = (centicelsius + 50) / 100;
  } else {
    rounded = (centicelsius - 50) / 100;
  }

  if (rounded > std::numeric_limits<int16_t>::max()) {
    return std::numeric_limits<int16_t>::max();
  }
  if (rounded < std::numeric_limits<int16_t>::min()) {
    return std::numeric_limits<int16_t>::min();
  }
  return static_cast<int16_t>(rounded);
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

  const uint32_t resistance_ohm = CalculateResistance(pin_value, battery_mv);
  return CalculateTemperatureCelsius(resistance_ohm);
}
