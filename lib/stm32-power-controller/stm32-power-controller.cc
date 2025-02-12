#include "stm32-power-controller.h"

#include <STM32LowPower.h>
#include <Wire.h>
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
  STM32RTC::getInstance().begin();
  impl_.begin();
  return true;
};

void Stm32PowerController::AttachInterruptWakeup(uint32_t pin, uint32_t mode) {
  impl_.attachInterruptWakeup(pin, &WakeUpCallback, mode,
                              LP_Mode::SHUTDOWN_MODE);
}

void Stm32PowerController::Sleep(uint32_t ms) {
  Wire.end();
  pinMode(kPinScl, INPUT_ANALOG);
  pinMode(kPinSda, INPUT_ANALOG);

  pinMode(kPinBatteryLed1, INPUT_ANALOG);
  pinMode(kPinBatteryLed2, INPUT_ANALOG);
  pinMode(kPinBatteryLed3, INPUT_ANALOG);
  pinMode(kPinWhiteLed, INPUT_ANALOG);
  pinMode(kPinChargeHighCurrentEnable, INPUT_ANALOG);

  Serial1.end();

  // When in stop mode, the SysTick interrupt doesn't fire to update Arduino's
  // `millis()` value. So, keep track of the elapsed time using the RTC, and
  // update the value manually on wakeup.
  STM32RTC& rtc = STM32RTC::getInstance();
  rtc_seconds_at_sleep = rtc.getEpoch(&rtc_subseconds_at_sleep);

  // Puts the processor into STM32 Stop mode. Power consumption is ~17uA (as of
  // 2024-10-26, with hardware v1.1)
  impl_.deepSleep(ms);

  uint32_t seconds;
  uint32_t subseconds;
  seconds = rtc.getEpoch(&subseconds);

  // This reaches into STM32's Arduino implementation and modifies the value
  // underlying millis() directly. This is not great, but it works.
  uwTick += (seconds - rtc_seconds_at_sleep) * 1000 +
            (subseconds - rtc_subseconds_at_sleep);

  pinMode(kPinBatteryLed1, OUTPUT);
  pinMode(kPinBatteryLed2, OUTPUT);
  pinMode(kPinBatteryLed3, OUTPUT);
  pinMode(kPinWhiteLed, OUTPUT);
  pinMode(kPinChargeHighCurrentEnable, OUTPUT);

  Wire.setSCL(kPinScl);
  Wire.setSDA(kPinSda);
  Wire.begin();
  Wire.setClock(400 * 1000);

  Serial1.begin(115200);
  Serial1.println("Wakeup");
}

void Stm32PowerController::Stop() {
  Wire.end();
  pinMode(kPinScl, INPUT_ANALOG);
  pinMode(kPinSda, INPUT_ANALOG);

  pinMode(kPinBatteryLed1, INPUT_ANALOG);
  pinMode(kPinBatteryLed2, INPUT_ANALOG);
  pinMode(kPinBatteryLed3, INPUT_ANALOG);
  pinMode(kPinWhiteLed, INPUT_ANALOG);
  pinMode(kPinChargeHighCurrentEnable, INPUT_ANALOG);

  Serial1.end();

  // TODO: measure power consumption
  impl_.shutdown();
}
