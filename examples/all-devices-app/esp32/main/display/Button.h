/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define BUTTON_1_GPIO_NUM ((gpio_num_t) 39)
#define BUTTON_2_GPIO_NUM ((gpio_num_t) 38)
#define BUTTON_3_GPIO_NUM ((gpio_num_t) 37)
#define BUTTON_NUMBER 3

class Button
{
public:
    Button() = default;
    explicit Button(gpio_num_t gpioNum) : mGPIONum(gpioNum) {}

    esp_err_t Init();
    esp_err_t Init(gpio_num_t gpioNum);

    gpio_num_t GetGPIONum() const { return mGPIONum; }

    static void TimerCallback(TimerHandle_t xTimer);

    friend void IRAM_ATTR button_isr_handler(void * arg);

private:
    gpio_num_t mGPIONum        = GPIO_NUM_NC;
    TimerHandle_t mButtonTimer = nullptr;
};
