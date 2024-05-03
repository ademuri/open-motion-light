#include "fake-pins.h"

#ifndef ARDUINO

const uint16_t *const VREFINT_CAL_ADDR = &kFakeVrefintCal;

#endif // ifndef ARDUINO