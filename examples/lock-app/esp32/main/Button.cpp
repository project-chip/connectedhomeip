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

#include "Button.h"
#include "AppTask.h"

// #include <lock/AppConfig.h>

esp_err_t Button::Init(gpio_num_t gpioNum, uint16_t debouncePeriod)
{
    mGPIONum         = gpioNum;
    mDebouncePeriod  = debouncePeriod / portTICK_PERIOD_MS;
    mState           = false;
    mLastPolledState = false;

    gpio_config_t io_conf = {};
    io_conf.intr_type     = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask  = 1ULL << gpioNum;
    io_conf.mode          = GPIO_MODE_INPUT;
    io_conf.pull_down_en  = GPIO_PULLDOWN_ENABLE;

    gpio_config(&io_conf);

    return gpio_set_direction(gpioNum, GPIO_MODE_INPUT);
}

bool Button::Poll()
{
    uint32_t now = xTaskGetTickCount();

    bool newState = gpio_get_level(mGPIONum);

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
