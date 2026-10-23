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
 *   This file includes dev borad compile-time configuration constants for BL61X.
 *
 */

#pragma once

#define CHIP_UART_PORT 0
#define CHIP_UART_RX_BUFFSIZE 256

#if defined(BL616CL)
#define BOOT_PIN_RESET 38
#elif defined(BL616)
#define BOOT_PIN_RESET 2
#endif

#define LED_B_PWM_CH 0
#define LED_B_PIN 0

#define LED_R_PWM_CH 1
#define LED_R_PIN 1

#define LED_G_PWM_CH 2
#define LED_G_PIN 30

#define MAX_PWM_CHANNEL 3

#define CHIP_UART_PIN_RX 7
#define CHIP_UART_PIN_TX 16
