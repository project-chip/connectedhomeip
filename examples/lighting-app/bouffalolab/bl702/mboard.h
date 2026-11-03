/*
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 * @file
 *   This file includes dev borad compile-time configuration constants for BL702.
 *
 */

#pragma once

#define CHIP_UART_PORT 0
#define CHIP_UART_RX_BUFFSIZE 256

#ifdef BL706_NIGHT_LIGHT

#define LED_B_PIN_PORT 2
#define LED_B_PIN 12

#define LED_R_PIN_PORT 0
#define LED_R_PIN 10

#define LED_G_PIN_PORT 1
#define LED_G_PIN 16

#define MAX_PWM_CHANNEL 3

#elif defined(XT_ZB6_DevKit)

#define LED_PIN_PORT 3
#define LED_PIN 18

#define BOOT_PIN_RESET 31

#define MAX_PWM_CHANNEL 1

#else

#define LED_PIN_PORT 2
#define LED_PIN 22

#define BOOT_PIN_RESET 31

#define MAX_PWM_CHANNEL 1

#endif

#define CHIP_UART_PIN_RX 15
#define CHIP_UART_PIN_TX 14
