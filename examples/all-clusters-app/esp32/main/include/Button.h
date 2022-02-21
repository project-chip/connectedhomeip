/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 * @file Button.h
 *
 * Describes a Button tied to a GPIO that provides debouncing and polling APIs
 *
 **/

#pragma once

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Button
{
public:
    /**
     * @brief
     *  Initialize a button
     * @param gpioNum           The GPIO pin this button should keep track of
     * @return esp_err_t
     */
    esp_err_t Init(gpio_num_t gpioNum);
    /**
     * @brief
     *  Get the pin number assosiate to a button
     * @return gpio_num_t
     */
    inline gpio_num_t GetGPIONum();

private:
    gpio_num_t mGPIONum;
};

inline gpio_num_t Button::GetGPIONum()
{
    return mGPIONum;
}