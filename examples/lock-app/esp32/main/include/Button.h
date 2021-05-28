/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppConfig.h"
#include "AppTask.h"
#include "freertos/FreeRTOS.h"

class Button
{
public:
    esp_err_t Init(gpio_num_t gpioNum, uint16_t debouncePeriod);
    bool Poll();
    bool IsPressed();
    uint32_t GetStateStartTime();
    uint32_t GetStateDuration();
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
