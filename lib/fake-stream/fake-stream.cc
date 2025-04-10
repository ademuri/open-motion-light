#include "fake-stream.h"

size_t Stream::readBytes(unsigned char *buffer, size_t length) {
  size_t count = 0;
  while (count < length) {
    int c = read();
    if (c < 0) {
      break;
    }
    *buffer++ = (char)c;
    count++;
  }
  return count;
}
