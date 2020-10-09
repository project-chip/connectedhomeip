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
     *
     * @param gpioNum           The GPIO pin this button should keep track of
     * @param debouncePeriod    The debouncing period in FreeRTOS ticks
     * @return esp_err_t
     */
    esp_err_t Init(gpio_num_t gpioNum, uint16_t debouncePeriod);
    /**
     * @brief
     *  Poll on the button and read its current state
     *
     * @return true     If a button event occurred
     * @return false    If no button event occurred
     */
    bool Poll();
    /**
     * @brief
     *  Returns the state of the button
     *
     * @return true     If the button is pressed
     * @return false    If the button is not pressed or released if poll() is true.
     */
    bool IsPressed();
    /**
     * @brief
     *  Get the time timestamp since the button entered its current state
     *
     * @return uint32_t The time in milliseconds since the app started
     */
    uint32_t GetStateStartTime();
    /**
     * @brief
     *  Get the duration in milliseconds since the button entered its current state
     *
     * @return uint32_t The time in milliseconds
     */
    uint32_t GetStateDuration();
    /**
     * @brief
     *  Get the duration in milliseconds the button spent in its previous state
     *
     * @return uint32_t The time in milliseconds
     */
    uint32_t GetPrevStateDuration();

private:
    // in ticks
    uint32_t mLastReadTime;
    // in ticks
    uint32_t mStateStartTime;
    // in ticks
    uint32_t mPrevStateDur;
    gpio_num_t mGPIONum;
    // in ticks
    uint16_t mDebouncePeriod;
    // true when button is pressed
    bool mState;
    bool mLastPolledState;
};

inline bool Button::IsPressed()
{
    return mState;
}

inline uint32_t Button::GetStateStartTime()
{
    return mStateStartTime * portTICK_PERIOD_MS;
}

inline uint32_t Button::GetPrevStateDuration()
{
    return mPrevStateDur * portTICK_PERIOD_MS;
}
