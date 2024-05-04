#pragma once

#include <gtest/gtest.h>

#include <iostream>

#include "controller.h"
#include "types.h"

std::ostream& operator<<(std::ostream& os, const PowerMode power_mode);

std::ostream& operator<<(std::ostream& os, const PowerStatus power_status);

std::ostream& operator<<(std::ostream& os, const USBStatus usb_status);

class LightTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset Arduino compatibility layer's static state
    setMillis(0);
    for (uint32_t pin = 0; pin < kPinMax; pin++) {
      setDigitalRead(pin, false);
      analogWrite(pin, 0);
    }
  }
};
