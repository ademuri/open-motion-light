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

#include "config-storage.h"

#include <gtest/gtest.h>

#include "types.h"

class ConfigStorageTest : public ::testing::Test {
 protected:
  void SetUp() override { EEPROM.reset(); }

  ConfigStorage configStorage;
  ConfigPb config;
};

TEST_F(ConfigStorageTest, RoundTripSaveAndLoadSucceeds) {
  config.version = 1;
  config.proximity_threshold = 182;
  EXPECT_FALSE(configStorage.TryLoadConfig(&config));
  EXPECT_TRUE(configStorage.SaveConfig(&config));
  ASSERT_TRUE(configStorage.TryLoadConfig(&config)) << EEPROM;
  EXPECT_EQ(1, config.version) << EEPROM;
  EXPECT_EQ(182, config.proximity_threshold) << EEPROM;
}

TEST_F(ConfigStorageTest, TryLoadConfigReturnsFalseForMagicNumber0Incorrect) {
  configStorage.SaveConfig(&config);
  ASSERT_TRUE(configStorage.TryLoadConfig(&config));

  EEPROM.put(0, 0);
  EXPECT_FALSE(configStorage.TryLoadConfig(&config));
}

TEST_F(ConfigStorageTest, TryLoadConfigReturnsFalseForMagicNumber1Incorrect) {
  configStorage.SaveConfig(&config);
  ASSERT_TRUE(configStorage.TryLoadConfig(&config));

  EEPROM.put(sizeof(ConfigStorage::kEEPROMMagicByte1), 0);
  EXPECT_FALSE(configStorage.TryLoadConfig(&config));
}

TEST_F(ConfigStorageTest, TryLoadConfigReturnsFalseForVersionIncorrect) {
  configStorage.SaveConfig(&config);
  ASSERT_TRUE(configStorage.TryLoadConfig(&config));

  EEPROM.put(2 * sizeof(ConfigStorage::kEEPROMMagicByte1), 0);
  ASSERT_NE(0, ConfigStorage::kConfigVersion);
  EXPECT_FALSE(configStorage.TryLoadConfig(&config));
}
