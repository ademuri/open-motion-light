#include "arduino-serial-port.h"

#include <Arduino.h>
#include <pb_arduino.h>

int ArduinoSerialPort::available() { return Serial1.available(); }

int ArduinoSerialPort::read() { return Serial1.read(); }

int ArduinoSerialPort::peek() { return Serial1.peek(); }

size_t ArduinoSerialPort::write(uint8_t c) { return Serial1.write(c); }

pb_istream_s ArduinoSerialPort::BuildPbIstream() {
  return as_pb_istream(Serial1);
}

pb_ostream_s ArduinoSerialPort::BuildPbOstream() {
  return as_pb_ostream(Serial1);
}
