/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <assert.h>
#include <stdio.h>

#include <filogic_led.h>

#include "LEDWidget.h"

void LEDWidget::TimerHandler(TimerHandle_t xTimer)
{
    LEDWidget * led_widget = (LEDWidget *) pvTimerGetTimerID(xTimer);
    led_widget->Toggle();
}

void LEDWidget::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(mTimer))
        CancelTimer();

    assert(xTimerChangePeriod(mTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) == pdPASS);
}

void LEDWidget::CancelTimer(void)
{
    assert(xTimerStop(mTimer, 0) == pdPASS);
}

void LEDWidget::Init(enum led_id led)
{
    filogic_led_init();

    mLed = led;

    mTimer = xTimerCreate(Name(),
                          1,             // == default timer period (mS)
                          false,         // no timer reload (==one-shot)
                          (void *) this, // init timer id = app task obj context
                          TimerHandler); // timer callback handler
}

const char * LEDWidget::Name(void)
{
    const char * name;

    switch (mLed)
    {
    case LED_LIGHT:
        name = "LED_LIGHT";
        break;
    case LED_STATUS:
        name = "LED_STATUS";
        break;
    default:
        name = "LED UNKNOWN";
        break;
    }

    return name;
}

void LEDWidget::Toggle(void)
{
    Set(!mState);
    StartTimer(mState ? mOn : mOff);
}

void LEDWidget::DoBlink(void)
{
    StartTimer(mOn);
}

void LEDWidget::DoSet(bool state)
{
    switch (mLed)
    {
    case LED_LIGHT:
        filogic_led_light_toggle(state);
        break;
    case LED_STATUS:
        filogic_led_status_toggle(state);
        break;
    }
    mState = state;
}

void LEDWidget::Set(bool state)
{
    CancelTimer();
    DoSet(state);
    // printf("%s %s\n", Name(), state ? "on" : "off");
}

void LEDWidget::Blink(int on, int off)
{
    if (mOn != on || mOff != off)
    {
        mOn  = on;
        mOff = off;
        // printf("%s blink: on %d off %d\n", Name(), mOn, mOff);
        DoBlink();
    }
}

void LEDWidget::Blink(int duration)
{
    Blink(duration, duration);
}

void LEDWidget::Color(enum led_color color)
{
    filogic_led_color_t _color = FILOGIC_LED_OFF;

    switch (color)
    {
    case LED_RED:
        _color = FILOGIC_LED_R;
    case LED_GREEN:
        _color = FILOGIC_LED_G;
    case LED_BLUE:
        _color = FILOGIC_LED_B;
    }

    switch (mLed)
    {
    case LED_LIGHT:
        filogic_led_light_color(_color);
        break;
    case LED_STATUS:
        filogic_led_status_color(_color);
        break;
    }

    // printf("%s color %d\n", Name(), color);
}
