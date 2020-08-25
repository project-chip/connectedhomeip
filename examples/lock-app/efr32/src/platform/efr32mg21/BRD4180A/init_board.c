/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/*******************************************************************************
 * @file
 * @brief init_board.c
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

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#include "hal-config.h"
#else
#include "bspconfig.h"
#endif

#include "board_features.h"
#include "em_cmu.h"

#include "bsp.h"

void initBoard(void)
{

    // Enable clock for BURTC
    CMU_ClockEnable(cmuClock_BURTC, true);
#if ((HAL_VCOM_ENABLE == 1) || (HAL_USART3_ENABLE == 1) || (HAL_USART1_ENABLE == 1) || (HAL_USART0_ENABLE == 1))
#if defined(FEATURE_EXP_HEADER_USART3)
    // Enable clock for USART3
    CMU_ClockEnable(cmuClock_USART3, true);
#elif defined(FEATURE_EXP_HEADER_USART1)
    // Enable clock for USART1
    CMU_ClockEnable(cmuClock_USART1, true);
#else
    // Enable clock for USART0
    CMU_ClockEnable(cmuClock_USART0, true);
#endif
#endif //(HAL_VCOM_ENABLE == 1)
#if ((HAL_I2CSENSOR_ENABLE == 1) || (HAL_VCOM_ENABLE == 1) || (HAL_SPIDISPLAY_ENABLE == 1) || (HAL_USART3_ENABLE == 1) ||          \
     (HAL_USART1_ENABLE == 1) || (HAL_USART0_ENABLE == 1))
    // Enable clock for PRS
    CMU_ClockEnable(cmuClock_PRS, true);
    // Enable GPIO clock source
    CMU_ClockEnable(cmuClock_GPIO, true);
#endif /* ((HAL_I2CSENSOR_ENABLE == 1)                                                                                             \
           || (HAL_VCOM_ENABLE == 1)                                                                                               \
           || (HAL_SPIDISPLAY_ENABLE == 1)                                                                                         \
           || (HAL_USART3_ENABLE == 1)                                                                                             \
           || (HAL_USART1_ENABLE == 1)                                                                                             \
           || (HAL_USART0_ENABLE == 1)) */
}

void initVcomEnable(void)
{
#warning                                                                                                                           \
    "WARNING: This radio board uses the same GPIO pin to enable the VCOM port, the LCD display and the temperature sensor! Enabling any of these features might disrupt the serial pins of the EXP header!"
#if defined(HAL_I2CSENSOR_ENABLE) || defined(HAL_SPIDISPLAY_ENABLE) || defined(HAL_VCOM_ENABLE)
#if HAL_I2CSENSOR_ENABLE || HAL_SPIDISPLAY_ENABLE || HAL_VCOM_ENABLE
#define COMMON_ENABLE 1
#else
#define COMMON_ENABLE 0
#endif

#if defined(BSP_I2CSENSOR_ENABLE_PORT)
#define ENABLE_PORT BSP_I2CSENSOR_ENABLE_PORT
#define ENABLE_PIN BSP_I2CSENSOR_ENABLE_PIN
#elif defined(BSP_SPIDISPLAY_ENABLE_PORT)
#define ENABLE_PORT BSP_SPIDISPLAY_ENABLE_PORT
#define ENABLE_PIN BSP_SPIDISPLAY_ENABLE_PIN
#else
#define ENABLE_PORT BSP_VCOM_ENABLE_PORT
#define ENABLE_PIN BSP_VCOM_ENABLE_PIN
#endif

    // Enable if requested
    GPIO_PinModeSet(ENABLE_PORT, ENABLE_PIN, gpioModePushPull, COMMON_ENABLE);

#endif
}
