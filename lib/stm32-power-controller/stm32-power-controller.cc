#include "stm32-power-controller.h"

#include <STM32LowPower.h>
#include <types.h>

#include "pins.h"

static uint32_t rtc_seconds_at_sleep = 0;
static uint32_t rtc_subseconds_at_sleep = 0;

extern __IO uint32_t uwTick;

namespace {

// Called when the processor wakes up from sleep.
void WakeUpCallback() {}

};  // namespace

bool Stm32PowerController::Begin() {
  impl_.begin();
  return true;
};

void Stm32PowerController::AttachInterruptWakeup(uint32_t pin, uint32_t mode) {
  impl_.attachInterruptWakeup(pin, &WakeUpCallback, mode,
                              LP_Mode::DEEP_SLEEP_MODE);
}

void Stm32PowerController::Sleep(uint32_t ms) {
  // When in stop mode, the SysTick interrupt doesn't fire to update Arduino's
  // `millis()` value. So, keep track of the elapsed time using the RTC, and
  // update the value manually on wakeup.
  STM32RTC& rtc = STM32RTC::getInstance();
  rtc_seconds_at_sleep = rtc.getEpoch(&rtc_subseconds_at_sleep);
  pinMode(kPinScl, INPUT);
  pinMode(kPinSda, INPUT);

  // Puts the processor into STM32 Stop mode
  // TODO: reduce power consumption. Currently deepSleep() is ~376uA, and
  // shutdown() is ~48uA.
  // TODO: Make sure that the light sensor is actually in standby.
  impl_.deepSleep(ms);
  // impl_.shutdown();

  uint32_t seconds;
  uint32_t subseconds;
  seconds = rtc.getEpoch(&subseconds);

  // This reaches into STM32's Arduino implementation and modifies the value
  // underlying millis() directly. This is not great, but it works.
  uwTick += (seconds - rtc_seconds_at_sleep) * 1000 +
            (subseconds - rtc_subseconds_at_sleep);
  // TODO: trigger I2C re-init?
}
