#include "arduino-serial-port.h"

#include <Arduino.h>
#include <pb_decode.h>
#include <pb_encode.h>

namespace {

bool PbPrintWrite(pb_ostream_t* stream, const pb_byte_t* buf, size_t count) {
  Print* p = reinterpret_cast<Print*>(stream->state);
  size_t written = p->write(buf, count);
  return written == count;
}

bool PbStreamRead(pb_istream_t* stream, pb_byte_t* buf, size_t count) {
  Stream* s = reinterpret_cast<Stream*>(stream->state);
  size_t written = s->readBytes(buf, count);
  return written == count;
}

}  // namespace

int ArduinoSerialPort::available() { return Serial1.available(); }

int ArduinoSerialPort::read() { return Serial1.read(); }

int ArduinoSerialPort::peek() { return Serial1.peek(); }

size_t ArduinoSerialPort::write(uint8_t c) { return Serial1.write(c); }

pb_istream_s ArduinoSerialPort::BuildPbIstream() {
  return {PbStreamRead, &Serial1, SIZE_MAX};
}

pb_ostream_s ArduinoSerialPort::BuildPbOstream() {
  return {PbPrintWrite, &Serial1, SIZE_MAX, /*bytes_written=*/0};
}
