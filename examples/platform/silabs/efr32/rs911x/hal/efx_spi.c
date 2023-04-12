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

#include "gpiointerrupt.h"
#include "sl_device_init_clocks.h"
#include "sl_status.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"

#include "rsi_board_configuration.h"
#include "rsi_driver.h"
#include "sl_device_init_dpll.h"
#include "sl_device_init_hfxo.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

StaticSemaphore_t xEfxSpiIntfSemaBuffer;
static SemaphoreHandle_t spiTransferLock;
static TaskHandle_t spiInitiatorTaskHandle = NULL;

#if defined(EFR32MG12)
#include "sl_spidrv_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
#define SPI_HANDLE sl_spidrv_exp_handle
#elif defined(EFR32MG24)
#include "sl_spidrv_eusart_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_eusart_exp_handle;
#define SPI_HANDLE sl_spidrv_eusart_exp_handle
#else
#error "Unknown platform"
#endif

extern void rsi_gpio_irq_cb(uint8_t irqnum);
//#define RS911X_USE_LDMA

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

    /* GPIO INIT of MG12 & MG24 : Reset, Wakeup, Interrupt */
    WFX_RSI_LOG("RSI_HAL: init GPIO");
    sl_wfx_host_gpio_init();

    /* Reset of Wifi chip */
    WFX_RSI_LOG("RSI_HAL: Reset Wifi");
    sl_wfx_host_reset_chip();
    WFX_RSI_LOG("RSI_HAL: Init done");
}

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
    if (xlen <= MIN_XLEN || (tx_buf == NULL && rx_buf == NULL)) // at least one buffer needs to be provided
    {
        return RSI_ERROR_INVALID_PARAM;
    }

    (void) mode; // currently not used;
    rsi_error_t rsiError = RSI_ERROR_NONE;

    if (xSemaphoreTake(spiTransferLock, portMAX_DELAY) != pdTRUE)
    {
        return RSI_ERROR_SPI_BUSY;
    }

    configASSERT(spiInitiatorTaskHandle == NULL); // No other task should currently be waiting for the dma completion
    spiInitiatorTaskHandle = xTaskGetCurrentTaskHandle();

    Ecode_t spiError;
    if (tx_buf == NULL) // Rx operation only
    {
        spiError = SPIDRV_MReceive(SPI_HANDLE, rx_buf, xlen, spi_dmaTransfertComplete);
    }
    else if (rx_buf == NULL) // Tx operation only
    {
        spiError = SPIDRV_MTransmit(SPI_HANDLE, tx_buf, xlen, spi_dmaTransfertComplete);
    }
    else // Tx and Rx operation
    {
        spiError = SPIDRV_MTransfer(SPI_HANDLE, tx_buf, rx_buf, xlen, spi_dmaTransfertComplete);
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
            SPIDRV_GetTransferStatus(SPI_HANDLE, &itemsTransferred, &itemsRemaining);
            WFX_RSI_LOG("SPI transfert timed out %d/%d (rx%x rx%x)", itemsTransferred, itemsRemaining, (uint32_t) tx_buf,
                        (uint32_t) rx_buf);

            SPIDRV_AbortTransfer(SPI_HANDLE);
            rsiError = RSI_ERROR_SPI_TIMEOUT;
        }
    }
    else
    {
        WFX_RSI_LOG("SPI transfert failed with err:%x (tx%x rx%x)", spiError, (uint32_t) tx_buf, (uint32_t) rx_buf);
        rsiError               = RSI_ERROR_SPI_FAIL;
        spiInitiatorTaskHandle = NULL; // SPI operation failed. No notification to received.
    }

    xSemaphoreGive(spiTransferLock);
    return rsiError;
}
