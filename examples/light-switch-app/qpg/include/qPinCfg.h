/*
 * Copyright (c) 2024, Qorvo Inc
 *
 *
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 *
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
#define APP_LEVEL_BUTTON PB3_BUTTON_GPIO_PIN
#define APP_SWITCH_BUTTON PB4_BUTTON_GPIO_PIN
#define APP_TOGGLE_BUTTON PB2_BUTTON_GPIO_PIN
#define APP_COLOR_BUTTON PB1_BUTTON_GPIO_PIN

// Mapping of LEDs
#define SYSTEM_STATE_LED WHITE_COOL_LED_GPIO_PIN

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
#define QPINCFG_BUTTONS                                                                                                            \
    QPINCFG_GPIO_LIST(APP_FUNCTION_BUTTON, APP_LEVEL_BUTTON, APP_SWITCH_BUTTON, APP_TOGGLE_BUTTON, APP_COLOR_BUTTON)

// --- Mapping GPIOs to LEDs
#define QPINCFG_STATUS_LED QPINCFG_GPIO_LIST(SYSTEM_STATE_LED)

// --- Unused pins
#define QPINCFG_UNUSED                                                                                                             \
    QPINCFG_GPIO_LIST(WHITE_WARM_LED_GPIO_PIN, ANIO0_GPIO_PIN, ANIO1_GPIO_PIN, EXT_32KXTAL_P, EXT_32KXTAL_N, BOARD_UNUSED_GPIO_PINS)

#endif // _QPINCFG_H_
