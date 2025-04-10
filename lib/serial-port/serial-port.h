#pragma once

#include <pb_decode.h>
#include <pb_encode.h>
#include <types.h>

#include <cstddef>

#ifndef ARDUINO
#include "fake-stream.h"
#endif  // ARDUINO

// Class which wraps a serial port, i.e. `Serial` on Arduino.
class SerialPort : public Stream {
 public:
  // From Stream
  virtual int available() override = 0;
  virtual int read() override = 0;
  virtual int peek() override = 0;

  // From Print
  virtual size_t write(uint8_t c) override = 0;

  // nanopb-specific (not inherited)
  virtual pb_istream_s BuildPbIstream() = 0;
  virtual pb_ostream_s BuildPbOstream() = 0;
};
