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
 * @brief hal-config.h
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

#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "board_features.h"
#include "hal-config-app-common.h"
#include "hal-config-board.h"

#ifndef HAL_VCOM_ENABLE
#define HAL_VCOM_ENABLE (0)
#endif
#ifndef HAL_I2CSENSOR_ENABLE
#define HAL_I2CSENSOR_ENABLE (0)
#endif

#ifndef HAL_SPIDISPLAY_ENABLE
#define HAL_SPIDISPLAY_ENABLE (1)
#endif
#define HAL_SPIDISPLAY_EXTCOMIN_CALLBACK
#if defined(FEATURE_IOEXPANDER)
#define HAL_SPIDISPLAY_EXTMODE_EXTCOMIN (0)
#else
#define HAL_SPIDISPLAY_EXTMODE_EXTCOMIN (1)
#endif
#define HAL_SPIDISPLAY_EXTMODE_SPI (0)
#define HAL_SPIDISPLAY_EXTCOMIN_USE_PRS (0)
#define HAL_SPIDISPLAY_EXTCOMIN_USE_CALLBACK (0)
#define HAL_SPIDISPLAY_FREQUENCY (1000000)

#define HAL_CLK_HFCLK_SOURCE (HAL_CLK_HFCLK_SOURCE_HFXO)
#define HAL_CLK_LFECLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFXO)
#define HAL_CLK_LFBCLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFXO)
#define HAL_CLK_LFACLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFRCO)
#define HAL_CLK_HFXO_AUTOSTART (HAL_CLK_HFXO_AUTOSTART_NONE)

#define HAL_DCDC_BYPASS (0)

// $[PA]
#define HAL_PA_ENABLE (1)

// $[SERIAL]
#define HAL_SERIAL_USART0_ENABLE (0)
#define HAL_SERIAL_LEUART0_ENABLE (0)
#define HAL_SERIAL_USART1_ENABLE (0)
#define HAL_SERIAL_USART2_ENABLE (0)
#define HAL_SERIAL_USART3_ENABLE (0)
#define HAL_SERIAL_RXWAKE_ENABLE (0)

#define HAL_SERIAL_APP_RX_QUEUE_SIZE (128)
#define HAL_SERIAL_APP_BAUD_RATE (115200)
#define HAL_SERIAL_APP_RXSTOP (16)
#define HAL_SERIAL_APP_RXSTART (16)
#define HAL_SERIAL_APP_TX_QUEUE_SIZE (128)
#define HAL_SERIAL_APP_FLOW_CONTROL (HAL_USART_FLOW_CONTROL_NONE)

// $[USART0]
#define HAL_USART0_ENABLE (1)
#define HAL_USART0_RX_QUEUE_SIZE (128)
#define HAL_USART0_BAUD_RATE (115200)
#define HAL_USART0_RXSTOP (16)
#define HAL_USART0_RXSTART (16)
#define HAL_USART0_TX_QUEUE_SIZE (128)
#define HAL_USART0_FLOW_CONTROL (HAL_USART_FLOW_CONTROL_NONE)

#endif
