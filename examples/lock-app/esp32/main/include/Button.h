/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTask.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include <stdint.h>

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
