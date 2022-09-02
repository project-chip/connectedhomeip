/***************************************************************************//**
 * @file
 * @brief LED Driver Instances
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_SIMPLE_LED_INSTANCES_H
#define SL_SIMPLE_LED_INSTANCES_H

#include "sl_simple_led.h"

extern const sl_led_t sl_led_led0;

extern const sl_led_t *sl_simple_led_array[];

#define SL_SIMPLE_LED_COUNT 1
#define SL_SIMPLE_LED_INSTANCE(n) (sl_simple_led_array[n])

void sl_simple_led_init_instances(void);

#endif // SL_SIMPLE_LED_INIT_H
