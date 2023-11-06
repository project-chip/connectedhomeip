/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppEvent.h"
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <stdint.h>
#ifdef CFG_PLF_RV32
#include "asr_gpio.h"
#elif defined CFG_PLF_DUET
#include "duet_gpio.h"
#else
#include "lega_gpio.h"
#endif

#define SWITCH1_BUTTON GPIO12_INDEX
#define SWITCH2_BUTTON GPIO13_INDEX

#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

#define GPIO_TASK_NAME "gpio"
#define GPIO_TASK_STACK_SIZE 1024

class ButtonHandler
{
public:
    static void Init(void);

private:
    static void GpioInit(void);
};
