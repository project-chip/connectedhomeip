/***************************************************************************//**
 * @file
 * @brief Simple Led Driver Configuration
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

#ifndef SL_SIMPLE_LED_LED0_CONFIG_H
#define SL_SIMPLE_LED_LED0_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Simple LED configuration
// <o SL_SIMPLE_LED_LED0_POLARITY>
// <SL_SIMPLE_LED_POLARITY_ACTIVE_LOW=> Active low
// <SL_SIMPLE_LED_POLARITY_ACTIVE_HIGH=> Active high
// <i> Default: SL_SIMPLE_LED_POLARITY_ACTIVE_HIGH
#define SL_SIMPLE_LED_LED0_POLARITY SL_SIMPLE_LED_POLARITY_ACTIVE_LOW
// </h> end led configuration

// <<< end of configuration section >>>

// <<< sl:start pin_tool >>>

// <gpio> SL_SIMPLE_LED_LED0
// $[GPIO_SL_SIMPLE_LED_LED0]
#define SL_SIMPLE_LED_LED0_PORT                  gpioPortF
#define SL_SIMPLE_LED_LED0_PIN                   6

// [GPIO_SL_SIMPLE_LED_LED0]$

// <<< sl:end pin_tool >>>

#endif // SL_SIMPLE_LED_LED0_CONFIG_H
