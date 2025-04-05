#include "serial-manager.h"

#include <iterator>

#include "serial.pb.h"

void SerialManager::Step() {
  static pb_istream_s istream = serial_port_->BuildPbIstream();
  static pb_ostream_s ostream = serial_port_->BuildPbOstream();

  static constexpr size_t kFirmwareVersionMaxLength =
      sizeof(StatusPb::firmware_version) / sizeof(char);

  if (serial_port_->available()) {
    // pb_decode_ex(&istream, &SerialRequest_msg, &request,
    // PB_ENCODE_DELIMITED);
    while (serial_port_->available()) {
      serial_port_->read();
    }

    SerialResponse response;
    // TODO: include config when the option in the request is set
    response.status.battery_voltage_millivolts =
        controller_->GetFilteredBatteryMillivolts();
    response.status.has_battery_voltage_millivolts = true;
    snprintf(response.status.firmware_version, 20, FIRMWARE_VERSION);
    response.status.has_firmware_version = true;
    pb_encode_ex(&ostream, &SerialResponse_msg, &response, PB_DECODE_DELIMITED);
  }
}
