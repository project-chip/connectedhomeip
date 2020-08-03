/***************************************************************************//**
 * @file
 * @brief hal-config-app-common.h
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

#ifndef HAL_CONFIG_APP_COMMON_H
#define HAL_CONFIG_APP_COMMON_H

#include "em_device.h"
#include "hal-config-types.h"

#if defined(FEATURE_IOEXPANDER)
#include "hal-config-ioexp.h"
#endif


#define HAL_EXTFLASH_FREQUENCY                        (1000000)

#define HAL_PTI_ENABLE                                (1)
#define HAL_PTI_MODE                                  (HAL_PTI_MODE_UART)
#define HAL_PTI_BAUD_RATE                             (1600000)

 
#define HAL_PA_RAMP                                   (10)
#define HAL_PA_2P4_LOWPOWER                           (0)
#define HAL_PA_POWER                                  (252)
#define HAL_PA_CURVE_HEADER                            "pa_curves_efr32.h"


#define HAL_PA_VOLTAGE                     BSP_PA_VOLTAGE

// Select antenna path on EFR32xG2x devices:
//   - RF2G2_IO1: 0
//   - RF2G2_IO2: 1
#define GECKO_RF_ANTENNA   1
#if defined(FEATURE_EXP_HEADER_USART3)

#define BSP_EXP_USART           USART3

#define BSP_EXP_USART_CTS_PIN   BSP_USART3_CTS_PIN
#define BSP_EXP_USART_CTS_PORT  BSP_USART3_CTS_PORT
#define BSP_EXP_USART_CTS_LOC   BSP_USART3_CTS_LOC

#define BSP_EXP_USART_RTS_PIN   BSP_USART3_RTS_PIN
#define BSP_EXP_USART_RTS_PORT  BSP_USART3_RTS_PORT
#define BSP_EXP_USART_RTS_LOC   BSP_USART3_RTS_LOC

#define BSP_EXP_USART_RX_PIN    BSP_USART3_RX_PIN
#define BSP_EXP_USART_RX_PORT   BSP_USART3_RX_PORT
#define BSP_EXP_USART_RX_LOC    BSP_USART3_RX_LOC

#define BSP_EXP_USART_TX_PIN    BSP_USART3_TX_PIN
#define BSP_EXP_USART_TX_PORT   BSP_USART3_TX_PORT
#define BSP_EXP_USART_TX_LOC    BSP_USART3_TX_LOC

#elif defined(FEATURE_EXP_HEADER_USART1)

#define BSP_EXP_USART           USART1

#define BSP_EXP_USART_CTS_PIN   BSP_USART1_CTS_PIN
#define BSP_EXP_USART_CTS_PORT  BSP_USART1_CTS_PORT
#define BSP_EXP_USART_CTS_LOC   BSP_USART1_CTS_LOC

#define BSP_EXP_USART_RTS_PIN   BSP_USART1_RTS_PIN
#define BSP_EXP_USART_RTS_PORT  BSP_USART1_RTS_PORT
#define BSP_EXP_USART_RTS_LOC   BSP_USART1_RTS_LOC

#define BSP_EXP_USART_RX_PIN    BSP_USART1_RX_PIN
#define BSP_EXP_USART_RX_PORT   BSP_USART1_RX_PORT
#define BSP_EXP_USART_RX_LOC    BSP_USART1_RX_LOC

#define BSP_EXP_USART_TX_PIN    BSP_USART1_TX_PIN
#define BSP_EXP_USART_TX_PORT   BSP_USART1_TX_PORT
#define BSP_EXP_USART_TX_LOC    BSP_USART1_TX_LOC

#else

#define BSP_EXP_USART           USART0

#define BSP_EXP_USART_CTS_PIN   BSP_USART0_CTS_PIN
#define BSP_EXP_USART_CTS_PORT  BSP_USART0_CTS_PORT
#define BSP_EXP_USART_CTS_LOC   BSP_USART0_CTS_LOC

#define BSP_EXP_USART_RTS_PIN   BSP_USART0_RTS_PIN
#define BSP_EXP_USART_RTS_PORT  BSP_USART0_RTS_PORT
#define BSP_EXP_USART_RTS_LOC   BSP_USART0_RTS_LOC

#define BSP_EXP_USART_RX_PIN    BSP_USART0_RX_PIN
#define BSP_EXP_USART_RX_PORT   BSP_USART0_RX_PORT
#define BSP_EXP_USART_RX_LOC    BSP_USART0_RX_LOC

#define BSP_EXP_USART_TX_PIN    BSP_USART0_TX_PIN
#define BSP_EXP_USART_TX_PORT   BSP_USART0_TX_PORT
#define BSP_EXP_USART_TX_LOC    BSP_USART0_TX_LOC

#endif // FEATURE_EXP_HEADER_USART3
                          
#endif /* HAL_CONFIG_APP_COMMON_H */