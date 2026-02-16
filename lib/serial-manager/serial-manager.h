#pragma once

#include <types.h>

#include "controller.h"
#include "serial-port.h"
#include "serial.pb.h"

class SerialManager {
 public:
  SerialManager(SerialPort *serial_port, Controller *controller)
      : serial_port_(serial_port), controller_(controller) {}

  // Initializes this instance.
  void Init();

  // Runs one iteration.
  void Step();

 private:
  SerialPort *const serial_port_;
  Controller *const controller_;
};
