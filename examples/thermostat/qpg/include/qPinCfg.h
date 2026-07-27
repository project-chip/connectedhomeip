/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/** @file "qPinCfg.h"
 *
 * Definition of the pins used by the application
 */

#ifndef _QPINCFG_H_
#define _QPINCFG_H_

/*****************************************************************************
 *                    User Customization
 *****************************************************************************/
// Uncomment the below line to override default board file from qorvo_internal.h
// User-defined board file to put in /Applications/shared/qPinCfg/inc/boards
// #define QPINCFG_CUSTOM_BOARD_FILENAME ""

/*****************************************************************************
 *                    Common Inclusion
 *****************************************************************************/
#include "qPinCfg_Common.h"

/*****************************************************************************
 *                    Pins Definitions
 *****************************************************************************/
// Mapping of Functional buttons
#define APP_FUNCTION_BUTTON PB5_BUTTON_GPIO_PIN
#define APP_READ_TEMPERATURE_BUTTON PB1_BUTTON_GPIO_PIN

// Mapping of LEDs
#define SYSTEM_STATE_LED WHITE_COOL_LED_GPIO_PIN
#define SYSTEM_OPERATING_LED WHITE_WARM_LED_GPIO_PIN

/*****************************************************************************
 *                    Modules Definitions
 * ***************************************************************************
 * Use QPINCFG_LIST to create the following lists to configure App Modules
 *
 * QPINCFG_GENERIC_INPUT - list of input GPIOs
 * QPINCFG_GENERIC_OUTPUT - list of output GPIOs
 * QPINCFG_BUTTONS - list of GPIOs used in ButtonHandler as Buttons or Sliders
 * QPINCFG_STATUS_LED - list of GPIOs used as Status LED by GPIO manipulation
 * QPINCFG_COLOR_LED - Red, Green Blue channel GPIOs connected to the color LED
 * QPINCFG_WHITE_LEDS - Warm and Cool White channel GPIOs connected
 * QPINCFG_UNUSED - list of unused GPIOs to be pulled low
 *
 *****************************************************************************/
// --- Mapping GPIOs to Switches or Buttons
#define QPINCFG_BUTTONS QPINCFG_GPIO_LIST(APP_FUNCTION_BUTTON, APP_READ_TEMPERATURE_BUTTON)

// --- Mapping GPIOs to LEDs
#define QPINCFG_STATUS_LED QPINCFG_GPIO_LIST(SYSTEM_STATE_LED, SYSTEM_OPERATING_LED)

// --- Unused pins
#define QPINCFG_UNUSED                                                                                                             \
    QPINCFG_GPIO_LIST(PB2_BUTTON_GPIO_PIN, PB3_BUTTON_GPIO_PIN, PB4_BUTTON_GPIO_PIN, ANIO0_GPIO_PIN, ANIO1_GPIO_PIN,               \
                      EXT_32KXTAL_P, EXT_32KXTAL_N, BOARD_UNUSED_GPIO_PINS)

#endif // _QPINCFG_H_
