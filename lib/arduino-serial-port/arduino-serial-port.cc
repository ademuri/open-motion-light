#include "arduino-serial-port.h"

#include <Arduino.h>

int ArduinoSerialPort::available() {
  return Serial1.available();
}

int ArduinoSerialPort::read() {
  return Serial1.read();
}

int ArduinoSerialPort::peek() {
  return Serial1.peek();
}

size_t ArduinoSerialPort::write(uint8_t c) {
  return Serial1.write(c);
}