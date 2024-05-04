#include "arduino-vcnl4010.h"

bool ArduinoVCNL4010::Begin() {
  // The light sensor isn't on the default I2C pins, but
  // `open-motion-light.cc::setup()` configures the default `Wire` to use the
  // correct pins.
  return sensor_.begin();
}

void ArduinoVCNL4010::SetLEDCurrent(uint8_t mA) {
  if (mA > 200) {
    mA = 200;
  }
  sensor_.setLEDcurrent(mA / 10);
}

uint16_t ArduinoVCNL4010::ReadProximity() { return sensor_.readProximity(); }

uint16_t ArduinoVCNL4010::ReadAmbient() { return sensor_.readAmbient(); }
