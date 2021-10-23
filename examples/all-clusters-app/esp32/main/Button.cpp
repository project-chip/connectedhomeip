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
 * @file Button.cpp
 *
 * Implements a Button tied to a GPIO and provides debouncing and polling
 *
 **/

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "Button.h"
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

extern const char * TAG;

esp_err_t Button::Init(gpio_num_t gpioNum, uint16_t debouncePeriod)
{
    mGPIONum         = gpioNum;
    mDebouncePeriod  = debouncePeriod / portTICK_PERIOD_MS;
    mState           = false;
    mLastPolledState = false;

    esp_err_t err = gpio_set_direction(gpioNum, GPIO_MODE_INPUT);
    if (err == ESP_OK)
    {
        Poll();
    }
    return err;
}

bool Button::Poll()
{
    uint32_t now = xTaskGetTickCount();

    bool newState = gpio_get_level(mGPIONum) == 0;

    if (newState != mLastPolledState)
    {
        mLastPolledState = newState;
        mLastReadTime    = now;
    }

    else if (newState != mState && (now - mLastReadTime) >= mDebouncePeriod)
    {
        mState          = newState;
        mPrevStateDur   = now - mStateStartTime;
        mStateStartTime = now;
        return true;
    }

    return false;
}

uint32_t Button::GetStateDuration()
{
    return (xTaskGetTickCount() - mStateStartTime) * portTICK_PERIOD_MS;
}
