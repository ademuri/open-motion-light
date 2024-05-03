#include "test-lib.h"

using PowerMode = Controller::PowerMode;

std::ostream& operator<<(std::ostream& os, const PowerMode power_mode) {
  switch(power_mode) {
    case PowerMode::kOff:
      os << "kOff";
      break;

    case PowerMode::kAuto:
      os << "kAuto";
      break;

    case PowerMode::kOn:
      os << "kOn";
      break;
  }
  return os;
}
