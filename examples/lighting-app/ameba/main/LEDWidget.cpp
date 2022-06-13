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

gpio_t gpio_led;

void LEDWidget::Init(PinName gpioNum)
{

    mGPIONum = gpioNum;
    mState   = false;

    if (gpioNum != (PinName) NC)
    {
        // Init LED control pin
        gpio_init(&gpio_led, gpioNum);
        gpio_dir(&gpio_led, PIN_OUTPUT); // Direction: Output
        gpio_mode(&gpio_led, PullNone);  // No pull
        gpio_write(&gpio_led, mState);
    }
}

void LEDWidget::Set(bool state)
{
    DoSet(state);
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;

    if (stateChange)
    {
        gpio_write(&gpio_led, state);
    }
}
