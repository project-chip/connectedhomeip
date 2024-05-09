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

#include <cycfg_pins.h>
#include <stdio.h>

LEDWidget LEDWidget::sLEDWidget;
bool usr_LED1_OnOffStatus = false;

void LEDWidget::Init(wiced_led_config_t * config, size_t count)
{
    const wiced_result_t result = wiced_led_manager_init(config, count);
    if (result != WICED_SUCCESS)
        printf("wiced_led_manager_init failed %d\n", result);
}

void LEDWidget::Set(bool state, wiced_led_t pin)
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

void LEDWidget::DoSetLEDOnOff(bool state, wiced_led_t pin)
{
    if (pin == PLATFORM_LED_RED)
    {
        usr_LED1_OnOffStatus = state;
    }

    if (state)
    {
        wiced_led_manager_enable_led(pin);
    }
    else
    {
        wiced_led_manager_disable_led(pin);
    }
}
