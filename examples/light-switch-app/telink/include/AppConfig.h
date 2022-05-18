/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// ---- Lighting Example App Config ----

// Buttons config
#define BUTTON_PORT DEVICE_DT_GET(DT_NODELABEL(gpioc))

#define BUTTON_PIN_1 2
#define BUTTON_PIN_3 3
#define BUTTON_PIN_4 1
#define BUTTON_PIN_2 0

// LEDs config
// System led config
#define SYSTEM_STATE_LED_PORT DEVICE_DT_GET(DT_NODELABEL(gpiob))
#define SYSTEM_STATE_LED_PIN 7

// Lighting LED config
#define LIGHTING_PWM_DEVICE DEVICE_DT_GET(DT_PWMS_CTLR(DT_ALIAS(pwm_led0)))
#define LIGHTING_PWM_CHANNEL DT_PWMS_CHANNEL(DT_ALIAS(pwm_led0))
