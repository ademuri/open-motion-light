#include "controller.h"

#include <gtest/gtest.h>

#include "pins.h"
#include "test-lib.h"
#include "types.h"

using PowerMode = Controller::PowerMode;

TEST(ControllerTest, Initializes) {
  Controller controller;
  EXPECT_TRUE(controller.Init());
}

TEST(ControllerTest, SetsPowerModeFromSwitch) {
  // These pins are inverted
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOff);

  setDigitalRead(kPinPowerAuto, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kAuto);

  setDigitalRead(kPinPowerOn, false);
  controller.Step();
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOn);

  setDigitalRead(kPinPowerAuto, true);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOn);

  setDigitalRead(kPinPowerOn, true);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOff);
}

TEST(ControllerTest, DebouncePowerMode) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  ASSERT_EQ(controller.GetPowerMode(), PowerMode::kOff);

  setDigitalRead(kPinPowerAuto, false);
  controller.Step();
  ASSERT_EQ(controller.GetPowerMode(), PowerMode::kAuto);

  setDigitalRead(kPinPowerOn, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kAuto);
  advanceMillis(10);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kAuto);
  advanceMillis(1);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOn);

  setDigitalRead(kPinPowerOn, true);
  advanceMillis(10);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kOn);
  advanceMillis(1);
  controller.Step();
  EXPECT_EQ(controller.GetPowerMode(), PowerMode::kAuto);
}
