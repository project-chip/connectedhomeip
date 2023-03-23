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

#define MIN_LVL (1)
#define MAX_LVL (254)

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

    mLed   = led;
    mLevel = MAX_LVL;

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

bool LEDWidget::Get(void)
{
    return mState;
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

void LEDWidget::Color(RgbColor_t rgb)
{
    filogic_led_color_t _color;
    _color.r = rgb.r;
    _color.g = rgb.g;
    _color.b = rgb.b;

    switch (mLed)
    {
    case LED_LIGHT:
        filogic_led_light_color(_color);
        break;
    case LED_STATUS:
        filogic_led_status_color(_color);
        break;
    }
}

void LEDWidget::SetLevel(uint8_t level)
{
    uint8_t fdim = 0;

    if (level >= MIN_LVL && level <= MAX_LVL)
    {
        mLevel = level;
    }
    else
    {
        return;
    }

    fdim = (level - MIN_LVL) / ((MAX_LVL - MIN_LVL) / (filogic_led_get_max_dim_level() - filogic_led_get_min_dim_level()));

    if (fdim > filogic_led_get_max_dim_level())
    {
        fdim = filogic_led_get_max_dim_level();
    }

    if (mLed == LED_LIGHT)
    {
        filogic_led_light_dim(fdim);
    }
    else if (mLed == LED_STATUS)
    {
        filogic_led_status_dim(fdim);
    }
    else
    {
        assert(0);
    }
}

uint8_t LEDWidget::GetLevel(void)
{
    uint8_t dim = 0;
    if (mLed == LED_LIGHT)
    {
        dim = filogic_led_light_get_cur_dim_level();
    }
    else if (mLed == LED_STATUS)
    {
        dim = filogic_led_status_get_cur_dim_level();
    }
    else
    {
        assert(0);
    }

    dim = dim * ((MAX_LVL - MIN_LVL) / (filogic_led_get_max_dim_level() - filogic_led_get_min_dim_level()));

    return dim;
}

uint8_t LEDWidget::GetMaxLevel(void)
{
    return MAX_LVL;
}

uint8_t LEDWidget::GetMinLevel(void)
{
    return MIN_LVL;
}
