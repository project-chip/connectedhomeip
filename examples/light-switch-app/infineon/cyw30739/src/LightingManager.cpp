/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <LightingManager.h>

#include <stdio.h>

using namespace chip;

LightingManager LightingManager::sLight;
bool usr_LED1_OnOffStatus = false;

/* config LED 1 */
static wiced_led_config_t chip_lighting_led_config_1 = {
    .led    = PLATFORM_LED_1,
    .bright = 50,
};
/* config LED 2 */
static wiced_led_config_t chip_lighting_led_config_2 = {
    .led    = PLATFORM_LED_2,
    .bright = 50,
};

wiced_result_t LightingManager::Init()
{
    wiced_result_t result;
    result = wiced_led_manager_init(&chip_lighting_led_config_1);
    if (result != WICED_SUCCESS)
        printf("Init LED1 fail (%d)\n", result);

    result = wiced_led_manager_init(&chip_lighting_led_config_2);
    if (result != WICED_SUCCESS)
        printf("Init LED2 fail (%d)\n", result);

    return result;
}

void LightingManager::Set(bool state, uint8_t pin)
{
    DoSetLEDOnOff(state, pin);
}

void LightingManager::Blink(wiced_led_t led_pin, uint32_t on_period_ms, uint32_t off_period_ms)
{
    wiced_led_manager_blink_led(led_pin, on_period_ms, off_period_ms);
}

bool LightingManager::IsLightOn(void)
{
    return usr_LED1_OnOffStatus;
}

void LightingManager::DoSetLEDOnOff(bool state, uint8_t pin)
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
