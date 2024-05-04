#include <Arduino.h>
#include <Wire.h>
#include <arduino-timer.h>

#include "arduino-vcnl4010.cc"
#include "controller.h"
#include "pins.h"

constexpr bool kDebugVCNL4010 = true;
CountDownTimer vcnl4010_timer{200};

ArduinoVCNL4010 vcnl4010;
Controller controller{&vcnl4010};

void setup() {
  Serial2.begin(115200);

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

  // From the datasheet for the MT9284BS6 LED driver, its recommended PWM
  // frequency is 20kHz < n < 1MHz.
  analogWriteFrequency(50 * 1000);
  analogWriteResolution(8);

  // TODO: handle failure
  controller.Init();

  vcnl4010_timer.Reset();
}

void loop() {
  controller.Step();

  if (kDebugVCNL4010 && vcnl4010_timer.Expired()) {
    Serial2.printf("ambient: %5u,   proximity: %5u\n",
                   controller.ReadAmbientLight(), controller.ReadProximity());
    vcnl4010_timer.Reset();
  }
}
