/*
 *
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
 * @file LEDWidget.cpp
 *
 * Implements an LED Widget controller that is usually tied to a GPIO
 * It also updates the display widget if it's enabled
 */

#include "LEDWidget.h"
#include "matter_led.h"
#include <stdlib.h>

void LEDWidget::InitGpio()
{
    matter_led_init();
}

void LEDWidget::Init(uint8_t gpioNum)
{
    mLEDHandle = matter_led_create(gpioNum, false, 0);
    mState     = false;
    mMinLevel  = 0;
    mMaxLevel  = 0;
    mLevel     = 0;
}

void LEDWidget::Init(uint8_t gpioNum, uint8_t aMinLevel, uint8_t aMaxLevel, uint8_t aDefaultLevel)
{
    uint16_t brightness;

    brightness = (aDefaultLevel * 65535.0) / mMaxLevel;

    mLEDHandle = matter_led_create(gpioNum, true, brightness);
    mState     = false;
    mMinLevel  = aMinLevel;
    mMaxLevel  = aMaxLevel;
    mLevel     = aDefaultLevel;
}

void LEDWidget::Invert(void)
{
    Set(!mState);
}

void LEDWidget::Set(bool state)
{
    if (mLEDHandle)
    {
        mState = state;
        matter_led_state_set(mLEDHandle, state);
    }
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
    if (mLEDHandle)
    {
        matter_led_blink_start(mLEDHandle, changeRateMS, changeRateMS, 0xFFFF);
    }
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
    if (mLEDHandle)
    {
        matter_led_blink_start(mLEDHandle, onTimeMS, offTimeMS, 0xFFFF);
    }
}

void LEDWidget::BlinkStop()
{
    if (mLEDHandle)
    {
        matter_led_blink_stop(mLEDHandle);
    }
}

void LEDWidget::SetLevel(uint8_t aLevel)
{
    if (mLEDHandle)
    {
        uint16_t brightness;

        mLevel     = aLevel;
        brightness = (aLevel * 65535.0) / mMaxLevel;
        matter_led_brightness_set(mLEDHandle, brightness);
    }
}
