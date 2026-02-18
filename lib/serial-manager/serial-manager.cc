#include "serial-manager.h"

#include "config-storage.h"
#include "serial.pb.h"

bool SerialManager::Init() { return controller_->Init(); }

void SerialManager::Step() {
  static constexpr size_t kFirmwareVersionMaxLength =
      sizeof(StatusPb::firmware_version) / sizeof(char);

  if (serial_port_->available()) {
    pb_istream_s istream = serial_port_->BuildPbIstream();
    pb_ostream_s ostream = serial_port_->BuildPbOstream();

    SerialRequest request;
    bool success = pb_decode_ex(&istream, &SerialRequest_msg, &request,
                                PB_ENCODE_DELIMITED);

    if (success && request.has_config) {
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

    response.status.proximity_value = controller_->ReadProximity();
    response.status.has_proximity_value = true;
    response.status.ambient_light_value = controller_->ReadAmbientLight();
    response.status.has_ambient_light_value = true;
    response.status.temperature_celsius = controller_->ReadTemperature();
    response.status.has_temperature_celsius = true;

    response.has_status = true;

    if (success && request.request_config) {
      response.config = *controller_->GetConfig();
      response.has_config = true;
    }

    // TODO: check return value
    pb_encode_ex(&ostream, &SerialResponse_msg, &response, PB_DECODE_DELIMITED);
  }
}
