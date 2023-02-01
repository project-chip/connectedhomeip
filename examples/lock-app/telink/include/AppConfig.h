// moved from Telink light-switch-app
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
#define LEDS_PORT DEVICE_DT_GET(DT_NODELABEL(gpiob))
#define SYSTEM_STATE_LED 7
#define LOCK_STATE_LED 5
#define LOCK_BUTTON_MASK 0x01 // TODO: find real mask
#define LIGHTING_PWM_SPEC_IDENTIFY_GREEN PWM_DT_SPEC_GET(DT_ALIAS(pwm_led3))

// from NRF AppConfig ======================================================

// #include "BoardUtil.h"

// // ---- Lock Example App Config ----

// #define FUNCTION_BUTTON DK_BTN1
// #define FUNCTION_BUTTON_MASK DK_BTN1_MSK

// #if NUMBER_OF_BUTTONS == 2
// #define BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON DK_BTN2
// #define BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON_MASK DK_BTN2_MSK
// #else
// #define LOCK_BUTTON DK_BTN2
// #define LOCK_BUTTON_MASK DK_BTN2_MSK
// #define BLE_ADVERTISEMENT_START_BUTTON DK_BTN4
// #define BLE_ADVERTISEMENT_START_BUTTON_MASK DK_BTN4_MSK
// #endif

// #define SYSTEM_STATE_LED DK_LED1
// #define LOCK_STATE_LED DK_LED2
// #if NUMBER_OF_LEDS == 4
// #define FACTORY_RESET_SIGNAL_LED DK_LED3
// #define FACTORY_RESET_SIGNAL_LED1 DK_LED4
// #endif