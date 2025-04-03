#pragma once

#include <types.h>

#include "serial-port.h"

class ArduinoSerialPort : public SerialPort {
 public:
  int available() override;
  int read() override;
  int peek() override;

  size_t write(uint8_t c) override;
};
