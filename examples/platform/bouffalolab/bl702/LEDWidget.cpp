/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <bl702_glb.h>
#include <bl_gpio.h>
#include <bl_sys.h>
#include <board.h>
#include <demo_pwm.h>
#include <hosal_gpio.h>

#include "LEDWidget.h"

void LEDWidget::Init()
{
    mPin = LED1_PIN;

    hosal_gpio_dev_t gpio_led = { .config = OUTPUT_OPEN_DRAIN_NO_PULL, .priv = NULL };
    gpio_led.port             = mPin;

    hosal_gpio_init(&gpio_led);
    SetOnoff(false);
}

void LEDWidget::Toggle(void)
{
    SetOnoff(1 - mOnoff);
}

void LEDWidget::SetOnoff(bool state)
{
    hosal_gpio_dev_t gpio_led = { .port = mPin, .config = OUTPUT_OPEN_DRAIN_NO_PULL, .priv = NULL };

    mOnoff = state;

    if (state)
    {
        hosal_gpio_output_set(&gpio_led, 1);
    }
    else
    {
        hosal_gpio_output_set(&gpio_led, 0);
    }
}

bool LEDWidget::GetOnoff(void)
{
    return mOnoff ? true : false;
}

void DimmableLEDWidget::Init()
{
    mOnoff = light_v = 0;

    demo_hosal_pwm_init();
    demo_hosal_pwm_start();
}

void DimmableLEDWidget::Toggle(void)
{
    SetOnoff(1 - mOnoff);
}

void DimmableLEDWidget::SetOnoff(bool state)
{
    mOnoff = state;
    if (mOnoff)
    {
        if (light_v)
        {
            set_level(light_v);
        }
        else
        {
            set_level(254);
        }
    }
    else
    {
        set_level(0);
    }
}

void DimmableLEDWidget::SetLevel(uint8_t level)
{
    set_level(level);
    light_v = level;
    mOnoff  = light_v > 0;
}

void ColorLEDWidget::Init()
{
    mOnoff = light_v = light_s = light_h = 0;

    demo_hosal_pwm_init();
    demo_hosal_pwm_start();
}

void ColorLEDWidget::Toggle(void)
{
    SetOnoff(1 - mOnoff);
}

void ColorLEDWidget::SetOnoff(bool state)
{
    mOnoff = state;
    if (mOnoff)
    {
        if (0 == light_v)
        {
            set_color(254, light_h, light_s);
        }
        else
        {
            set_color(light_v, light_h, light_s);
        }
    }
    else
    {
        set_color(0, light_h, light_s);
    }
}

void ColorLEDWidget::SetLevel(uint8_t level)
{
    SetColor(level, light_h, light_s);
}

void ColorLEDWidget::SetColor(uint8_t level, uint8_t hue, uint8_t sat)
{
    set_color(level, hue, sat);
    light_v = level;
    light_h = hue;
    light_s = sat;
    mOnoff  = light_v > 0;
}
