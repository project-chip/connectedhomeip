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

#pragma once

#include "board_features.h"
#include "em_device.h"
#include "hal-config-app-common.h"
#include "hal-config-types.h"

#ifndef HAL_VCOM_ENABLE
#define HAL_VCOM_ENABLE (1)
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

// $[CMU]
#define HAL_CLK_HFCLK_SOURCE (HAL_CLK_HFCLK_SOURCE_HFXO)
#define HAL_CLK_EM4CLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFRCO)
#define HAL_CLK_EM23CLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFRCO)
#define HAL_CLK_RTCCCLK_SOURCE (HAL_CLK_LFCLK_SOURCE_LFRCO)

// $[SERIAL]
#define HAL_SERIAL_USART0_ENABLE (0)
#define HAL_SERIAL_LEUART0_ENABLE (0)
#define HAL_SERIAL_USART1_ENABLE (0)
#define HAL_SERIAL_USART2_ENABLE (0)
#define HAL_SERIAL_USART3_ENABLE (0)
#define HAL_SERIAL_RXWAKE_ENABLE (0)
#define HAL_SERIAL_APP_RX_QUEUE_SIZE (128UL)
#define HAL_SERIAL_APP_BAUD_RATE (115200UL)
#define HAL_SERIAL_APP_RXSTOP (16UL)
#define HAL_SERIAL_APP_RXSTART (16UL)
#define HAL_SERIAL_APP_TX_QUEUE_SIZE (128UL)
#define HAL_SERIAL_APP_FLOW_CONTROL (HAL_USART_FLOW_CONTROL_HWUART)

// $[USART0]
#define HAL_USART0_ENABLE (1)
#define HAL_USART0_RX_QUEUE_SIZE (128UL)
#define HAL_USART0_BAUD_RATE (115200UL)
#define HAL_USART0_RXSTOP (16UL)
#define HAL_USART0_RXSTART (16UL)
#define HAL_USART0_TX_QUEUE_SIZE (128UL)
#define HAL_USART0_FLOW_CONTROL (HAL_USART_FLOW_CONTROL_HWUART)

// $[BTL_BUTTON]

#define BSP_BTL_BUTTON_PIN (2U)
#define BSP_BTL_BUTTON_PORT (gpioPortD)

// [BTL_BUTTON]$

// $[BUTTON]
#define BSP_BUTTON_PRESENT (1)

#define BSP_BUTTON0_PIN (2U)
#define BSP_BUTTON0_PORT (gpioPortD)

#define BSP_BUTTON1_PIN (3U)
#define BSP_BUTTON1_PORT (gpioPortD)

#define BSP_BUTTON_COUNT (2U)
#define BSP_BUTTON_INIT                                                                                                            \
    {                                                                                                                              \
        { BSP_BUTTON0_PORT, BSP_BUTTON0_PIN }, { BSP_BUTTON1_PORT, BSP_BUTTON1_PIN }                                               \
    }
#define BSP_BUTTON_GPIO_DOUT (HAL_GPIO_DOUT_LOW)
#define BSP_BUTTON_GPIO_MODE (HAL_GPIO_MODE_INPUT)
// [BUTTON]$

// $[CMU]
#define BSP_CLK_HFXO_PRESENT (1)
#define BSP_CLK_HFXO_FREQ (38400000UL)
#define BSP_CLK_HFXO_INIT CMU_HFXOINIT_DEFAULT
#define BSP_CLK_HFXO_CTUNE (129)
#define BSP_CLK_LFXO_PRESENT (1)
#define BSP_CLK_LFXO_INIT CMU_LFXOINIT_DEFAULT
#define BSP_CLK_LFXO_FREQ (32768U)
#define BSP_CLK_LFXO_CTUNE (79U)
// [CMU]$

// $[GPIO]
#define PORTIO_GPIO_SWV_PIN (3U)
#define PORTIO_GPIO_SWV_PORT (gpioPortA)

#define BSP_TRACE_SWO_PIN (3U)
#define BSP_TRACE_SWO_PORT (gpioPortA)

// [GPIO]$

// $[LED]
#define BSP_LED_PRESENT (1)

#define BSP_LED0_PIN (0U)
#define BSP_LED0_PORT (gpioPortB)

#define BSP_LED1_PIN (1U)
#define BSP_LED1_PORT (gpioPortB)

#define BSP_LED_COUNT (2U)
#define BSP_LED_INIT                                                                                                               \
    {                                                                                                                              \
        { BSP_LED0_PORT, BSP_LED0_PIN }, { BSP_LED1_PORT, BSP_LED1_PIN }                                                           \
    }
#define BSP_LED_POLARITY (1)
// [LED]$

// $[PA]

#define BSP_PA_VOLTAGE (3300U)
// [PA]$

// $[PRS]
#define PORTIO_PRS_ASYNCH4_PIN (0U)
#define PORTIO_PRS_ASYNCH4_PORT (gpioPortA)

// [PRS]$

// $[PTI]
#define PORTIO_PTI_DFRAME_PIN (5U)
#define PORTIO_PTI_DFRAME_PORT (gpioPortC)

#define PORTIO_PTI_DOUT_PIN (4U)
#define PORTIO_PTI_DOUT_PORT (gpioPortC)

#define BSP_PTI_DFRAME_PIN (5U)
#define BSP_PTI_DFRAME_PORT (gpioPortC)

#define BSP_PTI_DOUT_PIN (4U)
#define BSP_PTI_DOUT_PORT (gpioPortC)

// [PTI]$

// $[SERIAL]
#define BSP_SERIAL_APP_TX_PIN (5U)
#define BSP_SERIAL_APP_TX_PORT (gpioPortA)

#define BSP_SERIAL_APP_RX_PIN (6U)
#define BSP_SERIAL_APP_RX_PORT (gpioPortA)

#define BSP_SERIAL_APP_CTS_PIN (4U)
#define BSP_SERIAL_APP_CTS_PORT (gpioPortA)

#define BSP_SERIAL_APP_RTS_PIN (1U)
#define BSP_SERIAL_APP_RTS_PORT (gpioPortC)

// [SERIAL]$

// $[SPIDISPLAY]

#define BSP_SPIDISPLAY_CS_PIN (3U)
#define BSP_SPIDISPLAY_CS_PORT (gpioPortC)

#define BSP_SPIDISPLAY_ENABLE_PIN (4U)
#define BSP_SPIDISPLAY_ENABLE_PORT (gpioPortD)

#define BSP_SPIDISPLAY_EXTCOMIN_PIN (0U)
#define BSP_SPIDISPLAY_EXTCOMIN_PORT (gpioPortA)

#define BSP_SPIDISPLAY_DISPLAY (HAL_DISPLAY_SHARP_LS013B7DH03)
#define BSP_SPIDISPLAY_USART (HAL_SPI_PORT_USART2)
#define BSP_SPIDISPLAY_EXTCOMIN_CHANNEL (4)
#define BSP_SPIDISPLAY_MOSI_PIN (0U)
#define BSP_SPIDISPLAY_MOSI_PORT (gpioPortC)

#define BSP_SPIDISPLAY_MISO_PIN (1U)
#define BSP_SPIDISPLAY_MISO_PORT (gpioPortC)

#define BSP_SPIDISPLAY_CLK_PIN (2U)
#define BSP_SPIDISPLAY_CLK_PORT (gpioPortC)

// [SPIDISPLAY]$

// $[SPINCP]
#define BSP_SPINCP_NHOSTINT_PIN (2U)
#define BSP_SPINCP_NHOSTINT_PORT (gpioPortD)

#define BSP_SPINCP_NWAKE_PIN (3U)
#define BSP_SPINCP_NWAKE_PORT (gpioPortD)

#define BSP_SPINCP_USART_PORT (HAL_SPI_PORT_USART2)
#define BSP_SPINCP_MOSI_PIN (0U)
#define BSP_SPINCP_MOSI_PORT (gpioPortC)

#define BSP_SPINCP_MISO_PIN (1U)
#define BSP_SPINCP_MISO_PORT (gpioPortC)

#define BSP_SPINCP_CLK_PIN (2U)
#define BSP_SPINCP_CLK_PORT (gpioPortC)

#define BSP_SPINCP_CS_PIN (3U)
#define BSP_SPINCP_CS_PORT (gpioPortC)

// [SPINCP]$

// $[UARTNCP]
#define BSP_UARTNCP_USART_PORT (HAL_SERIAL_PORT_USART0)
#define BSP_UARTNCP_TX_PIN (5U)
#define BSP_UARTNCP_TX_PORT (gpioPortA)

#define BSP_UARTNCP_RX_PIN (6U)
#define BSP_UARTNCP_RX_PORT (gpioPortA)

#define BSP_UARTNCP_CTS_PIN (4U)
#define BSP_UARTNCP_CTS_PORT (gpioPortA)

#define BSP_UARTNCP_RTS_PIN (1U)
#define BSP_UARTNCP_RTS_PORT (gpioPortC)

// [UARTNCP]$

// $[USART0]
#define PORTIO_USART0_CTS_PIN (4U)
#define PORTIO_USART0_CTS_PORT (gpioPortA)

#define PORTIO_USART0_RTS_PIN (1U)
#define PORTIO_USART0_RTS_PORT (gpioPortC)

#define PORTIO_USART0_RX_PIN (6U)
#define PORTIO_USART0_RX_PORT (gpioPortA)

#define PORTIO_USART0_TX_PIN (5U)
#define PORTIO_USART0_TX_PORT (gpioPortA)

#define BSP_USART0_TX_PIN (5U)
#define BSP_USART0_TX_PORT (gpioPortA)

#define BSP_USART0_RX_PIN (6U)
#define BSP_USART0_RX_PORT (gpioPortA)

#define BSP_USART0_CTS_PIN (4U)
#define BSP_USART0_CTS_PORT (gpioPortA)

#define BSP_USART0_RTS_PIN (1U)
#define BSP_USART0_RTS_PORT (gpioPortC)

// [USART0]$

// $[USART2]
#define PORTIO_USART2_CLK_PIN (2U)
#define PORTIO_USART2_CLK_PORT (gpioPortC)

#define PORTIO_USART2_CS_PIN (3U)
#define PORTIO_USART2_CS_PORT (gpioPortC)

#define PORTIO_USART2_RX_PIN (1U)
#define PORTIO_USART2_RX_PORT (gpioPortC)

#define PORTIO_USART2_TX_PIN (0U)
#define PORTIO_USART2_TX_PORT (gpioPortC)

#define BSP_USART2_MOSI_PIN (0U)
#define BSP_USART2_MOSI_PORT (gpioPortC)

#define BSP_USART2_MISO_PIN (1U)
#define BSP_USART2_MISO_PORT (gpioPortC)

#define BSP_USART2_CLK_PIN (2U)
#define BSP_USART2_CLK_PORT (gpioPortC)

#define BSP_USART2_CS_PIN (3U)
#define BSP_USART2_CS_PORT (gpioPortC)

// [USART2]$

// $[VCOM]

#define BSP_VCOM_ENABLE_PIN (4U)
#define BSP_VCOM_ENABLE_PORT (gpioPortD)

// [VCOM]$

#if defined(_SILICON_LABS_MODULE)
#include "sl_module.h"
#endif
