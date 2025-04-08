#include "config-storage.h"

#include <pb_decode.h>
#include <pb_encode.h>
#include <types.h>

#include "serial.pb.h"

constexpr size_t kEepromOffset = sizeof(uint32_t) * 3;
constexpr size_t kEepromDataMaxSize = 2 * 1024 - kEepromOffset;

bool nanopbOutputCallback(pb_ostream_t *stream, const uint8_t *buf,
                          size_t count) {
  if (buf == nullptr) {
    return false;
  }

  if (count > stream->max_size) {
    return false;
  }

  size_t *bytes_written = static_cast<size_t *>(stream->state);

  for (size_t i = 0; i < count; i++) {
    EEPROM.write(kEepromOffset + i + *bytes_written, buf[i]);
  }
  *bytes_written += count;

  return true;
}

bool nanopbInputCallback(pb_istream_t *stream, uint8_t *buf, size_t count) {
  if (buf == nullptr) {
    return false;
  }

  if (count > stream->bytes_left) {
    return false;
  }

  size_t *bytes_read = static_cast<size_t *>(stream->state);

  for (size_t i = 0; i < count; i++) {
    buf[i] = EEPROM.read(kEepromOffset + i + *bytes_read);
  }

  *bytes_read += count;

  return true;
}

bool ConfigStorage::TryLoadConfig(ConfigPb *const config) {
  uint32_t magicByte0;
  uint32_t magicByte1;
  EEPROM.get(0, magicByte0);
  EEPROM.get(sizeof(magicByte0), magicByte1);

  if (!(magicByte0 == kEEPROMMagicByte0 && magicByte1 == kEEPROMMagicByte1)) {
    return false;
  }

  uint32_t configVersion;
  EEPROM.get(2 * sizeof(magicByte0), configVersion);
  if (configVersion != kConfigVersion) {
    return false;
  }

  size_t bytes_read = 0;
  pb_istream_t eepromStream = {&nanopbInputCallback,
                               static_cast<void *>(&bytes_read),
                               kEepromDataMaxSize};
  return pb_decode_ex(&eepromStream, &ConfigPb_msg, config,
                      PB_DECODE_DELIMITED);
}

bool ConfigStorage::SaveConfig(ConfigPb const *const config) {
  size_t bytes_written = 0;
  pb_ostream_t eepromStream = {&nanopbOutputCallback,
                               static_cast<void *>(&bytes_written),
                               kEepromDataMaxSize, /*bytes_written=*/0};
  bool result =
      pb_encode_ex(&eepromStream, &ConfigPb_msg, config, PB_ENCODE_DELIMITED);

  if (!result) {
    return false;
  }

  EEPROM.put(2 * sizeof(kEEPROMMagicByte0), kConfigVersion);
  EEPROM.put(0, kEEPROMMagicByte0);
  EEPROM.put(sizeof(kEEPROMMagicByte0), kEEPROMMagicByte1);
  return true;
}