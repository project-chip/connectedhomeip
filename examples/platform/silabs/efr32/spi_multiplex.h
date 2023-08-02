/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/*
 * This file contains all the functions specific to the MG24 family for
 * multiplexing the SPI port with WiFi NCP and other WSTK
 * devices such as External Flash and LCD.
 * That can be extended to other families as well.
 */

#pragma once

#if defined(EFR32MG24)
#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "em_eusart.h"
#include "semphr.h"
#include "sl_memlcd_display.h"
#include "sl_mx25_flash_shutdown_usart_config.h"
#include "sl_spidrv_instances.h"
#include "spidrv.h"

#define SL_SPIDRV_LCD_BITRATE SL_MEMLCD_SCLK_FREQ
#define SL_SPIDRV_MX25_FLASH_BITRATE 16000000

#ifdef RS911X_WIFI
#include "sl_spidrv_eusart_exp_config.h"

#define SL_SPIDRV_EXP_BITRATE_MULTIPLEXED SL_SPIDRV_EUSART_EXP_BITRATE
#define SL_SPIDRV_UART_CONSOLE_BITRATE SL_UARTDRV_EUSART_VCOM_BAUDRATE
#define SL_SPIDRV_FRAME_LENGTH SL_SPIDRV_EUSART_EXP_FRAME_LENGTH
#define SL_SPIDRV_HANDLE sl_spidrv_eusart_exp_handle

#elif WF200_WIFI
#include "sl_spidrv_exp_config.h"
#include "sl_wfx_host_api.h"

// TODO: (MATTER-1906) Investigate why using SL_SPIDRV_EXP_BITRATE is causing WF200 init failure
// REF: sl_spidrv_exp_config.h
#define SL_SPIDRV_EXP_BITRATE_MULTIPLEXED 10000000
#define SL_SPIDRV_UART_CONSOLE_BITRATE SL_UARTDRV_USART_VCOM_BAUDRATE
#define SL_SPIDRV_FRAME_LENGTH SL_SPIDRV_EXP_FRAME_LENGTH
#define SL_SPIDRV_HANDLE sl_spidrv_exp_handle
#endif /* RS911X_WIFI || WF200_WIFI */

/****************************************************************************
 * @fn  void SPIDRV_SetBaudrate()
 * @brief
 *     Sets the SPI driver to required baudrate
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void SPIDRV_SetBaudrate(uint32_t);

#if defined(RS911X_WIFI)
/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spi_cs_assert()
 * @brief
 *     Assert chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spi_cs_assert(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spi_cs_deassert()
 * @brief
 *     De-Assert chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spi_cs_deassert(void);
#endif /* RS911X_WIFI */

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spiflash_cs_assert()
 * @brief
 *     Assert SPI flash chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spiflash_cs_assert(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spiflash_cs_deassert()
 * @brief
 *     De-Assert SPI flash chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spiflash_cs_deassert(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_pre_bootloader_spi_transfer()
 * @brief
 *     Take a semaphore and controlling CS pin for EXP header and SPI flash
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_pre_bootloader_spi_transfer(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_post_bootloader_spi_transfer()
 * @brief
 *     De-Assert EXT SPI flash CS pin and release semaphore
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_post_bootloader_spi_transfer(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_pre_lcd_spi_transfer()
 * @brief
 *     Take a semaphore and setting LCD baudrate
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_pre_lcd_spi_transfer(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_post_lcd_spi_transfer()
 * @brief
 *     Release semaphore
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_post_lcd_spi_transfer(void);

#if defined(WF200_WIFI)
/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_pre_uart_transfer()
 * @brief
 *     Take a semaphore and setting GPIO, disable IRQ
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_pre_uart_transfer(void);

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_post_uart_transfer()
 * @brief
 *     Reset GPIO, enabled IRQ, release semaphore
 * @param[in] None
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_post_uart_transfer(void);
#endif /* WF200_WIFI */

#ifdef __cplusplus
}
#endif
#endif /* EFR32MG24 */
