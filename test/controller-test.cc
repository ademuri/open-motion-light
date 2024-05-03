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
  setMillis(0);
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
  setMillis(0);
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

TEST(ControllerTest, MotionTriggersLed) {
  setMillis(0);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 + 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinMotionSensor, true);
  controller.Step();
  const uint32_t duty_cycle = controller.GetLedDutyCycle();
  ASSERT_GT(duty_cycle, 0);
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  setDigitalRead(kPinMotionSensor, false);

  advanceMillis(10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 - 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  advanceMillis(1);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
}

TEST(ControllerTest, ContinuedMotionKeepsLedOn) {
  setMillis(0);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 + 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinMotionSensor, true);
  controller.Step();
  const uint32_t duty_cycle = controller.GetLedDutyCycle();
  ASSERT_GT(duty_cycle, 0);
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  setDigitalRead(kPinMotionSensor, false);

  advanceMillis(10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 - 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  setDigitalRead(kPinMotionSensor, true);
  advanceMillis(1);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 * 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  setDigitalRead(kPinMotionSensor, false);
  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  advanceMillis(1);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
}
