#pragma once

#include "serial.pb.h"

class ConfigStorage {
 public:
  // Tries to load the config from storage (EEPROM). Returns whether the load
  // was successful.
  static bool TryLoadConfig(ConfigPb *config);

  // Save the config to storage (EEPROM).
  static bool SaveConfig(ConfigPb const *config);

  // These are visible for testing:

  // Magic values, to determine if the EEPROM is valid.
  static constexpr uint32_t kEEPROMMagicByte0 = 0xDEADBEEF;
  static constexpr uint32_t kEEPROMMagicByte1 = 0xBADDF00D;

  // Current config version. This increments exactly when a new config version
  // is incompatible with the older version. This is unlikely - it's a defensive
  // measure.
  static constexpr uint32_t kConfigVersion = 1;
};
