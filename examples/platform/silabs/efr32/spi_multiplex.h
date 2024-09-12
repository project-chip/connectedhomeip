/**
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

#pragma once

#ifndef SL_LCDCTRL_MUX
#define SL_LCDCTRL_MUX (EFR32MG24 && SL_WIFI && DISPLAY_ENABLED)
#endif // SL_LCDCTRL_MUX

#ifndef SL_UARTCTRL_MUX
#define SL_UARTCTRL_MUX (EFR32MG24 && WF200_WIFI && ENABLE_CHIP_SHELL)
#endif // SL_UARTCTRL_MUX

#ifndef SL_MX25CTRL_MUX
#define SL_MX25CTRL_MUX (EFR32MG24 && SL_WIFI && CONFIG_USE_EXTERNAL_FLASH)
#endif // SL_MX25CTRL_MUX

#ifndef SL_BTLCTRL_MUX
#define SL_BTLCTRL_MUX (EFR32MG24 && SL_WIFI && CONFIG_USE_EXTERNAL_FLASH)
#endif // SL_BTLCTRL_MUX

#ifndef SL_SPICTRL_MUX
#define SL_SPICTRL_MUX (EFR32MG24 && SL_WIFI && (SL_LCDCTRL_MUX || SL_UARTCTRL_MUX || SL_MX25CTRL_MUX || SL_BTLCTRL_MUX))
#endif // SL_SPICTRL_MUX

#if SL_SPICTRL_MUX
#ifdef __cplusplus
extern "C" {
#endif

#if SL_SPICTRL_MUX
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
#endif // SL_SPICTRL_MUX

#if SL_MUX25CTRL_MUX
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
#endif // SL_MUX25CTRL_MUX

#if SL_BTLCTRL_MUX
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
#endif // SL_BTLCTRL_MUX

#if SL_LCDCTRL_MUX
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
#endif // SL_LCDCTRL_MUX

#if SL_UARTCTRL_MUX
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
#endif // SL_UARTCTRL_MUX

#ifdef __cplusplus
}
#endif
#endif // SL_SPICTRL_MUX
