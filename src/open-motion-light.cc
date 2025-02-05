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

#include <Arduino.h>
#include <Wire.h>
#include <arduino-timer.h>
#include <edge-filter.h>

#include "arduino-vcnl4020.h"
#include "clock.h"
#include "controller.h"
#include "pins.h"
#include "stm32-power-controller.h"

// This enables printing values for the VCNL4020 to the serial console.
// #define DEBUG_VCNL4020_BRIGHTNESS

#define DEBUG_VCNL4020_PROXIMITY

CountDownTimer vncl4020_timer{200};

ArduinoVCNL4020 vcnl4020;

Stm32PowerController power_controller;
Controller controller{&vcnl4020, &power_controller};

#ifdef DEBUG_VCNL4020_PROXIMITY
constexpr uint16_t proximity_threshold = 5;
EdgeFilter proximity_edge_filter([]() { return vcnl4020.ReadProximity(); }, /*alpha=*/ 255);
#endif  // DEBUG_VCNL4020_PROXIMITY

void setup() {
  Serial1.begin(115200);
  Serial1.println("Booting...");

  // USB pins
  pinMode(kPinCc1, INPUT_ANALOG);
  pinMode(kPinCc2, INPUT_ANALOG);

  // UI pins
  pinMode(kPinBatteryLed1, OUTPUT);
  pinMode(kPinBatteryLed2, OUTPUT);
  pinMode(kPinBatteryLed3, OUTPUT);
  pinMode(kPinWhiteLed, OUTPUT);

  // Sensors
  pinMode(kPinMotionSensor, INPUT);
  pinMode(kPinLightSensorInterrupt, INPUT);

  // Power switch
  pinMode(kPinPowerAuto, INPUT);
  pinMode(kPinPowerOn, INPUT);

  // Battery management
  pinMode(kPinBatteryCharge, INPUT);
  pinMode(kPinBatteryDone, INPUT);
  pinMode(kPinChargeHighCurrentEnable, OUTPUT);
  pinMode(kPin5vDetect, INPUT_PULLDOWN);

  // Indicate that the program has started.
  digitalWrite(kPinBatteryLed3, true);
  delay(100);
  digitalWrite(kPinBatteryLed3, false);

  // I2C - used for light sensor
  Wire.setSCL(kPinScl);
  Wire.setSDA(kPinSda);
  Wire.begin();
  Wire.setClock(400 * 1000);

  // From the datasheet for the MT9284BS6 LED driver, its recommended PWM
  // frequency is 20kHz < n < 1MHz.
  analogWriteFrequency(20 * 1000);
  analogWriteResolution(8);

  if (!controller.Init()) {
    while (true) {
      uint8_t brightness = (millis() / 100) % 2;
      analogWrite(kPinBatteryLed1, brightness);
      analogWrite(kPinBatteryLed2, brightness);
      analogWrite(kPinBatteryLed3, brightness);
    }
  }

#ifdef DEBUG_VCNL4020_BRIGHTNESS
  vcnl4020.SetPeriodicAmbient(true);
  vncl4020_timer.Reset();
#endif  // DEBUG_VCNL4020_BRIGHTNESS

#ifdef DEBUG_VCNL4020_PROXIMITY
  vcnl4020.SetPeriodicProximity(true);
  vncl4020_timer.Reset();
#endif  // DEBUG_VCNL4020_PROXIMITY

  digitalWrite(kPinBatteryLed2, true);
  delay(100);
  digitalWrite(kPinBatteryLed2, false);
}

void loop() {
  controller.Step();
  delay(1);

#ifdef DEBUG_VCNL4020_BRIGHTNESS
  if (vncl4020_timer.Expired()) {
    if (vcnl4020.AmbientReady()) {
      Serial1.printf("ambient: %5u\n", vcnl4020.ReadAmbient());
    }
    vncl4020_timer.Reset();
  }
#endif  // ifdef DEBUG_VCNL4020_BRIGHTNESS

#ifdef DEBUG_VCNL4020_PROXIMITY
  if (vcnl4020.ProximityReady()) {
    edge_filter.Run();
    Serial1.printf("rising: %u, falling: %u, value: %u, slope: %d\n",
                   edge_filter.Rising(threshold),
                   edge_filter.Falling(threshold),
                   edge_filter.GetFilteredValue(), edge_filter.Slope());
  }
  // if (vcnl4020.ProximityReady()) {
  //   Serial1.printf("proximity: %5u\n", vcnl4020.ReadProximity());
  // }
#endif  // ifdef DEBUG_VCNL4020_PROXIMITY
}
