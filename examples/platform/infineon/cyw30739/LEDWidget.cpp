/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "LEDWidget.h"
#include <stdio.h>

bool usr_LED1_OnOffStatus = false;

void LEDWidget::Init(wiced_led_config_t * lighting_led_config, uint8_t nums)
{
    wiced_result_t result;

    for (uint8_t i = 0; i < nums; i++)
    {
        result = wiced_led_manager_init(&lighting_led_config[i]);
        if (result != WICED_SUCCESS)
            printf("wiced_led_manager_init fail i=%d, (%d)", i, result);
    }
}

void LEDWidget::Set(bool state, uint8_t pin)
{
    DoSetLEDOnOff(state, pin);
}

void LEDWidget::Blink(wiced_led_t led_pin, uint32_t on_period_ms, uint32_t off_period_ms)
{
    wiced_led_manager_blink_led(led_pin, on_period_ms, off_period_ms);
}

bool LEDWidget::IsLEDOn(void)
{
    return usr_LED1_OnOffStatus;
}

void LEDWidget::DoSetLEDOnOff(bool state, uint8_t pin)
{
    if (pin == PLATFORM_LED_1)
    {
        usr_LED1_OnOffStatus = state;
        if (state)
        {
            wiced_led_manager_enable_led(PLATFORM_LED_1);
        }
        else
        {
            wiced_led_manager_disable_led(PLATFORM_LED_1);
        }
    }
    else if (pin == PLATFORM_LED_2)
    {
        if (state)
        {
            wiced_led_manager_enable_led(PLATFORM_LED_2);
        }
        else
        {
            wiced_led_manager_disable_led(PLATFORM_LED_2);
        }
    }
    else
    {
        printf("Invalid pin number\n");
    }
}
