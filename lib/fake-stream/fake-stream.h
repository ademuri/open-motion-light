#pragma once

#ifndef ARDUINO

#include <types.h>

#include "fake-print.h"

class Stream : public Print {
 public:
  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;

  size_t readBytes(unsigned char *buffer, size_t length);
};

#endif  // ARDUINO
