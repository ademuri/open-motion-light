#include <Arduino.h>
#include <Wire.h>
#include <arduino-timer.h>

#include "arduino-vcnl4010.cc"
#include "clock.h"
#include "controller.h"
#include "pins.h"

// This enables printing values for the VCNL4010 to the serial console.
// #define DEBUG_VCNL4010

CountDownTimer vcnl4010_timer{200};

ArduinoVCNL4010 vcnl4010;
Controller controller{&vcnl4010};

void setup() {
#ifdef DEBUG_VCNL4010
  Serial2.begin(115200);
#endif  // ifdef DEBUG_VCNL4010

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
  // analogWriteFrequency(125 * 1000);
  // Using a lower frequency for now because we're limited by the 1MHz core
  // frequency: 1_024_000 / 256 = 4000 Hz max.
  analogWriteFrequency(4000);
  analogWriteResolution(8);

  // TODO: handle failure
  controller.Init();

  vcnl4010_timer.Reset();
}

void loop() {
  controller.Step();
  delay(1);

#ifdef DEBUG_VCNL4010
  if (vcnl4010_timer.Expired()) {
    Serial2.printf("ambient: %5u,   proximity: %5u\n",
                   controller.ReadAmbientLight(), controller.ReadProximity());
    vcnl4010_timer.Reset();
  }
#endif  // ifdef DEBUG_VCNL4010
}
