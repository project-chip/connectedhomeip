/*
 *
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
