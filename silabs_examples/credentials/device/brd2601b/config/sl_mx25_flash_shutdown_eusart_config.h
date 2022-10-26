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
// {eusart signal=TX,RX,SCLK} SL_MX25_FLASH_SHUTDOWN
// [EUSART_SL_MX25_FLASH_SHUTDOWN]
#define SL_MX25_FLASH_SHUTDOWN_PERIPHERAL        EUSART1
#define SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO     1

// EUSART1 TX on PC03
#define SL_MX25_FLASH_SHUTDOWN_TX_PORT           gpioPortC
#define SL_MX25_FLASH_SHUTDOWN_TX_PIN            3

// EUSART1 RX on PC02
#define SL_MX25_FLASH_SHUTDOWN_RX_PORT           gpioPortC
#define SL_MX25_FLASH_SHUTDOWN_RX_PIN            2

// EUSART1 SCLK on PC01
#define SL_MX25_FLASH_SHUTDOWN_SCLK_PORT         gpioPortC
#define SL_MX25_FLASH_SHUTDOWN_SCLK_PIN          1

// [EUSART_SL_MX25_FLASH_SHUTDOWN]

// <gpio> SL_MX25_FLASH_SHUTDOWN_CS

// $[GPIO_SL_MX25_FLASH_SHUTDOWN_CS]
#define SL_MX25_FLASH_SHUTDOWN_CS_PORT           gpioPortC
#define SL_MX25_FLASH_SHUTDOWN_CS_PIN            0

// [GPIO_SL_MX25_FLASH_SHUTDOWN_CS]$

// <<< sl:end pin_tool >>>

#endif // SL_MX25_FLASH_SHUTDOWN_CONFIG_H
