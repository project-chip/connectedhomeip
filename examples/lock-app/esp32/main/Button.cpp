/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Button.h"
#include "AppTask.h"

//#include <lock/AppConfig.h>

esp_err_t Button::Init(gpio_num_t gpioNum, uint16_t debouncePeriod)
{
    mGPIONum         = gpioNum;
    mDebouncePeriod  = debouncePeriod / portTICK_PERIOD_MS;
    mState           = false;
    mLastPolledState = false;

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
