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

#include "spi_multiplex.h"
#include "dmadrv.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"

/****************************************************************************
 * @fn  void spi_drv_reinit()
 * @brief
 *     Re-Intializes SPI driver to required baudrate
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void spi_drv_reinit(uint32_t baudrate)
{
    // USART is used in MG24 + WF200 combination
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.msbf                   = true;
    usartInit.clockMode              = usartClockMode0;
    usartInit.baudrate               = baudrate;
    uint32_t databits                = SL_SPIDRV_EXP_FRAME_LENGTH - 4U + _USART_FRAME_DATABITS_FOUR;
    usartInit.databits               = (USART_Databits_TypeDef) databits;
    usartInit.autoCsEnable           = true;

    USART_InitSync(USART0, &usartInit);
}

/****************************************************************************
 * @fn  void set_spi_baudrate()
 * @brief
 *     Setting the appropriate SPI baudrate
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void set_spi_baudrate(peripheraltype_t pr_type)
{
    if (pr_type == LCD)
    {
        spi_drv_reinit(LCD_BIT_RATE);
    }
    else if (pr_type == EXP_HDR)
    {
        spi_drv_reinit(EXP_HDR_BIT_RATE);
    }
    else if (pr_type == EXT_SPIFLASH)
    {
        spi_drv_reinit(SPI_FLASH_BIT_RATE);
    }
}

/****************************************************************************
 * @fn  void spiflash_cs_assert()
 * @brief
 *     Assert SPI flash chip select.
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void spiflash_cs_assert(void)
{
    GPIO_PinOutClear(SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN);
}

/****************************************************************************
 * @fn  void spiflash_cs_deassert()
 * @brief
 *     De-Assert SPI flash chip select.
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void spiflash_cs_deassert(void)
{
    GPIO_PinOutSet(SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN);
}

/****************************************************************************
 * @fn  void pre_bootloader_spi_transfer()
 * @brief
 *     Take a semaphore and controlling CS pin for EXP header and SPI flash
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void pre_bootloader_spi_transfer(void)
{
    if (xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    /*
     * CS for Expansion header controlled within GSDK,
     * however we need to ensure CS for Expansion header is High/disabled before use of EXT SPI Flash
     */
    sl_wfx_host_spi_cs_deassert();
    /*
     * Assert CS pin for EXT SPI Flash
     */
    spiflash_cs_assert();
}

/****************************************************************************
 * @fn  void post_bootloader_spi_transfer()
 * @brief
 *     De-Assert EXT SPI flash CS pin and release semaphore
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void post_bootloader_spi_transfer(void)
{
    /*
     * De-Assert CS pin for EXT SPI Flash
     */
    spiflash_cs_deassert();
    xSemaphoreGive(spi_sem_sync_hdl);
}

/****************************************************************************
 * @fn  void pre_lcd_spi_transfer()
 * @brief
 *     Take a semaphore and setting LCD baudrate
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void pre_lcd_spi_transfer(void)
{
    if (xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    if (pr_type != LCD)
    {
        pr_type = LCD;
        set_spi_baudrate(pr_type);
    }
    /*LCD CS is handled as part of LCD gsdk*/
}

/****************************************************************************
 * @fn  void post_lcd_spi_transfer()
 * @brief
 *     Release semaphore
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void post_lcd_spi_transfer(void)
{
    xSemaphoreGive(spi_sem_sync_hdl);
}
