// Copyright 2026 Google LLC
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

#include "external-temp-sensor.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

#include "controller.h"
#include "test-lib.h"
#include "types.h"

namespace {

class ExternalTempSensorTest : public LightTest {
 protected:
  uint16_t battery_voltage_mv_ = 3200;
  MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>
      battery_filter_{[this]() { return battery_voltage_mv_; }};
};

TEST_F(ExternalTempSensorTest, InitializesPinOnBegin) {
  ExternalTempSensor sensor(battery_filter_);
  EXPECT_TRUE(sensor.Begin());
  EXPECT_EQ(getPinMode(kPinTempSensor), INPUT_ANALOG);
}

TEST_F(ExternalTempSensorTest, InitializesCustomPinOnBegin) {
  constexpr uint32_t kCustomPin = PA0;
  ExternalTempSensor sensor(battery_filter_, kCustomPin);
  EXPECT_TRUE(sensor.Begin());
  EXPECT_EQ(getPinMode(kCustomPin), INPUT_ANALOG);
}

TEST_F(ExternalTempSensorTest, CalculateResistanceHandlesInvalidInputs) {
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(0, 3200), 0u);
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(-10, 3200), 0u);
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(512, 0), 0u);
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(512, -3200), 0u);
}

TEST_F(ExternalTempSensorTest, CalculateResistanceNominalValues) {
  // At 25°C, R = 10,000 Ω.
  // With 38 µA bias current, V_pin = 38 µA * 10,000 Ω = 380 mV.
  // With battery = 3200 mV and 10-bit ADC (1024 counts):
  // pin_value = round((380 / 3200) * 1024) = 122 counts.
  // Resistance: (122 * 3200 * 125 + 2432) / 4864 = 10,033 Ω.
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(122, 3200), 10033u);

  // Cold threshold: 1.04 V at 38 µA -> 27,368.4 Ω.
  // pin_value = round((1040 / 3200) * 1024) = 333 counts.
  // Resistance: (333 * 3200 * 125 + 2432) / 4864 = 27,385 Ω.
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(333, 3200), 27385u);

  // Hot threshold: 188 mV at 38 µA -> 4,947.4 Ω.
  // pin_value = round((188 / 3200) * 1024) = 60 counts.
  // Resistance: (60 * 3200 * 125 + 2432) / 4864 = 4,934 Ω.
  EXPECT_EQ(ExternalTempSensor::CalculateResistance(60, 3200), 4934u);
}

TEST_F(ExternalTempSensorTest, CalculateTemperatureCenticelsiusInvalidInputs) {
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(0),
            std::numeric_limits<int32_t>::min());
}

TEST_F(ExternalTempSensorTest, CalculateTemperatureCenticelsiusKnownValues) {
  // Nominal temperature: 10,000 Ω -> exactly 25.00 °C (2500 centicelsius).
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(10000), 2500);

  // Cold threshold from DESIGN.md: 27,368 Ω -> 3.94 °C (394 centicelsius).
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(27368), 394);

  // Hot threshold from DESIGN.md: 4,947 Ω -> 41.73 °C (4173 centicelsius).
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(4947), 4173);

  // Freezing point (0°C): 33,620 Ω -> 0.00 °C (0 centicelsius).
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(33620), 0);

  // 50°C: 3,588 Ω -> 50.00 °C (5000 centicelsius).
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCenticelsius(3588), 5000);
}

TEST_F(ExternalTempSensorTest, CalculateTemperatureCelsiusKnownValues) {
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(0),
            std::numeric_limits<int16_t>::min());
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(10000), 25);
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(27368), 4);
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(4947), 42);
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(33620), 0);
  EXPECT_EQ(ExternalTempSensor::CalculateTemperatureCelsius(3588), 50);
}

TEST_F(ExternalTempSensorTest, ReadTemperatureReturnsErrorWhenUninitialized) {
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();
  // Battery filter has not been run yet, so battery mV is 0.
  setAnalogRead(kPinTempSensor, 122);
  EXPECT_EQ(sensor.ReadTemperature(), std::numeric_limits<int16_t>::min());
}

TEST_F(ExternalTempSensorTest, ReadTemperatureReturnsErrorWhenPinZero) {
  battery_voltage_mv_ = 3200;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();
  setAnalogRead(kPinTempSensor, 0);
  EXPECT_EQ(sensor.ReadTemperature(), std::numeric_limits<int16_t>::min());
}

TEST_F(ExternalTempSensorTest, ReadTemperatureNominal25Celsius) {
  battery_voltage_mv_ = 3200;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();

  // 10k Ω at 38 µA gives 380 mV.
  // ADC count: 380 * 1024 / 3200 = 121.6, round to 122.
  setAnalogRead(kPinTempSensor, 122);
  EXPECT_EQ(sensor.ReadTemperature(), 25);
}

TEST_F(ExternalTempSensorTest, ReadTemperatureColdThreshold) {
  battery_voltage_mv_ = 3200;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();

  // Cold threshold: 1.04V = 1040 mV -> ADC count: 1040 * 1024 / 3200 = 332.8 ->
  // 333.
  setAnalogRead(kPinTempSensor, 333);
  EXPECT_EQ(sensor.ReadTemperature(), 4);
}

TEST_F(ExternalTempSensorTest, ReadTemperatureHotThreshold) {
  battery_voltage_mv_ = 3200;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();

  // Hot threshold: 188 mV -> ADC count: 188 * 1024 / 3200 = 60.16 -> 60.
  setAnalogRead(kPinTempSensor, 60);
  EXPECT_EQ(sensor.ReadTemperature(), 42);
}

TEST_F(ExternalTempSensorTest,
       ReadTemperatureWorksWithDifferentBatteryVoltages) {
  // If battery voltage is 3600 mV:
  // 10k Ω at 38 µA gives 380 mV.
  // ADC count: 380 * 1024 / 3600 = 108.08 -> 108.
  battery_voltage_mv_ = 3600;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_);
  sensor.Begin();

  setAnalogRead(kPinTempSensor, 108);
  EXPECT_EQ(sensor.ReadTemperature(), 25);
}

TEST_F(ExternalTempSensorTest, ReadTemperatureWithCustomPin) {
  constexpr uint32_t kCustomPin = PA3;
  battery_voltage_mv_ = 3200;
  battery_filter_.Run();
  ExternalTempSensor sensor(battery_filter_, kCustomPin);
  sensor.Begin();

  setAnalogRead(kCustomPin, 122);
  EXPECT_EQ(sensor.ReadTemperature(), 25);
}

TEST_F(ExternalTempSensorTest, UsesFilteredBatteryVoltage) {
  // Verify that the sensor uses the median-filtered battery voltage,
  // rejecting noisy outliers in the battery voltage.
  uint16_t read_count = 0;
  MedianFilter<uint16_t, uint16_t, Controller::kBatteryMedianFilterSize>
      noisy_battery_filter([&read_count]() -> uint16_t {
        read_count++;
        // Sequence with an outlier: 3200, 3200, 5000 (spike), 3200, 3200.
        if (read_count == 3) {
          return 5000;
        }
        return 3200;
      });

  for (uint8_t i = 0; i < Controller::kBatteryMedianFilterSize; i++) {
    noisy_battery_filter.Run();
  }
  // Despite the 5000 mV outlier, the median filtered value is 3200 mV.
  EXPECT_EQ(noisy_battery_filter.GetFilteredValue(), 3200);

  ExternalTempSensor sensor(noisy_battery_filter);
  sensor.Begin();
  setAnalogRead(kPinTempSensor, 122);
  EXPECT_EQ(sensor.ReadTemperature(), 25);
}

}  // namespace
