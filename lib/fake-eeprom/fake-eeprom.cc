#include "fake-eeprom.h"

#include <gtest/gtest.h>

uint8_t EEPROMClass::read(int idx) const {
  EXPECT_GE(idx, 0);
  EXPECT_LT(idx, kSize);
  return data_[idx];
}

void EEPROMClass::write(int idx, uint8_t val) {
  EXPECT_GE(idx, 0);
  EXPECT_LT(idx, kSize);
  data_[idx] = val;
}

void EEPROMClass::reset() { data_.fill(0); }

std::ostream& operator<<(std::ostream& os, const EEPROMClass eeprom) {
  uint32_t magicByte0;
  uint32_t magicByte1;
  uint32_t configVersion;
  // uint16_t instead of uint8_t so it's not interpreted as an ASCII code
  uint16_t firstByte = eeprom.read(3 * sizeof(magicByte0));

  eeprom.get(0, magicByte0);
  eeprom.get(sizeof(magicByte0), magicByte1);
  eeprom.get(2 * sizeof(magicByte0), configVersion);

  os << "EEPROM{ 0x" << std::hex << magicByte0 << ", 0x" << magicByte1 << ", 0x"
     << configVersion;

  for (size_t i = 0; i < 25; ++i) {
    uint16_t val = eeprom.read(3 * sizeof(magicByte0) + i);
    os << ", " << std::dec << val;
  }

  os << " }";
  return os;
}

EEPROMClass EEPROM;
