/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

// Buttons config
#define BUTTON_PORT DEVICE_DT_GET(DT_NODELABEL(gpioc))

#define BUTTON_PIN_1 2
#define BUTTON_PIN_3 3
#define BUTTON_PIN_4 1
#define BUTTON_PIN_2 0

// LEDs config
#define LEDS_PORT DEVICE_DT_GET(DT_NODELABEL(gpiob))
#define SYSTEM_STATE_LED 7
#if APP_USE_IDENTIFY_PWM
#define LIGHTING_PWM_SPEC_IDENTIFY_GREEN PWM_DT_SPEC_GET(DT_ALIAS(pwm_led3))
#endif
