/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
