#pragma once

#include <Adafruit_VCNL4010.h>
#include <types.h>

#include "vcnl4010.h"

// Implementation of the VCNL4010 HAL on the hardware device.
class ArduinoVCNL4010 : public VCNL4010 {
 public:
  // Initializes the sensor.
  bool Begin() override;

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  void SetLEDCurrent(uint8_t mA) override;

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  uint16_t ReadProximity() override;

  // Reads the 16-bit ambient light value.
  uint16_t ReadAmbient() override;

 private:
  // TODO: modify this library to allow lower-power usage, or write a new one.
  // Additionally, the readProximity and readAmbient calls are blocking, which
  // isn't what we want.
  Adafruit_VCNL4010 sensor_;
};
