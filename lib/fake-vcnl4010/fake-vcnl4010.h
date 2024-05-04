#pragma once

#include <gtest/gtest.h>
#include <types.h>

#include "vcnl4010.h"

// Fake implementation of the VCNL4010 for testing.
class FakeVCNL4010 : public VCNL4010 {
 public:
  // Initializes the sensor.
  bool Begin() override { return true; }

  // Sets the current for the proximity sensor LED, in milliamps. The precision
  // is 10s of milliamps.
  void SetLEDCurrent(uint8_t mA) override {
    ASSERT_LE(mA, 200);
    ASSERT_EQ(mA % 10, 0) << "LED current has 10s precision";
    led_current_ma_ = mA;
  }

  uint8_t GetLEDCurrent() { return led_current_ma_; }

  // Reads the 16-bit proximity sensor value. This depends on the LED current.
  uint16_t ReadProximity() override { return proximity_; }

  // Sets the value of the proximity for testing.
  void SetProximity(uint16_t val) { proximity_ = val; }

  // Reads the 16-bit ambient light value.
  uint16_t ReadAmbient() override { return ambient_; }

  // Sets the value of the ambient light for testing.
  void SetAmbient(uint16_t val) { ambient_ = val; }

 private:
  uint8_t led_current_ma_;
  uint16_t proximity_;
  uint16_t ambient_;
};
