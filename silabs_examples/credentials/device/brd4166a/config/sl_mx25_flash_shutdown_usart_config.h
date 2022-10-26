/***************************************************************************//**
 * @file
 * @brief SL_MX25_FLASH_SHUTDOWN_USART Config
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_MX25_FLASH_SHUTDOWN_CONFIG_H
#define SL_MX25_FLASH_SHUTDOWN_CONFIG_H

// <<< sl:start pin_tool >>>
// {usart signal=TX,RX,CLK} SL_MX25_FLASH_SHUTDOWN
// [USART_SL_MX25_FLASH_SHUTDOWN]
#define SL_MX25_FLASH_SHUTDOWN_PERIPHERAL        USART2
#define SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO     2

// USART2 TX on PK0
#define SL_MX25_FLASH_SHUTDOWN_TX_PORT           gpioPortK
#define SL_MX25_FLASH_SHUTDOWN_TX_PIN            0
#define SL_MX25_FLASH_SHUTDOWN_TX_LOC            29

// USART2 RX on PK2
#define SL_MX25_FLASH_SHUTDOWN_RX_PORT           gpioPortK
#define SL_MX25_FLASH_SHUTDOWN_RX_PIN            2
#define SL_MX25_FLASH_SHUTDOWN_RX_LOC            30

// USART2 CLK on PF7
#define SL_MX25_FLASH_SHUTDOWN_CLK_PORT          gpioPortF
#define SL_MX25_FLASH_SHUTDOWN_CLK_PIN           7
#define SL_MX25_FLASH_SHUTDOWN_CLK_LOC           18

// [USART_SL_MX25_FLASH_SHUTDOWN]

// <gpio> SL_MX25_FLASH_SHUTDOWN_CS

// $[GPIO_SL_MX25_FLASH_SHUTDOWN_CS]
#define SL_MX25_FLASH_SHUTDOWN_CS_PORT           gpioPortK
#define SL_MX25_FLASH_SHUTDOWN_CS_PIN            1

// [GPIO_SL_MX25_FLASH_SHUTDOWN_CS]$

// <<< sl:end pin_tool >>>

#endif // SL_MX25_FLASH_SHUTDOWN_CONFIG_H
