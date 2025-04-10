#include "fake-serial-port.h"

#include <gtest/gtest.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "serial.pb.h"

bool pb_print_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count) {
  Print *p = reinterpret_cast<Print *>(stream->state);
  size_t written = p->write(buf, count);
  return written == count;
};

bool pb_stream_read(pb_istream_t *stream, pb_byte_t *buf, size_t count) {
  Stream *s = reinterpret_cast<Stream *>(stream->state);
  size_t written = s->readBytes(buf, count);
  return written == count;
};

int FakeSerialPort::available() { return input_buffer_.size() - read_idx_; }

int FakeSerialPort::read() {
  if (read_idx_ >= input_buffer_bytes_written_) {
    return -1;
  }
  return input_buffer_[read_idx_++];
}

int FakeSerialPort::peek() {
  if (read_idx_ >= input_buffer_bytes_written_) {
    return -1;
  }
  return input_buffer_[read_idx_];
}

size_t FakeSerialPort::write(uint8_t c) {
  output_buffer_.push_back(c);
  return 1;
}

pb_istream_s FakeSerialPort::BuildPbIstream() {
  return {pb_stream_read, reinterpret_cast<void *>(this),
          input_buffer_bytes_written_};
}

pb_ostream_s FakeSerialPort::BuildPbOstream() {
  return {pb_print_write, reinterpret_cast<void *>(this), kBufferMaxSize,
          /*bytes_written=*/0};
}

void FakeSerialPort::Reset() {
  input_buffer_.fill(0);
  output_buffer_.clear();
  read_idx_ = 0;
  input_buffer_bytes_written_ = 0;
}
