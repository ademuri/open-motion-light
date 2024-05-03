#include "controller.h"

#include <gtest/gtest.h>

#include "pins.h"
#include "test-lib.h"
#include "types.h"

using PowerMode = Controller::PowerMode;

class ControllerTest : public LightTest {
  void SetUp() override {
    LightTest::SetUp();
    setDigitalRead(kPinPowerAuto, true);
    setDigitalRead(kPinPowerOn, true);
  }
};

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

TEST(ControllerTest, UpdatesLedWhenSwitchingModes) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  const uint32_t duty_cycle = controller.GetLedDutyCycle();
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinPowerAuto, false);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  setDigitalRead(kPinPowerOn, false);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinMotionSensor, true);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinPowerAuto, false);
  setDigitalRead(kPinMotionSensor, false);
  advanceMillis(100);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle)
      << "Switching to auto should turn on light";

  setDigitalRead(kPinPowerOn, false);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  setDigitalRead(kPinPowerOn, true);
  advanceMillis(11);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
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

TEST(ControllerTest, MotionTriggersLed) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  const uint32_t duty_cycle = controller.GetLedDutyCycle();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinPowerAuto, false);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);
  advanceMillis(10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinMotionSensor, true);
  controller.Step();
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
  setDigitalRead(kPinPowerAuto, false);
  setDigitalRead(kPinPowerOn, true);

  Controller controller;
  ASSERT_TRUE(controller.Init());
  const uint32_t duty_cycle = controller.GetLedDutyCycle();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), duty_cycle);

  advanceMillis(controller.GetMotionTimeoutSeconds() * 1000 + 10);
  controller.Step();
  EXPECT_EQ(getAnalogWrite(kPinWhiteLed), 0);

  setDigitalRead(kPinMotionSensor, true);
  controller.Step();
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
