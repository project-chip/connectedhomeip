/***************************************************************************//**
 * @file
 * @brief Simple Button Driver User Config
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

#ifndef SL_SIMPLE_BUTTON_BTN0_CONFIG_H
#define SL_SIMPLE_BUTTON_BTN0_CONFIG_H

#include "em_gpio.h"
#include "sl_simple_button.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <o SL_SIMPLE_BUTTON_BTN0_MODE>
// <SL_SIMPLE_BUTTON_MODE_INTERRUPT=> Interrupt
// <SL_SIMPLE_BUTTON_MODE_POLL_AND_DEBOUNCE=> Poll and Debounce
// <SL_SIMPLE_BUTTON_MODE_POLL=> Poll
// <i> Default: SL_SIMPLE_BUTTON_MODE_INTERRUPT
#define SL_SIMPLE_BUTTON_BTN0_MODE       SL_SIMPLE_BUTTON_MODE_INTERRUPT
// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_SIMPLE_BUTTON_BTN0
// $[GPIO_SL_SIMPLE_BUTTON_BTN0]
#define SL_SIMPLE_BUTTON_BTN0_PORT               gpioPortD
#define SL_SIMPLE_BUTTON_BTN0_PIN                14

// [GPIO_SL_SIMPLE_BUTTON_BTN0]$

// <<< sl:end pin_tool >>>

#endif // SL_SIMPLE_BUTTON_BTN0_CONFIG_H
