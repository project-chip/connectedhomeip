/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "cyhal.h"
#include "timers.h" // provides FreeRTOS timer support

#define GPIO_INTERRUPT_PRIORITY (5)

class ButtonHandler
{
public:
    static void Init(void);

private:
    static void GpioInit(void);
    static void light_button_callback(void * handler_arg, cyhal_gpio_event_t event);
    static void func_button_callback(void * handler_arg, cyhal_gpio_event_t event);
    static void TimerCallback(TimerHandle_t xTimer);
};
