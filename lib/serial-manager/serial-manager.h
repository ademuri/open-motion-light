#pragma once

#include <types.h>

#include "serial-port.h"

class SerialManager {
 public:
  SerialManager(SerialPort *serial_port) : serial_port_(serial_port) {}

  // Runs one iteration.
  void Step();

 private:
  SerialPort *const serial_port_;
};
