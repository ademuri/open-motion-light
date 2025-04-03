#pragma once

#include <types.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "serial-port.h"

class ArduinoSerialPort : public SerialPort {
 public:
  int available() override;
  int read() override;
  int peek() override;

  size_t write(uint8_t c) override;

  pb_istream_s BuildPbIstream() override;
  pb_ostream_s BuildPbOstream() override;
};
