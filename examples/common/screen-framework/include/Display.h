/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 * @file Display.h
 *
 * This describes helper APIs for the M5Stack's Display
 *
 */

#pragma once

#include "esp_system.h"

#if CONFIG_DEVICE_TYPE_M5STACK

#define CONFIG_HAVE_DISPLAY 1
// for some reason this is backwards (turns out this is because of a 2019 update to the m5stack hw)
#define INVERT_DISPLAY INVERT_ON

#elif CONFIG_DEVICE_TYPE_ESP32_WROVER_KIT

#define CONFIG_HAVE_DISPLAY 1
#define INVERT_DISPLAY INVERT_OFF

#else

#define CONFIG_HAVE_DISPLAY 0

#endif

#if CONFIG_HAVE_DISPLAY

extern "C" {
#include "tft.h"
#include "tftspi.h"
} // extern "C"

// To reduce wear (and heat) on the screen, the display will always go off after a few seconds
#define DISPLAY_TIMEOUT_MS 30000

extern uint16_t DisplayHeight;
extern uint16_t DisplayWidth;

/**
 * @brief
 *  Initialize the M5Stack's display driver and set the default bright control and timeout
 *
 * @return esp_err_t    0 if the display driver was initialized correctly
 */
extern esp_err_t InitDisplay();
/**
 * @brief
 *  Clear the display by setting the whole screen to black
 */
extern void ClearDisplay();

/**
 * @brief
 *  Clear a portion of the display by drawing a black rectangle based on the given arguments
 *
 *  Calling this with default arguments is the same as calling `ClearDisplay()`.
 *
 * @param x_percent_start   The starting x coordinate specified as a percentage of the screen's width.
 * @param y_percent_start   The starting y coordinate specified as a percentage of the screen's height.
 * @param x_percent_end     The end x coordinate specified as a percentage of the screen's width.
 * @param y_percent_end     The end y coordinate specified as a percentage of the screen's height.
 */
extern void ClearRect(uint16_t x_percent_start = 0, uint16_t y_percent_start = 0, uint16_t x_percent_end = 100,
                      uint16_t y_percent_end = 100);
/**
 * @brief
 *  Display a status message at a given vertical position
 *
 *  The status message will be drawn from the left edge of the screen
 *
 * @param msg   The message to display
 * @param vpos  The vertical position(0-100 percent) of the message. Where 0 is the top of the screen
 */
extern void DisplayStatusMessage(char * msg, uint16_t vpos);
/**
 * @brief
 *  Reset the display timeout and set the brightness back up to default values
 *
 * @return true     If the display was woken
 */
extern bool WakeDisplay();

#endif // #if CONFIG_HAVE_DISPLAY
