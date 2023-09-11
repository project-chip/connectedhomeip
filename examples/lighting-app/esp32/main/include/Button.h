/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1
#define BUTTON_NUMBER 3
class Button
{
public:
    Button();
    Button(gpio_num_t gpioNum);

    esp_err_t Init();
    esp_err_t Init(gpio_num_t gpioNum);

    inline gpio_num_t GetGPIONum();
    static void TimerCallback(TimerHandle_t xTimer);

    friend void IRAM_ATTR button_isr_handler(void * arg);

private:
    gpio_num_t mGPIONum;
    TimerHandle_t mbuttonTimer; // FreeRTOS timers used for debouncing buttons
};

inline gpio_num_t Button::GetGPIONum()
{
    return mGPIONum;
}
