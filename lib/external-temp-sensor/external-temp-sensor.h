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

#include <cstdint>

#include "controller.h"
#include "pins.h"
#include "temperature-sensor.h"

// Implementation of the temperature sensor HAL using an external NTC
// thermistor biased by a constant-current source (such as the BQ25170DSGR
// charger TS pin), using fixed-point arithmetic for performance and small code
// size.
class ExternalTempSensor : public TemperatureSensor {
 public:
  explicit ExternalTempSensor(
      MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>&
          battery_median_filter,
      uint32_t pin = kPinTempSensor);

  bool Begin() override;
  int16_t ReadTemperature() override;

  // Resistance in Ohms calculated from the analog pin reading and battery
  // voltage (in mV) using fixed-point integer arithmetic.
  static uint32_t CalculateResistance(int pin_value, int battery_mv);

  // Temperature in hundredths of a degree Celsius (centicelsius, 0.01°C)
  // calculated from resistance using a fixed-point Steinhart-Hart equation.
  static int32_t CalculateTemperatureCenticelsius(uint32_t resistance_ohm);

  // Temperature in degrees Celsius (rounded to nearest integer) calculated
  // using fixed-point arithmetic.
  static int16_t CalculateTemperatureCelsius(uint32_t resistance_ohm);

 private:
  static constexpr int32_t kNominalTemperatureCentikelvin = 29815;
  static constexpr uint32_t kNominalResistanceOhm = 10000;
  static constexpr int32_t kBetaCoefficient = 3950;
  static constexpr int32_t kBiasCurrentMicroamps = 38;
  static constexpr int32_t kAdcMaxCount = 1024;
  static constexpr int32_t kKelvinOffsetCentikelvin = 27315;

  // Computes the natural logarithm in Q16.16 fixed-point format for a Q16.16
  // input.
  static int32_t FixedLnQ16(int32_t x_q16);

  MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>&
      battery_median_filter_;
  uint32_t pin_;
};
