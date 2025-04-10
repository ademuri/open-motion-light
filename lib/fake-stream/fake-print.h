#pragma once

#ifndef ARDUINO

#include <types.h>

class Print {
 public:
  virtual size_t write(uint8_t c) = 0;

  size_t write(const uint8_t *buffer, size_t size);
};

#endif  // ARDUINO
