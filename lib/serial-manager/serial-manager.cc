#include "serial-manager.h"

#include <iterator>

#include "serial.pb.h"
#include "config-storage.h"

void SerialManager::Step() {
  static pb_istream_s istream = serial_port_->BuildPbIstream();
  static pb_ostream_s ostream = serial_port_->BuildPbOstream();

  static constexpr size_t kFirmwareVersionMaxLength =
      sizeof(StatusPb::firmware_version) / sizeof(char);

  if (serial_port_->available()) {
    SerialRequest request;
    // TODO: check return value
    pb_decode_ex(&istream, &SerialRequest_msg, &request, PB_ENCODE_DELIMITED);

    if (request.has_config) {
      controller_->SetConfig(request.config);
      ConfigStorage::SaveConfig(&request.config);
    }

    SerialResponse response;
    response.status.battery_voltage_millivolts =
        controller_->GetFilteredBatteryMillivolts();
    response.status.has_battery_voltage_millivolts = true;
    snprintf(response.status.firmware_version, kFirmwareVersionMaxLength,
             FIRMWARE_VERSION);
    response.status.has_firmware_version = true;

    if (request.request_config) {
      response.config = *controller_->GetConfig();
      response.has_config = true;
    }

    // TODO: check return value
    pb_encode_ex(&ostream, &SerialResponse_msg, &response, PB_DECODE_DELIMITED);
  }
}
