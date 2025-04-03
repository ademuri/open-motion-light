/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <STM32LowPower.h>
#include <types.h>

#include "power-controller.h"

class Stm32PowerController : public PowerController {
 public:
  bool Begin() override;
  void AttachInterruptWakeup(uint32_t pin, uint32_t mode) override;
  void Sleep(uint32_t millis) override;
  void Stop() override;

 private:
  STM32LowPower impl_;
};
