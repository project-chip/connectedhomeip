/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef LED_DIM_PWM_TEST_CONFIGURATION_H
#define LED_DIM_PWM_TEST_CONFIGURATION_H

#define EMBER_AF_API_STACK "stack/include/ember.h"
#define EMBER_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define EMBER_AF_API_BULB_PWM_DRIVER "plugin-common/bulb-pwm-driver/bulb-pwm-driver.h"

#define emberAfCorePrintln(...)

#include "app/thread/test-app/zoip/zoip-test/simulation/thread-zclip.h"

#endif // LED_DIM_PWM_TEST_CONFIGURATION_H
