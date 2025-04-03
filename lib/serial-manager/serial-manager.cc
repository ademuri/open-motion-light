#include "serial-manager.h"

#include <iterator>

#include "serial.pb.h"

void SerialManager::Step() {
  static pb_istream_s istream = serial_port_->BuildPbIstream();
  static pb_ostream_s ostream = serial_port_->BuildPbOstream();

  static SerialRequest request;
  static SerialResponse response;

  static constexpr size_t kFirmwareVersionMaxLength =
      std::size(response.status.firmware_version);

  if (serial_port_->available()) {
    pb_decode_ex(&istream, &SerialRequest_msg, &request, PB_ENCODE_DELIMITED);

    // TODO: include config when the option in the request is set
    response.status.battery_voltage = 1.0;
    pb_encode_ex(&ostream, &SerialResponse_msg, &response, PB_DECODE_DELIMITED);
  }
}
