#pragma once

#include <cstddef>
#include <types.h>

// Class which wraps a serial port, i.e. `Serial` on Arduino.
class SerialPort
#ifdef ARDUINO
    : public Stream
#endif  // ARDUINO
{
 public:
  // From Stream
  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;

  // From Print
  virtual size_t write(uint8_t c) = 0;

};
