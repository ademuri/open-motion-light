#pragma once

// #define CUSTOM_CLOCK

#ifdef CUSTOM_CLOCK
extern "C" {
void SystemClock_Config();
}
#endif  // ifdef CUSTOM_CLOCK
