#pragma once

#include <iostream>
#include <gtest/gtest.h>

#include "controller.h"
#include "types.h"

std::ostream& operator<<(std::ostream& os, const Controller::PowerMode power_mode);

class LightTest : public ::testing::Test {
  protected:
  void SetUp() override {
    // Reset Arduino compatibility layer's static state
    setMillis(0);
    for (uint32_t pin = 0; pin <= kPinMax; pin++) {
      setDigitalRead(pin, false);
      analogWrite(pin, 0);
    }
  }

};
