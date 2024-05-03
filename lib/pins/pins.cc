#include "pins.h"

const uint16_t *const kAdcReferencePointer =
    static_cast<const uint16_t*>(VREFINT_CAL_ADDR);
