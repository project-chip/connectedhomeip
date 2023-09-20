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

/**
 * Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmadrv.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_device.h"
#include "em_gpio.h"
#include "em_ldma.h"
#if defined(EFR32MG12)
#include "em_usart.h"
#elif defined(EFR32MG24)
#include "em_eusart.h"
#endif
#include "spidrv.h"

#include "silabs_utils.h"

#include "gpiointerrupt.h"
#include "sl_device_init_clocks.h"
#include "sl_memlcd.h"
#include "sl_status.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "btl_interface.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"

#ifdef CHIP_9117
#include "cmsis_os2.h"
#include "sl_board_configuration.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_si91x_types.h"
#include "sl_wifi_callback_framework.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#else
#include "rsi_board_configuration.h"
#include "rsi_driver.h"
#endif

#include "sl_device_init_dpll.h"
#include "sl_device_init_hfxo.h"

#define DEFAULT_SPI_TRASFER_MODE 0
// Macro to drive semaphore block minimun timer in milli seconds
#define RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS (50)
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#define CONCAT(A, B) (A##B)
#define SPI_CLOCK(N) CONCAT(cmuClock_USART, N)

#if defined(EFR32MG24)
StaticSemaphore_t spi_sem_peripheral;
SemaphoreHandle_t spi_sem_sync_hdl;
#endif /* EFR32MG24 */

StaticSemaphore_t xEfxSpiIntfSemaBuffer;
static SemaphoreHandle_t spiTransferLock;
static TaskHandle_t spiInitiatorTaskHandle = NULL;

static uint32_t dummy_buffer; /* Used for DMA - when results don't matter */

#if defined(EFR32MG12)
#include "sl_spidrv_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
#define SL_SPIDRV_HANDLE sl_spidrv_exp_handle
#elif defined(EFR32MG24)
#include "spi_multiplex.h"
#else
#error "Unknown platform"
#endif

// variable to identify spi configured for expansion header
// EUSART configuration available on the SPIDRV
static bool spi_enabled = false;

extern void rsi_gpio_irq_cb(uint8_t irqnum);

/********************************************************
 * @fn   sl_wfx_host_gpio_init(void)
 * @brief
 *        Deal with the PINS that are not associated with SPI -
 *        Ie. RESET, Wakeup
 * @return
 *        None
 **********************************************************/
void sl_wfx_host_gpio_init(void)
{
    // Enable GPIO clock.
    CMU_ClockEnable(cmuClock_GPIO, true);

#if defined(EFR32MG24)
    // Set CS pin to high/inactive
    GPIO_PinModeSet(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN, gpioModePushPull, PINOUT_SET);
#endif

    GPIO_PinModeSet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin, gpioModePushPull, PINOUT_SET);
    GPIO_PinModeSet(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin, gpioModePushPull, PINOUT_CLEAR);

    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    // Set up interrupt based callback function - trigger on both edges.
    GPIOINT_Init();
    GPIO_PinModeSet(WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin, gpioModeInputPull, PINOUT_CLEAR);
    GPIO_ExtIntConfig(WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin, SL_WFX_HOST_PINOUT_SPI_IRQ, true, false, true);
    GPIOINT_CallbackRegister(SL_WFX_HOST_PINOUT_SPI_IRQ, rsi_gpio_irq_cb);
    GPIO_IntDisable(1 << SL_WFX_HOST_PINOUT_SPI_IRQ); /* Will be enabled by RSI */

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_SetPriority(GPIO_EVEN_IRQn, WFX_SPI_NVIC_PRIORITY);
    NVIC_SetPriority(GPIO_ODD_IRQn, WFX_SPI_NVIC_PRIORITY);
    spi_enabled = true;
}

/*****************************************************************
 * @fn  void sl_wfx_host_reset_chip(void)
 * @brief
 *      To reset the WiFi CHIP
 * @return
 *      None
 ****************************************************************/
void sl_wfx_host_reset_chip(void)
{
    // Pull it low for at least 1 ms to issue a reset sequence
    GPIO_PinOutClear(WFX_RESET_PIN.port, WFX_RESET_PIN.pin);

    // Delay for 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    // Hold pin high to get chip out of reset
    GPIO_PinOutSet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin);

    // Delay for 3ms
    vTaskDelay(pdMS_TO_TICKS(3));
}

/*****************************************************************
 * @fn   void rsi_hal_board_init(void)
 * @brief
 *       Initialize the board
 * @return
 *       None
 ****************************************************************/
void rsi_hal_board_init(void)
{
    spiTransferLock = xSemaphoreCreateBinaryStatic(&xEfxSpiIntfSemaBuffer);
    xSemaphoreGive(spiTransferLock);

#if defined(EFR32MG24)
    if (spi_sem_sync_hdl == NULL)
    {
        spi_sem_sync_hdl = xSemaphoreCreateBinaryStatic(&spi_sem_peripheral);
    }
    configASSERT(spi_sem_sync_hdl);
    xSemaphoreGive(spi_sem_sync_hdl);
#endif /* EFR32MG24 */

    /* GPIO INIT of MG12 & MG24 : Reset, Wakeup, Interrupt */
    sl_wfx_host_gpio_init();

    /* Reset of Wifi chip */
    sl_wfx_host_reset_chip();
}

// wifi-sdk
sl_status_t sl_si91x_host_bus_init(void)
{
    rsi_hal_board_init();
    return SL_STATUS_OK;
}

void sl_si91x_host_enable_high_speed_bus()
{
    // dummy function for wifi-sdk
}

#if defined(EFR32MG24)

void SPIDRV_SetBaudrate(uint32_t baudrate)
{
    if (EUSART_BaudrateGet(MY_USART) == baudrate)
    {
        // EUSART synced to baudrate already
        return;
    }
    EUSART_BaudrateSet(MY_USART, 0, baudrate);
}

sl_status_t sl_wfx_host_spi_cs_assert(void)
{
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);

    if (!spi_enabled) // Reduce sl_spidrv_init_instances
    {
        sl_spidrv_init_instances();
        spi_enabled = true;
    }
    GPIO_PinOutClear(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_deassert(void)
{
    if (spi_enabled)
    {
        if (ECODE_EMDRV_SPIDRV_OK != SPIDRV_DeInit(SL_SPIDRV_HANDLE))
        {
            xSemaphoreGive(spi_sem_sync_hdl);
            return SL_STATUS_FAIL;
        }
        spi_enabled = false;
    }
    GPIO_PinOutSet(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN);
    GPIO->EUSARTROUTE[SL_SPIDRV_EUSART_EXP_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
    xSemaphoreGive(spi_sem_sync_hdl);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spiflash_cs_assert(void)
{
    GPIO_PinOutClear(SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spiflash_cs_deassert(void)
{
    GPIO_PinOutSet(SL_MX25_FLASH_SHUTDOWN_CS_PORT, SL_MX25_FLASH_SHUTDOWN_CS_PIN);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_pre_bootloader_spi_transfer(void)
{
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
    if (spi_enabled)
    {
        if (ECODE_EMDRV_SPIDRV_OK != SPIDRV_DeInit(SL_SPIDRV_HANDLE))
        {
            xSemaphoreGive(spi_sem_sync_hdl);
            return SL_STATUS_FAIL;
        }
        spi_enabled = false;
    }
    // bootloader_init takes care of SPIDRV_Init()
    int32_t status = bootloader_init();
    if (status != BOOTLOADER_OK)
    {
        SILABS_LOG("bootloader_init error: %x", status);
        xSemaphoreGive(spi_sem_sync_hdl);
        return SL_STATUS_FAIL;
    }
    sl_wfx_host_spiflash_cs_assert();
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_bootloader_spi_transfer(void)
{
    // bootloader_deinit will do USART disable
    int32_t status = bootloader_deinit();
    if (status != BOOTLOADER_OK)
    {
        SILABS_LOG("bootloader_deinit error: %x", status);
        xSemaphoreGive(spi_sem_sync_hdl);
        return SL_STATUS_FAIL;
    }
    GPIO->USARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
    sl_wfx_host_spiflash_cs_deassert();
    xSemaphoreGive(spi_sem_sync_hdl);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_pre_lcd_spi_transfer(void)
{
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
    if (spi_enabled)
    {
        if (ECODE_EMDRV_SPIDRV_OK != SPIDRV_DeInit(SL_SPIDRV_HANDLE))
        {
            xSemaphoreGive(spi_sem_sync_hdl);
            return SL_STATUS_FAIL;
        }
        spi_enabled = false;
    }
    // sl_memlcd_refresh takes care of SPIDRV_Init()
    if (SL_STATUS_OK != sl_memlcd_refresh(sl_memlcd_get()))
    {
        xSemaphoreGive(spi_sem_sync_hdl);
        return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_lcd_spi_transfer(void)
{
    USART_Enable(SL_MEMLCD_SPI_PERIPHERAL, usartDisable);
    CMU_ClockEnable(SPI_CLOCK(SL_MEMLCD_SPI_PERIPHERAL_NO), false);
    GPIO->USARTROUTE[SL_MEMLCD_SPI_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
    xSemaphoreGive(spi_sem_sync_hdl);
    return SL_STATUS_OK;
}

#endif /* EFR32MG24 */

/*****************************************************************************
 *@brief
 *    Spi dma transfer is complete Callback
 *    Notify the task that initiated the SPI transfer that it is completed.
 *    The callback needs is a SPIDRV_Callback_t function pointer type
 * @param[in] pxHandle: spidrv instance handle
 * @param[in] transferStatus: Error code linked to the completed spi transfer. As master, the return code is irrelevant
 * @param[in] lCount: number of bytes transferred.
 *
 * @return
 *    None
 ******************************************************************************/
static void spi_dmaTransfertComplete(SPIDRV_HandleData_t * pxHandle, Ecode_t transferStatus, int itemsTransferred)
{
    configASSERT(spiInitiatorTaskHandle != NULL);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(spiInitiatorTaskHandle, &xHigherPriorityTaskWoken);
    spiInitiatorTaskHandle = NULL;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*********************************************************************
 * @fn   int16_t rsi_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t xlen, uint8_t mode)
 * @brief
 *       Do a SPI transfer - Mode is 8/16 bit - But every 8 bit is aligned
 * @param[in] tx_buf:
 * @param[in] rx_buf:
 * @param[in] xlen:
 * @param[in] mode:
 * @return
 *        None
 **************************************************************************/
int16_t rsi_spi_transfer(uint8_t * tx_buf, uint8_t * rx_buf, uint16_t xlen, uint8_t mode)
{
#if defined(EFR32MG24)
    sl_wfx_host_spi_cs_assert();
#endif /* EFR32MG24 */
    /*
        TODO: tx_buf and rx_buf needs to be replaced with a dummy buffer of length xlen to align with SDK of WiFi
    */
    if (xlen <= MIN_XLEN || (tx_buf == NULL && rx_buf == NULL))
    {
        rx_buf = (uint8_t *) &dummy_buffer;
        tx_buf = (uint8_t *) &dummy_buffer;
    }

    (void) mode; // currently not used;
    error_t rsiError = RSI_ERROR_NONE;

    xSemaphoreTake(spiTransferLock, portMAX_DELAY);

    // No other task should currently be waiting for the dma completion
    configASSERT(spiInitiatorTaskHandle == NULL);
    spiInitiatorTaskHandle = xTaskGetCurrentTaskHandle();

    Ecode_t spiError;
    if (tx_buf == NULL) // Rx operation only
    {
        spiError = SPIDRV_MReceive(SL_SPIDRV_HANDLE, rx_buf, xlen, spi_dmaTransfertComplete);
    }
    else if (rx_buf == NULL) // Tx operation only
    {
        spiError = SPIDRV_MTransmit(SL_SPIDRV_HANDLE, tx_buf, xlen, spi_dmaTransfertComplete);
    }
    else // Tx and Rx operation
    {
        spiError = SPIDRV_MTransfer(SL_SPIDRV_HANDLE, tx_buf, rx_buf, xlen, spi_dmaTransfertComplete);
    }

    if (spiError == ECODE_EMDRV_SPIDRV_OK)
    {
        // rsi implementation expect a synchronous operation
        // wait for the notification that the dma completed in a block state.
        // it does not consume any CPU time.
        if (ulTaskNotifyTake(pdTRUE, RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS) != pdPASS)
        {
            int itemsTransferred = 0;
            int itemsRemaining   = 0;
            SPIDRV_GetTransferStatus(SL_SPIDRV_HANDLE, &itemsTransferred, &itemsRemaining);
            SILABS_LOG("ERR: SPI timed out %d/%d (rx%x rx%x)", itemsTransferred, itemsRemaining, (uint32_t) tx_buf,
                       (uint32_t) rx_buf);

            SPIDRV_AbortTransfer(SL_SPIDRV_HANDLE);
            rsiError = RSI_ERROR_SPI_TIMEOUT;
        }
    }
    else
    {
        SILABS_LOG("ERR: SPI failed with error:%x (tx%x rx%x)", spiError, (uint32_t) tx_buf, (uint32_t) rx_buf);
        rsiError               = RSI_ERROR_SPI_FAIL;
        spiInitiatorTaskHandle = NULL; // SPI operation failed. No notification to received.
    }

    xSemaphoreGive(spiTransferLock);
#if defined(EFR32MG24)
    sl_wfx_host_spi_cs_deassert();
#endif /* EFR32MG24 */
    return rsiError;
}

/*********************************************************************
 * @fn   int16_t rsi_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t xlen, uint8_t mode)
 * @brief
 *       Do a SPI transfer - Mode is 8/16 bit - But every 8 bit is aligned
 * @param[in] tx_buf:
 * @param[in] rx_buf:
 * @param[in] xlen:
 * @param[in] mode:
 * @return
 *        None
 **************************************************************************/
sl_status_t sl_si91x_host_spi_transfer(const void * tx_buf, void * rx_buf, uint16_t xlen)
{
    return (rsi_spi_transfer((uint8_t *) tx_buf, rx_buf, xlen, DEFAULT_SPI_TRASFER_MODE));
}
