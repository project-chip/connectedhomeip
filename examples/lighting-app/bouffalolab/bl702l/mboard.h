/*
 *  Copyright (c) 2018, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes dev borad compile-time configuration constants for BL702L.
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

// #define LED_B_PIN_PORT 3
// #define LED_B_PIN 18

// #define LED_R_PIN_PORT 4
// #define LED_R_PIN 19

// #define LED_G_PIN_PORT 0
// #define LED_G_PIN 20

#define LED_PIN_PORT 3
#define LED_PIN 18

#define MAX_PWM_CHANNEL 1

#elif defined(BL704LDK)
#define LED_PIN_PORT 1
#define LED_PIN 18

#define MAX_PWM_CHANNEL 1
#else

#define LED_PIN_PORT 2
#define LED_PIN 22

#define MAX_PWM_CHANNEL 1

#endif

#define SPI_WIFI_MISO_PIN 4
#define SPI_WIFI_MOSI_PIN 5
#define SPI_WIFI_CLK_PIN 3
#define SPI_WIFI_CS_PIN 6
#define SPI_WIFI_IRQ_PIN 10

#define CHIP_UART_PIN_RX 15
#define CHIP_UART_PIN_TX 14
