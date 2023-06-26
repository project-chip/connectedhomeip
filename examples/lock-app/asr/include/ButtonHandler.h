/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer suppo
#ifdef CFG_PLF_RV32
#include "asr_gpio.h"
#define duet_gpio_dev_t asr_gpio_dev_t
#define DUET_INPUT_PULL_UP ASR_INPUT_PULL_UP
#define duet_gpio_init asr_gpio_init
#define duet_gpio_output_low asr_gpio_output_low
#define duet_gpio_output_high asr_gpio_output_high
#define duet_gpio_input_get asr_gpio_input_get
#define duet_gpio_enable_irq asr_gpio_enable_irq
#define DUET_IRQ_TRIGGER_FALLING_EDGE ASR_IRQ_TRIGGER_FALLING_EDGE
#else
#include "duet_gpio.h"
#endif

#define GPIO_INTERRUPT_PRIORITY (5)

class ButtonHandler
{
public:
    static void Init(void);

private:
    static void GpioInit(void);
    static void lockbuttonIsr(void * handler_arg);
    static void functionbuttonIsr(void * handler_arg);
    static void TimerCallback(TimerHandle_t xTimer);
};
