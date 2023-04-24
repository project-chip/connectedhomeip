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

#if (defined(EFR32MG24) && defined(WF200_WIFI))
#include "sl_wfx.h"
#endif /* EFR32MG24 && WF200_WIFI */

#include "dmadrv.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#if SL_WIFI
#include "btl_interface.h"
#include "sl_memlcd.h"
#include "spi_multiplex.h"
#endif /* SL_WIFI */

#define CONCAT(A, B) (A##B)
#define SPI_CLOCK(N) CONCAT(cmuClock_USART, N)
#ifdef WF200_WIFI
#define SL_SPIDRV_FRAME_LENGTH SL_SPIDRV_EXP_FRAME_LENGTH
#else
#define SL_SPIDRV_FRAME_LENGTH 8 // default value
#endif

const uint32_t spiBitRates[] = { [EXP_HDR] = SL_BIT_RATE_EXP_HDR, [LCD] = SL_BIT_RATE_LCD, [EXT_SPIFLASH] = SL_BIT_RATE_SPI_FLASH };

extern void efr32Log(const char *, ...);

extern SPIDRV_Handle_t sl_spidrv_eusart_exp_handle;
extern SPIDRV_Init_t sl_spidrv_eusart_init_exp;

/****************************************************************************
 * @fn  void spi_drv_reinit()
 * @brief
 *     Re-Intializes SPI driver to required baudrate
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void spi_drv_reinit(uint32_t baudrate)
{
    if (USART_BaudrateGet(USART0) == baudrate)
    {
        // USART synced to baudrate already
        return;
    }
    // USART is used in MG24 + WF200 combination
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.msbf                   = true;
    usartInit.clockMode              = usartClockMode0;
#ifdef WF200_WIFI
    usartInit.baudrate = baudrate;
#elif RS911X_WIFI
    usartInit.baudrate = spiBitRates[pr_type];
#endif
    uint32_t databits      = SL_SPIDRV_FRAME_LENGTH - 4U + _USART_FRAME_DATABITS_FOUR;
    usartInit.databits     = (USART_Databits_TypeDef) databits;
    usartInit.autoCsEnable = true;

    USART_InitSync(USART0, &usartInit);
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
     * Assert CS pin for EXT SPI Flash
     */
#ifdef WF200_WIFI
    spi_drv_reinit(SL_BIT_RATE_SPI_FLASH);
#else
    spi_switch(EXT_SPIFLASH);
#endif
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
#ifdef WF200_WIFI
    spi_drv_reinit(SL_BIT_RATE_LCD);
#else
    spi_switch(LCD);
#endif
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

#ifdef RS911X_WIFI
/****************************************************************************
 * @fn  void spi_switch(pr_type)
 * @brief
 *     Handles switching between pheripherals spi
 * @param[in] prType - enum indicating type of pheripheral to switch the SPI to
 * @return returns void
 *****************************************************************************/
void spi_switch(peripheraltype_t prType)
{
    efr32Log("switching spi from %u to %u", pr_type, prType);
    if (pr_type != EXP_HDR && prType == EXP_HDR)
    {
        if (pr_type == LCD)
        {
            /* deinit USART of LCD*/
            USART_Enable(SL_MEMLCD_SPI_PERIPHERAL, usartDisable);
            CMU_ClockEnable(SPI_CLOCK(SL_MEMLCD_SPI_PERIPHERAL_NO), false);
            // USART_Reset(SL_MEMLCD_SPI_PERIPHERAL);
            GPIO->USARTROUTE[SL_MEMLCD_SPI_PERIPHERAL_NO].ROUTEEN = 0;
        }
        else if (pr_type == EXT_SPIFLASH)
        {
            bootloader_deinit();
            GPIO->USARTROUTE[0].ROUTEEN = 0;
        }
        pr_type = EXP_HDR;
        /* init EUSART of RS911x*/
        SPIDRV_Init(sl_spidrv_eusart_exp_handle, &sl_spidrv_eusart_init_exp);
    }
    else if (pr_type != LCD && prType == LCD)
    {
        if (pr_type == EXT_SPIFLASH)
        {
            spi_drv_reinit(SL_BIT_RATE_LCD);
            pr_type = prType;
            efr32Log("spi switched to %u", pr_type);
            return;
        }
        if (pr_type == EXP_HDR)
        {
            efr32Log("deinited eusart of exp hdr");
            SPIDRV_DeInit(sl_spidrv_eusart_exp_handle);
            GPIO->EUSARTROUTE[SL_SPIDRV_EUSART_EXP_PERIPHERAL_NO].ROUTEEN = 0;
        }
        pr_type = LCD;
        efr32Log("inited usart for lcd");
        sl_memlcd_refresh(sl_memlcd_get());
    }
    else if (pr_type != EXT_SPIFLASH && prType == EXT_SPIFLASH)
    {
        if (pr_type == LCD)
        {
            spi_drv_reinit(SL_BIT_RATE_SPI_FLASH);
            pr_type = prType;
            efr32Log("spi switched to %u", pr_type);
            return;
        }
        else if (pr_type == EXP_HDR)
        {
            SPIDRV_DeInit(sl_spidrv_eusart_exp_handle);
            GPIO->EUSARTROUTE[SL_SPIDRV_EUSART_EXP_PERIPHERAL_NO].ROUTEEN = 0;
        }
        pr_type = EXT_SPIFLASH;
        /* init EUSART of RS911x*/
        bootloader_init();
    }
    efr32Log("spi switched to %u", pr_type);
}
#endif

#if (defined(EFR32MG24) && defined(WF200_WIFI))
/****************************************************************************
 * @fn  void pre_uart_transfer()
 * @brief
 *     Take a semaphore and setting GPIO, disable IRQ
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void pre_uart_transfer(void)
{
    if (spi_sem_sync_hdl == NULL)
    {
        // UART is initialized before host SPI interface
        // spi_sem_sync_hdl will not be initalized during execution
        GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1);
        return;
    }
    sl_wfx_disable_irq();
    sl_wfx_host_disable_platform_interrupt();
    if (xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1);
}
/****************************************************************************
 * @fn  void post_uart_transfer()
 * @brief
 *     Reset GPIO, enabled IRQ, release semaphore
 * @param[in] None
 * @return returns void
 *****************************************************************************/
void post_uart_transfer(void)
{
    if (spi_sem_sync_hdl == NULL)
    {
        return;
    }
    GPIO_PinModeSet(gpioPortA, 8, gpioModeInputPull, 1);
    xSemaphoreGive(spi_sem_sync_hdl);
    sl_wfx_host_enable_platform_interrupt();
    sl_wfx_enable_irq();
}
#endif /* EFR32MG24 && WF200_WIFI */