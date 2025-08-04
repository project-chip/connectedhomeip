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
#pragma once

#include <wiced_led_manager.h>

class LEDWidget
{
public:
    void Init(wiced_led_config_t * config, size_t count);
    void Set(bool state, wiced_led_t pin);
    void Blink(wiced_led_t led_pin, uint32_t on_period_ms, uint32_t off_period_ms);
    bool IsLEDOn();

private:
    void DoSetLEDOnOff(bool state, wiced_led_t pin);
    friend LEDWidget & LEDWid(void);
    bool usr_LED1_OnOffStatus;
    static LEDWidget sLEDWidget;
};

inline LEDWidget & LEDWid(void)
{
    return LEDWidget::sLEDWidget;
}
