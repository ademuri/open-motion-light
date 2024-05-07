#include "controller.h"

#include <gtest/gtest.h>

#include "fake-vcnl4010.h"
#include "pins.h"
#include "test-lib.h"
#include "types.h"

// Returns an analog value which approximates the millivolt reading. Note: due
// to precision limits, this is NOT exact.
uint16_t ComputeAnalogValueForMillivolts(uint32_t millivolts) {
  const uint32_t battery_millivolts = Controller::ReadRawBatteryMillivolts();
  EXPECT_LE(millivolts, battery_millivolts);
  return (millivolts * Controller::kAdcConfiguredMaxCount) / battery_millivolts;
}

TEST(ControllerTestFunctionsTest, ComputeAnalogValueForMillivolts) {
  const uint32_t pin = PA0;

  setAnalogRead(AVREF, kFakeVrefintCal * 0.75 / 4);
  const uint16_t battery_millivolts = 4000;
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), battery_millivolts);

  const uint16_t value_1000 = ComputeAnalogValueForMillivolts(1000);
  setAnalogRead(pin, value_1000);
  EXPECT_EQ(Controller::ReadAnalogVoltageMillivolts(pin, battery_millivolts),
            1000);

  const uint16_t value_250 = ComputeAnalogValueForMillivolts(250);
  setAnalogRead(pin, value_250);
  EXPECT_EQ(Controller::ReadAnalogVoltageMillivolts(pin, battery_millivolts),
            250);
}

class ControllerTest : public LightTest {
 protected:
  void SetUp() override {
    LightTest::SetUp();
    setDigitalRead(kPinPowerAuto, true);
    setDigitalRead(kPinPowerOn, true);
    setDigitalRead(kPinBatteryCharge, true);
    setDigitalRead(kPinBatteryDone, true);

    // This yields a voltage of 3000 / 0.85 = 3529mV.
    setAnalogRead(AVREF, kFakeVrefintCal * 0.85 / 4);
    ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 3529);
  }

  FakeVCNL4010 vcnl4010;
  Controller controller{&vcnl4010};
};

TEST_F(ControllerTest, Initializes) { EXPECT_TRUE(controller.Init()); }

TEST_F(ControllerTest, SetsPowerModeFromSwitch) {
  // These pins are inverted
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

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

TEST_F(ControllerTest, UpdatesLedWhenSwitchingModes) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

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

TEST_F(ControllerTest, DebouncePowerMode) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

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

TEST_F(ControllerTest, MotionTriggersLed) {
  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, true);

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

TEST_F(ControllerTest, ContinuedMotionKeepsLedOn) {
  setDigitalRead(kPinPowerAuto, false);
  setDigitalRead(kPinPowerOn, true);

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

TEST_F(ControllerTest, ReadsBatteryVoltage) {
  ASSERT_TRUE(controller.Init());

  setAnalogRead(AVREF, kFakeVrefintCal / 4);
  EXPECT_EQ(Controller::ReadRawBatteryMillivolts(), 3000);

  setAnalogRead(AVREF, kFakeVrefintCal * 0.75 / 4);
  EXPECT_EQ(Controller::ReadRawBatteryMillivolts(), 4000);

  setAnalogRead(AVREF, kFakeVrefintCal * 1.5 / 4);
  EXPECT_EQ(Controller::ReadRawBatteryMillivolts(), 2000);
}

TEST_F(ControllerTest, FiltersBatteryVoltage) {
  setAnalogRead(AVREF, kFakeVrefintCal / 4);

  ASSERT_TRUE(controller.Init());
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 3000);
  EXPECT_EQ(controller.GetFilteredBatteryMillivolts(), 3000);

  advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
  controller.Step();
  EXPECT_EQ(controller.GetFilteredBatteryMillivolts(), 3000);

  advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
  setAnalogRead(AVREF, kFakeVrefintCal * 0.75 / 4);
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 4000);
  controller.Step();
  EXPECT_GE(controller.GetFilteredBatteryMillivolts(), 3000);
  EXPECT_LT(controller.GetFilteredBatteryMillivolts(), 4000);

  for (uint32_t n = 0; n < (256 / Controller::kBatteryFilterAlpha) * 2; n++) {
    advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
    controller.Step();
    EXPECT_GE(controller.GetFilteredBatteryMillivolts(), 3000) << "step " << n;
    EXPECT_LT(controller.GetFilteredBatteryMillivolts(), 4000) << "step " << n;
  }
  EXPECT_GT(controller.GetFilteredBatteryMillivolts(), 3800);
}

TEST_F(ControllerTest, SetsPowerStatus) {
  setDigitalRead(kPinBatteryCharge, true);
  setDigitalRead(kPinBatteryDone, true);
  setAnalogRead(kPinCc1, ComputeAnalogValueForMillivolts(0));

  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kBattery);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kBattery);

  setDigitalRead(kPinBatteryCharge, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kCharging);

  setDigitalRead(kPinBatteryCharge, true);
  setDigitalRead(kPinBatteryDone, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kCharged);

  setDigitalRead(kPinBatteryCharge, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kChargeError);

  setDigitalRead(kPinBatteryCharge, true);
  setDigitalRead(kPinBatteryDone, true);
  controller.Step();
  ASSERT_EQ(controller.GetPowerStatus(), PowerStatus::kBattery);

  setAnalogRead(kPinCc1, ComputeAnalogValueForMillivolts(1000));
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kChargeError);
}

TEST_F(ControllerTest, DetectsLowBatteryVoltage) {
  setDigitalRead(kPinBatteryCharge, true);
  setDigitalRead(kPinBatteryDone, true);

  setAnalogRead(AVREF, kFakeVrefintCal * 1.5 / 4);
  ASSERT_EQ(controller.ReadRawBatteryMillivolts(), 2000);

  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kBattery);
  controller.Step();
  ASSERT_EQ(controller.GetFilteredBatteryMillivolts(), 2000);
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kLowBatteryCutoff);

  setDigitalRead(kPinBatteryCharge, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(),
            PowerStatus::kLowBatteryCutoffCharging);

  setDigitalRead(kPinBatteryDone, false);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kLowBatteryCutoff);

  setDigitalRead(kPinBatteryCharge, true);
  controller.Step();
  EXPECT_EQ(controller.GetPowerStatus(), PowerStatus::kLowBatteryCutoff);
}

TEST_F(ControllerTest, SetsUSBStatus) {
  // Battery voltage should be max ~3.6V.
  setAnalogRead(AVREF, kFakeVrefintCal * 0.8 / 4);
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 3750);

  const uint16_t vusb_min = ComputeAnalogValueForMillivolts(200);
  const uint16_t vusb_max = ComputeAnalogValueForMillivolts(610);
  const uint16_t v1_5_min = ComputeAnalogValueForMillivolts(700);
  const uint16_t v1_5_max = ComputeAnalogValueForMillivolts(1160);
  const uint16_t v3_0_min = ComputeAnalogValueForMillivolts(1310);
  const uint16_t v3_0_max = ComputeAnalogValueForMillivolts(2040);
  setAnalogRead(kPinCc1, 0);
  setAnalogRead(kPinCc2, 0);

  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kNoConnection);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kNoConnection);
  EXPECT_FALSE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, vusb_min - 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kNoConnection);
  EXPECT_FALSE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, vusb_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kStandardUsb);
  EXPECT_FALSE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, vusb_max - 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kStandardUsb);
  EXPECT_FALSE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, v1_5_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB1_5);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, v1_5_max - 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB1_5);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, v3_0_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB3_0);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, v3_0_max - 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB3_0);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));
}

TEST_F(ControllerTest, SetsUSBFromCC1AndCC2) {
  // Battery voltage should be max ~3.6V.
  setAnalogRead(AVREF, kFakeVrefintCal * 0.8 / 4);
  ASSERT_EQ(Controller::ReadRawBatteryMillivolts(), 3750);

  const uint16_t vusb_max = ComputeAnalogValueForMillivolts(610);
  const uint16_t v1_5_min = ComputeAnalogValueForMillivolts(700);
  setAnalogRead(kPinCc1, 0);
  setAnalogRead(kPinCc2, 0);

  ASSERT_TRUE(controller.Init());
  controller.Step();
  ASSERT_EQ(controller.GetUSBStatus(), USBStatus::kNoConnection);
  EXPECT_FALSE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, v1_5_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB1_5);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, 0);
  setAnalogRead(kPinCc2, v1_5_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB1_5);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));

  setAnalogRead(kPinCc1, vusb_max - 1);
  setAnalogRead(kPinCc2, v1_5_min + 1);
  controller.Step();
  EXPECT_EQ(controller.GetUSBStatus(), USBStatus::kUSB1_5);
  EXPECT_TRUE(getDigitalWrite(kPinChargeHighCurrentEnable));
}

TEST_F(ControllerTest, ReadsAnalogVoltage) {
  const uint32_t pin = PA0;
  setAnalogRead(pin, Controller::kAdcConfiguredMaxCount);
  EXPECT_EQ(
      Controller::ReadAnalogVoltageMillivolts(pin, /*battery_millivolts=*/3000),
      3000);

  setAnalogRead(pin, Controller::kAdcConfiguredMaxCount / 2);
  EXPECT_EQ(
      Controller::ReadAnalogVoltageMillivolts(pin, /*battery_millivolts=*/3000),
      1500);
}

std::array<uint32_t, 3> BatteryLeds(uint32_t led1, uint32_t led2,
                                    uint32_t led3) {
  return {led1, led2, led3};
}

std::array<uint32_t, 3> GetBatteryLeds() {
  return BatteryLeds(getAnalogWrite(kPinBatteryLed1),
                     getAnalogWrite(kPinBatteryLed2),
                     getDigitalWrite(kPinBatteryLed3));
}

TEST_F(ControllerTest, DisplaysBatteryVoltage) {
  setDigitalRead(kPinBatteryCharge, true);
  setDigitalRead(kPinBatteryDone, true);

  setAnalogRead(AVREF,
                (kFakeVrefintCal * Controller::kReferenceSupplyMillivolts) /
                    (Controller::kBatteryVoltage1 + 100) / 4);
  ASSERT_GT(controller.ReadRawBatteryMillivolts(),
            Controller::kBatteryVoltage1);
  ASSERT_TRUE(controller.Init());
  EXPECT_EQ(GetBatteryLeds(), BatteryLeds(0, 0, 0));

  setDigitalRead(kPinPowerAuto, false);
  controller.Step();
  ASSERT_EQ(controller.GetPowerMode(), PowerMode::kAuto);
  EXPECT_EQ(GetBatteryLeds(), BatteryLeds(255, 255, 1));

  advanceMillis(Controller::kBatteryLevelDisplayTimeSeconds * 1000 + 10);
  controller.Step();
  EXPECT_EQ(GetBatteryLeds(), BatteryLeds(0, 0, 0));

  // Medium voltage
  setAnalogRead(AVREF,
                (kFakeVrefintCal * Controller::kReferenceSupplyMillivolts) /
                    (Controller::kBatteryVoltage0 + 10) / 4);
  ASSERT_GT(controller.ReadRawBatteryMillivolts(),
            Controller::kBatteryVoltage0);
  ASSERT_LT(controller.ReadRawBatteryMillivolts(),
            Controller::kBatteryVoltage1);
  // Pump the battery voltage filters
  for (uint32_t n = 0; n < 100; n++) {
    advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
    controller.Step();
  }

  setDigitalRead(kPinPowerAuto, true);
  setDigitalRead(kPinPowerOn, false);
  controller.Step();
  ASSERT_EQ(controller.GetPowerMode(), PowerMode::kOn);
  EXPECT_EQ(
      GetBatteryLeds(),
      BatteryLeds(Controller::kBatteryLedPlaceholderBrightness, 255, 1));

  // Low voltage
  setAnalogRead(AVREF,
                (kFakeVrefintCal * Controller::kReferenceSupplyMillivolts) /
                    (Controller::kBatteryVoltage0 - 10) / 4);
  ASSERT_LT(controller.ReadRawBatteryMillivolts(),
            Controller::kBatteryVoltage0);
  // Pump the battery voltage filters
  for (uint32_t n = 0; n < 100; n++) {
    advanceMillis(Controller::kBatteryFilterRunIntervalMillis + 1);
    controller.Step();
  }

  setDigitalRead(kPinPowerAuto, false);
  setDigitalRead(kPinPowerOn, true);
  controller.Step();
  ASSERT_EQ(controller.GetPowerMode(), PowerMode::kAuto);
  EXPECT_EQ(GetBatteryLeds(),
            BatteryLeds(Controller::kBatteryLedPlaceholderBrightness,
                        Controller::kBatteryLedPlaceholderBrightness, 1));
}

TEST_F(ControllerTest, AmbientAndProximitySensorsWork) {
  ASSERT_TRUE(controller.Init());

  vcnl4010.SetAmbient(500);
  EXPECT_EQ(controller.ReadAmbientLight(), 500);

  vcnl4010.SetProximity(600);
  EXPECT_EQ(controller.ReadProximity(), 600);
}
