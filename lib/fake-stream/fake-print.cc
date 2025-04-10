#include "fake-print.h"

size_t Print::write(const uint8_t *buffer, size_t size) {
  size_t n = 0;
  while (size--) {
    if (write(*buffer++)) {
      n++;
    } else {
      break;
    }
  }
  return n;
}
