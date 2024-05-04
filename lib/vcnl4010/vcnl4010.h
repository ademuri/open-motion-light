#pragma once

#include <types.h>

// HAL for the VCNL4010 ambient light and proximity sensor.
class VCNL4010 {
  // Initializes the sensor.
  virtual bool Begin() = 0;

  // Sets the current for the proximity sensor LED, in milliamps. The precision is 10s of milliamps.
  virtual void SetLEDCurrent(uint8_t mA) = 0;

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  virtual uint16_t ReadProximity() = 0;

  // Reads the 16-bit ambient light value.
  virtual uint16_t ReadAmbient() = 0;
};
