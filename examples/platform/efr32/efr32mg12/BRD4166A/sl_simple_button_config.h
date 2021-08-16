/***************************************************************************//**
 * @file
 * @brief Simple Button Driver Configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_SIMPLE_BUTTON_CONFIG_H
#define SL_SIMPLE_BUTTON_CONFIG_H

#include "em_gpio.h"
// <<< Use Configuration Wizard in Context Menu >>>

// <h> Simple Button Driver Configuration

// <o SL_SIMPLE_BUTTON_DEBOUNCE_BITS> Number of bits <1-15>
// <i> Default: 5
// <i> Defines the number of calls to sl_simple_button_poll_step wherein the button
// <i> state must remain the same before the button state is considered debounced
#define SL_SIMPLE_BUTTON_DEBOUNCE_BITS    5U

// <o SL_SIMPLE_BUTTON_GPIO_MODE>
// <gpioModeInput=> GPIO Input
// <gpioModeInputPull=> GPIO Input Pull
// <gpioModeInputPullFilter=> GPIO Input Pull Filter
// <i> Default: gpioModeInput
#define SL_SIMPLE_BUTTON_GPIO_MODE        gpioModeInput

// <q SL_SIMPLE_BUTTON_GPIO_DOUT>
// <i> SL_SIMPLE_BUTTON_GPIO_MODE == gpioModeInput, Filter if DOUT is set
// <i> SL_SIMPLE_BUTTON_GPIO_MODE == gpioModeInputPull, DOUT determines pull direction
// <i> SL_SIMPLE_BUTTON_GPIO_MODE == gpioModeInputPullFilter, DOUT determines pull direction
#define SL_SIMPLE_BUTTON_GPIO_DOUT        0U

// <q SL_SIMPLE_BUTTON_POLARITY>
// <i> 0  Active Low
// <i> 1  Active High
// <i> Default: 0
#define SL_SIMPLE_BUTTON_POLARITY         0U

// </h>

// <<< end of configuration section >>>
#endif // SL_SIMPLE_BUTTON_CONFIG_H
