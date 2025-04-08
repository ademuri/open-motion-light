#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>

class EEPROMClass {
 public:
  uint8_t read(int idx) const;
  void write(int idx, uint8_t val);
  
  template<typename T> T &get(int idx, T &t) const;
  template<typename T> const T &put(int idx, const T &t);
  
  // Specific to tests, not part of the Arduino API
  void reset();

 private:
  static constexpr size_t kSize = 2 * 1024;
  std::array<uint8_t, kSize> data_;
};

template <typename T>
T& EEPROMClass::get(int idx, T& t) const{
  EXPECT_GE(idx, 0);
  EXPECT_LT(idx, kSize - sizeof(T));

  uint8_t* ptr = reinterpret_cast<uint8_t*>(&t);
  for (size_t i = 0; i < sizeof(T); ++i) {
    ptr[i] = data_[idx + i];
  }
  return t;
}

template <typename T>
const T& EEPROMClass::put(int idx, const T& t) {
  EXPECT_GE(idx, 0);
  EXPECT_LT(idx, kSize - sizeof(T));

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&t);
  for (size_t i = 0; i < sizeof(T); ++i) {
    data_[idx + i] = ptr[i];
  }
  return t;
}

extern EEPROMClass EEPROM;

std::ostream& operator<<(std::ostream& os, const EEPROMClass eeprom);
