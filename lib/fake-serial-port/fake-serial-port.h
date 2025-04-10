#pragma once

#include "serial-port.h"

class FakeSerialPort : public SerialPort {
 public:
  int available() override;
  int read() override;
  int peek() override;
  size_t write(uint8_t c) override;
  pb_istream_s BuildPbIstream() override;
  pb_ostream_s BuildPbOstream() override;

  void Reset();

  template <typename T>
  void WritePb(const pb_msgdesc_t& msg, const T& t);

  template <typename T>
  void ReadPb(const pb_msgdesc_t& msg, T* t);

 private:
  static constexpr size_t kBufferMaxSize = 1024;

  std::array<uint8_t, kBufferMaxSize> input_buffer_;
  std::string output_buffer_;
  size_t read_idx_ = 0;
  size_t input_buffer_bytes_written_ = 0;
};

template <typename T>
void FakeSerialPort::WritePb(const pb_msgdesc_s& msg, const T& t) {
  input_buffer_.fill(0);

  read_idx_ = 0;
  pb_ostream_s ostream = pb_ostream_from_buffer(
      reinterpret_cast<uint8_t*>(input_buffer_.data()), kBufferMaxSize);
  ASSERT_TRUE(pb_encode_ex(&ostream, &msg, &t, PB_ENCODE_DELIMITED));
  input_buffer_bytes_written_ = ostream.bytes_written;
}

template <typename T>
void FakeSerialPort::ReadPb(const pb_msgdesc_s& msg, T* t) {
  pb_istream_s istream = pb_istream_from_buffer(
      reinterpret_cast<const uint8_t*>(output_buffer_.data()),
      output_buffer_.size());
  ASSERT_TRUE(pb_decode_ex(&istream, &msg, t, PB_DECODE_DELIMITED));
}
