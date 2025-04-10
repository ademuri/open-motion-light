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

#include "serial-manager.h"

#include <gtest/gtest.h>

#include "config-storage.h"
#include "controller.h"
#include "fake-power-controller.h"
#include "fake-serial-port.h"
#include "fake-vcnl4020.h"
#include "types.h"

namespace {

class SerialManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    serial_port.Reset();
    controller.SetConfig({
      version : 1,
      brightnessMode : BrightnessMode::BRIGHTNESS_MODE_DISABLED,
      autoBrightnessThreshold : 4 * 30,
      proximity_mode : ProximityMode::PROXIMITY_MODE_DISABLED,
      proximity_toggle_timeout_seconds : 10 * 60,
      proximity_threshold : 300,
    });
  }

  FakeSerialPort serial_port;
  FakeVCNL4020 vcnl4020;
  FakePowerController power_controller;
  Controller controller{&vcnl4020, &power_controller};
  SerialManager serial_manager{&serial_port, &controller};
};

TEST_F(SerialManagerTest, RoundTripConfig) {
  ConfigPb config;
  config.version = 1;
  config.proximity_threshold = 182;
  config.proximity_mode = ProximityMode::PROXIMITY_MODE_TOGGLE;
  config.brightnessMode = BrightnessMode::BRIGHTNESS_MODE_ON_WHEN_BELOW;
  config.autoBrightnessThreshold = 123;
  config.proximity_toggle_timeout_seconds = 1234;

  SerialRequest request;
  request.has_config = true;
  request.config = config;

  serial_port.WritePb(SerialRequest_msg, request);
  serial_manager.Step();

  SerialResponse response;
  serial_port.ReadPb(SerialResponse_msg, &response);

  EXPECT_FALSE(response.has_config);
  EXPECT_TRUE(response.has_status);

  ConfigPb storedConfig;
  ASSERT_TRUE(ConfigStorage::TryLoadConfig(&storedConfig));
  EXPECT_EQ(storedConfig.version, 1);
  EXPECT_EQ(storedConfig.proximity_threshold, 182);
  EXPECT_EQ(storedConfig.proximity_mode,
            ProximityMode::PROXIMITY_MODE_TOGGLE);
  EXPECT_EQ(storedConfig.brightnessMode,
            BrightnessMode::BRIGHTNESS_MODE_ON_WHEN_BELOW);
  EXPECT_EQ(storedConfig.autoBrightnessThreshold, 123);
  EXPECT_EQ(storedConfig.proximity_toggle_timeout_seconds, 1234);

  EXPECT_EQ(controller.GetConfig()->version, 1);
  EXPECT_EQ(controller.GetConfig()->proximity_threshold, 182);
  EXPECT_EQ(controller.GetConfig()->proximity_mode,
            ProximityMode::PROXIMITY_MODE_TOGGLE);
  EXPECT_EQ(controller.GetConfig()->brightnessMode,
            BrightnessMode::BRIGHTNESS_MODE_ON_WHEN_BELOW);
  EXPECT_EQ(controller.GetConfig()->autoBrightnessThreshold, 123);
  EXPECT_EQ(controller.GetConfig()->proximity_toggle_timeout_seconds, 1234);

  serial_port.Reset();
  request.request_config = true;
  request.has_request_config = true;
  serial_port.WritePb(SerialRequest_msg, request);
  serial_manager.Step();
  serial_port.ReadPb(SerialResponse_msg, &response);

  EXPECT_TRUE(response.has_config);
  EXPECT_EQ(response.config.version, 1);
  EXPECT_EQ(response.config.proximity_threshold, 182);
  EXPECT_EQ(response.config.proximity_mode,
            ProximityMode::PROXIMITY_MODE_TOGGLE);
  EXPECT_EQ(response.config.brightnessMode,
            BrightnessMode::BRIGHTNESS_MODE_ON_WHEN_BELOW);
  EXPECT_EQ(response.config.autoBrightnessThreshold, 123);
  EXPECT_EQ(response.config.proximity_toggle_timeout_seconds, 1234);
}

TEST_F(SerialManagerTest, ReturnsStatus) {
  controller.Init();
  controller.Step();
  setAnalogRead(AVREF, kFakeVrefintCal * 0.75 / 4);
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 4000);
  for (uint32_t n = 0; n < 100; n++) {
    advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
    controller.Step();
  }
  EXPECT_GT(controller.GetFilteredBatteryMillivolts(), 3800);

  SerialRequest request;
  serial_port.WritePb(SerialRequest_msg, request);
  serial_manager.Step();

  SerialResponse response;
  serial_port.ReadPb(SerialResponse_msg, &response);

  EXPECT_FALSE(response.has_config);
  EXPECT_TRUE(response.has_status);
  EXPECT_EQ(response.status.battery_voltage_millivolts,
            controller.GetFilteredBatteryMillivolts());
  EXPECT_STREQ(response.status.firmware_version, FIRMWARE_VERSION);
}

}  // namespace
