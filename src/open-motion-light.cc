#include <Arduino.h>
#include <Wire.h>
#include <arduino-timer.h>

#include "pins.h"

PowerMode power_mode = PowerMode::kOff;
CountDownTimer power_mode_read_timer{10};

void setup() {
  // USB pins
  pinMode(kPinCc1, INPUT_ANALOG);
  pinMode(kPinCc2, INPUT_ANALOG);
  pinMode(kPinSerialTx, OUTPUT);
  pinMode(kPinSerialRx, INPUT);

  // UI pins
  pinMode(kPinBatteryLed1, OUTPUT);
  pinMode(kPinBatteryLed2, OUTPUT);
  pinMode(kPinBatteryLed3, OUTPUT);
  pinMode(kPinSw1, INPUT_PULLUP);
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

  // I2C - used for light sensor
  Wire.setSCL(kPinScl);
  Wire.setSDA(kPinSda);
  Wire.begin();
  Wire.setClock(400 * 1000);

  // From the datasheet for the MT9284BS6 LED driver, its recommended PWM frequency is 20kHz < n < 1MHz.
  analogWriteFrequency(50 * 1000);
  analogWriteResolution(8);
}

void loop() {

}
