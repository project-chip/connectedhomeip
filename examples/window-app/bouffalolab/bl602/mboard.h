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
 *   This file includes dev borad compile-time configuration constants for BL602.
 *
 */

#pragma once

#define CHIP_UART_PORT 0
#define CHIP_UART_RX_BUFFSIZE 256

#define LCD_SIZE 128
#define LCD_MARGIN_SIZE 1
#define LCD_BORDER_SIZE 2
#define LCD_FRAME_SIZE (2 * LCD_MARGIN_SIZE + LCD_BORDER_SIZE)
#define LCD_COVER_SIZE (LCD_SIZE - 2 * LCD_FRAME_SIZE)
#define LIFT_OPEN_LIMIT 0
#define LIFT_CLOSED_LIMIT (LCD_COVER_SIZE - 1)
#define LIFT_DELTA 1000 // 10%
#define TILT_OPEN_LIMIT 0
#define TILT_CLOSED_LIMIT (LCD_COVER_SIZE / 10 - 1)
#define TILT_DELTA 1000 // 10%

#define WINDOW_COVER_COUNT 2

#ifndef WINDOW_COVER_ENDPOINT1
#define WINDOW_COVER_ENDPOINT1 1
#endif

#ifndef WINDOW_COVER_ENDPOINT2
#define WINDOW_COVER_ENDPOINT2 2
#endif


#ifndef LONG_PRESS_TIMEOUT
#define LONG_PRESS_TIMEOUT 3000
#endif

#ifndef COVER_LIFT_TILT_TIMEOUT
#define COVER_LIFT_TILT_TIMEOUT 500
#endif

#define CHIP_UART_PIN_RX 7
#define CHIP_UART_PIN_TX 16