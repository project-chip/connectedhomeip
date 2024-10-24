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

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"

#include "dmadrv.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_device.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "gpiointerrupt.h"
#include "sl_device_init_clocks.h"
#include "sl_device_init_hfxo.h"
#include "sl_spidrv_instances.h"
#include "sl_status.h"
#include "spidrv.h"

#include "silabs_utils.h"
#include "spi_multiplex.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"

#ifdef SL_BOARD_NAME
#include "sl_board_control.h"
#endif // SL_BOARD_NAME

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#include "rsi_board_configuration.h"
#include "rsi_driver.h"

#if SL_BTLCTRL_MUX
#include "btl_interface.h"
#endif // SL_BTLCTRL_MUX
#if SL_LCDCTRL_MUX
#include "sl_memlcd.h"
#include "sl_memlcd_display.h"
#endif // SL_LCDCTRL_MUX
#if SL_MX25CTRL_MUX
#include "sl_mx25_flash_shutdown_usart_config.h"
#endif // SL_MX25CTRL_MUX

#include "em_eusart.h"
#include "sl_spidrv_eusart_exp_config.h"
#include "spi_multiplex.h"

#define SL_SPIDRV_HANDLE sl_spidrv_eusart_exp_handle
#define SL_SPIDRV_EXP_BITRATE_MULTIPLEXED SL_SPIDRV_EUSART_EXP_BITRATE

#define CONCAT(A, B) (A##B)
#define SPI_CLOCK(N) CONCAT(cmuClock_USART, N)
// Macro to drive semaphore block minimun timer in milli seconds
#define RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS (50)

#if SL_SPICTRL_MUX
StaticSemaphore_t spi_sem_peripheral;
static SemaphoreHandle_t spi_sem_sync_hdl;
#endif // SL_SPICTRL_MUX

StaticSemaphore_t xEfxSpiIntfSemaBuffer;
static SemaphoreHandle_t spiTransferLock;
static TaskHandle_t spiInitiatorTaskHandle = NULL;

static uint32_t dummy_buffer; /* Used for DMA - when results don't matter */

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

    // Set CS pin to high/inactive
    GPIO_PinModeSet(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN, gpioModePushPull, PINOUT_SET);

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

#if SL_SPICTRL_MUX
    if (spi_sem_sync_hdl == NULL)
    {
        spi_sem_sync_hdl = xSemaphoreCreateBinaryStatic(&spi_sem_peripheral);
    }
    configASSERT(spi_sem_sync_hdl);
    xSemaphoreGive(spi_sem_sync_hdl);
#endif /* SL_SPICTRL_MUX */

    /* GPIO INIT of MG24 : Reset, Wakeup, Interrupt */
    sl_wfx_host_gpio_init();

    /* Reset of Wifi chip */
    sl_wfx_host_reset_chip();
}

#if SL_SPICTRL_MUX
sl_status_t sl_wfx_host_spi_cs_assert(void)
{
#if SL_SPICTRL_MUX
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
#endif                // SL_SPICTRL_MUX
    if (!spi_enabled) // Reduce sl_spidrv_init_instances
    {
        sl_spidrv_init_instances();
        GPIO_PinOutClear(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN);
        spi_enabled = true;
    }
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_deassert(void)
{
    sl_status_t status = SL_STATUS_OK;
    if (spi_enabled)
    {
        status = SPIDRV_DeInit(SL_SPIDRV_HANDLE);
        if (SL_STATUS_OK == status)
        {
            GPIO_PinOutSet(SL_SPIDRV_EUSART_EXP_CS_PORT, SL_SPIDRV_EUSART_EXP_CS_PIN);
            GPIO->EUSARTROUTE[SL_SPIDRV_EUSART_EXP_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
            spi_enabled                                                   = false;
        }
    }
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    return status;
}
#endif // SL_SPICTRL_MUX

#if SL_MX25CTRL_MUX
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
#endif // SL_MX25CTRL_MUX

#if SL_BTLCTRL_MUX
sl_status_t sl_wfx_host_pre_bootloader_spi_transfer(void)
{
#if SL_SPICTRL_MUX
    if (sl_wfx_host_spi_cs_deassert() != SL_STATUS_OK)
    {
        return SL_STATUS_FAIL;
    }
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
#endif // SL_SPICTRL_MUX
    // bootloader_init takes care of SPIDRV_Init()
    int32_t status = bootloader_init();
    if (status != BOOTLOADER_OK)
    {
        SILABS_LOG("bootloader_init error: %x", status);
#if SL_SPICTRL_MUX
        xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
        return SL_STATUS_FAIL;
    }
#if SL_MX25CTRL_MUX
    sl_wfx_host_spiflash_cs_assert();
#endif // SL_MX25CTRL_MUX
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_bootloader_spi_transfer(void)
{
    // bootloader_deinit will do USART disable
    int32_t status = bootloader_deinit();
    if (status != BOOTLOADER_OK)
    {
        SILABS_LOG("bootloader_deinit error: %x", status);
#if SL_SPICTRL_MUX
        xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
        return SL_STATUS_FAIL;
    }
    GPIO->USARTROUTE[SL_MX25_FLASH_SHUTDOWN_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
#if SL_MX25CTRL_MUX
    sl_wfx_host_spiflash_cs_deassert();
#endif // SL_MX25CTRL_MUX
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    return SL_STATUS_OK;
}
#endif // SL_BTLCTRL_MUX

#if SL_LCDCTRL_MUX
sl_status_t sl_wfx_host_pre_lcd_spi_transfer(void)
{
#if SL_SPICTRL_MUX
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
#endif // SL_SPICTRL_MUX
    sl_status_t status = sl_board_enable_display();
    if (SL_STATUS_OK == status)
    {
        // sl_memlcd_refresh takes care of SPIDRV_Init()
        status = sl_memlcd_refresh(sl_memlcd_get());
    }
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    return status;
}

sl_status_t sl_wfx_host_post_lcd_spi_transfer(void)
{
    USART_Enable(SL_MEMLCD_SPI_PERIPHERAL, usartDisable);
    CMU_ClockEnable(SPI_CLOCK(SL_MEMLCD_SPI_PERIPHERAL_NO), false);
    GPIO->USARTROUTE[SL_MEMLCD_SPI_PERIPHERAL_NO].ROUTEEN = PINOUT_CLEAR;
    sl_status_t status                                    = sl_board_disable_display();
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    return status;
}
#endif // SL_LCDCTRL_MUX

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
 * @fn         int16_t rsi_spi_transfer(uint8_t *ptrBuf, uint16_t bufLen, uint8_t *valBuf, uint8_t mode)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transfered
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description
 * This API is used to transfer/receive data to the Wi-Fi module through the SPI interface.
 **************************************************************************/
int16_t rsi_spi_transfer(uint8_t * tx_buf, uint8_t * rx_buf, uint16_t xlen, uint8_t mode)
{
#if SL_SPICTRL_MUX
    sl_wfx_host_spi_cs_assert();
#endif // SL_SPICTRL_MUX
    /*
        TODO: tx_buf and rx_buf needs to be replaced with a dummy buffer of length xlen to align with SDK of WiFi
    */
    if (xlen <= MIN_XLEN || (tx_buf == NULL && rx_buf == NULL))
    {
        if (xlen > sizeof(dummy_buffer))
        {
            return RSI_ERROR_INVALID_PARAM; // Ensuring that the dummy buffer won't corrupt the memory
        }
        rx_buf = (uint8_t *) &dummy_buffer;
        tx_buf = (uint8_t *) &dummy_buffer;
    }

    (void) mode; // currently not used;
    int16_t rsiError = RSI_ERROR_NONE;

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
#if SL_SPICTRL_MUX
    sl_wfx_host_spi_cs_deassert();
#endif // SL_SPICTRL_MUX
    return rsiError;
}
