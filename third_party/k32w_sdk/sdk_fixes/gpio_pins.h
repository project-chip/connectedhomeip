/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GPIO_PINS_H__
#define __GPIO_PINS_H__

#include "GPIO_Adapter.h"

/*! @file */
/*!*/
/*! This file contains gpio pin definitions used by gpio peripheral driver.*/
/*! The enums in _gpio_pins map to the real gpio pin numbers defined in*/
/*! gpioPinLookupTable. And this might be different in different board.*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (defined OM15082)
#define BOARD_LED_RED1_GPIO GPIO
#define BOARD_LED_RED1_GPIO_PORT 0U
#define BOARD_LED_RED1_GPIO_PIN 6U
#define IOCON_LED_RED1_PIN BOARD_LED_RED1_GPIO_PIN

#define BOARD_LED_RED2_GPIO GPIO
#define BOARD_LED_RED2_GPIO_PORT 0U
#define BOARD_LED_RED2_GPIO_PIN 3U
#define IOCON_LED_RED2_PIN BOARD_LED_RED2_GPIO_PIN

#define IOCON_LED_MODE_FUNC (0U)
#else
/* There are 2 red LEDs on DK6 board: PIO0 and PIO3 */
#define BOARD_LED_RED1_GPIO GPIO
#define BOARD_LED_RED1_GPIO_PORT 0U
#define BOARD_LED_RED1_GPIO_PIN 0U
#define IOCON_LED_RED1_PIN BOARD_LED_RED1_GPIO_PIN

#define BOARD_LED_RED2_GPIO GPIO
#define BOARD_LED_RED2_GPIO_PORT 0U
#define BOARD_LED_RED2_GPIO_PIN 3U
#define IOCON_LED_RED2_PIN BOARD_LED_RED2_GPIO_PIN

#define IOCON_LED_MODE_FUNC (0U)
#endif /* OM15082 */

#if !(defined OM15082)
/* We have 2 switch push-buttons on DK6 board */
#define BOARD_USER_BUTTON1_GPIO GPIO
#define BOARD_USER_BUTTON1_GPIO_PORT 0U
#define BOARD_USER_BUTTON1_GPIO_PIN 1U
#define IOCON_USER_BUTTON1_PIN BOARD_USER_BUTTON1_GPIO_PIN

#define BOARD_USER_BUTTON2_GPIO GPIO
#define BOARD_USER_BUTTON2_GPIO_PORT 0U
#define BOARD_USER_BUTTON2_GPIO_PIN 5U /* shared with ISP entry */
#define IOCON_USER_BUTTON2_PIN BOARD_USER_BUTTON2_GPIO_PIN

#else
/* We have 3 extra switch push-buttons on OM15082 board */
#define BOARD_USER_BUTTON1_GPIO GPIO
#define BOARD_USER_BUTTON1_GPIO_PORT 0U
#define BOARD_USER_BUTTON1_GPIO_PIN 15U
#define IOCON_USER_BUTTON1_PIN BOARD_USER_BUTTON1_GPIO_PIN

#define BOARD_USER_BUTTON2_GPIO GPIO
#define BOARD_USER_BUTTON2_GPIO_PORT 0U
#define BOARD_USER_BUTTON2_GPIO_PIN 7U
#define IOCON_USER_BUTTON2_PIN BOARD_USER_BUTTON2_GPIO_PIN

#define BOARD_USER_BUTTON3_GPIO GPIO
#define BOARD_USER_BUTTON3_GPIO_PORT 0U
#define BOARD_USER_BUTTON3_GPIO_PIN 4U
#define IOCON_USER_BUTTON3_PIN BOARD_USER_BUTTON3_GPIO_PIN

#define BOARD_USER_BUTTON4_GPIO GPIO
#define BOARD_USER_BUTTON4_GPIO_PORT 0U
#define BOARD_USER_BUTTON4_GPIO_PIN 1U
#define IOCON_USER_BUTTON4_PIN BOARD_USER_BUTTON4_GPIO_PIN

#endif /* OM15082 */

#define IOCON_USER_BUTTON_MODE_FUNC (0U)

/* Battery level input */
#define BOARD_BAT_LEVEL_GPIO_PORT (0U)
#define BOARD_BAT_LEVEL_GPIO_PIN (14U)

extern const gpioInputPinConfig_t dk6_button_io_pins[];
extern const gpioOutputPinConfig_t dk6_leds_io_pins[];

#define ledPins dk6_leds_io_pins
#define switchPins dk6_button_io_pins

#endif /* __GPIO_PINS_H__ */
