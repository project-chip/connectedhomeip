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

StaticSemaphore_t xEfxSpiIntfSemaBuffer;
static SemaphoreHandle_t spi_sem;

#if defined(EFR32MG12)
#include "sl_spidrv_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
#endif

#if defined(EFR32MG24)
#include "sl_spidrv_eusart_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_eusart_exp_handle;
#endif

static unsigned int tx_dma_channel;
static unsigned int rx_dma_channel;

static uint32_t dummy_data; /* Used for DMA - when results don't matter */
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
    spi_sem = xSemaphoreCreateBinaryStatic(&xEfxSpiIntfSemaBuffer);
    xSemaphoreGive(spi_sem);

    /* Assign DMA channel from Handle*/
#if defined(EFR32MG12)
    /* MG12 + rs9116 combination uses USART driver */
    tx_dma_channel = sl_spidrv_exp_handle->txDMACh;
    rx_dma_channel = sl_spidrv_exp_handle->rxDMACh;

#elif defined(EFR32MG24)
    /* MG24 + rs9116 combination uses EUSART driver */
    tx_dma_channel = sl_spidrv_eusart_exp_handle->txDMACh;
    rx_dma_channel = sl_spidrv_eusart_exp_handle->rxDMACh;
#endif

    /* GPIO INIT of MG12 & MG24 : Reset, Wakeup, Interrupt */
    WFX_RSI_LOG("RSI_HAL: init GPIO");
    sl_wfx_host_gpio_init();

    /* Reset of Wifi chip */
    WFX_RSI_LOG("RSI_HAL: Reset Wifi");
    sl_wfx_host_reset_chip();
    WFX_RSI_LOG("RSI_HAL: Init done");
}

/*****************************************************************************
 *@fn static bool rx_dma_complete(unsigned int channel, unsigned int sequenceNo, void *userParam)
 *
 *@brief
 *    complete dma
 *
 * @param[in] channel:
 * @param[in] sequenceNO: sequence number
 * @param[in] userParam :user parameter
 *
 * @return
 *    None
 ******************************************************************************/
static bool rx_dma_complete(unsigned int channel, unsigned int sequenceNo, void * userParam)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // uint8_t *buf = (void *)userParam;

    (void) channel;
    (void) sequenceNo;
    (void) userParam;

    // WFX_RSI_LOG ("SPI: DMA done [%x,%x,%x,%x]", buf [0], buf [1], buf [2], buf [3]);
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return true;
}

/*************************************************************
 * @fn   static void receiveDMA(uint8_t *rx_buf, uint16_t xlen)
 * @brief
 *       RX buf was specified
 *       TX buf was not specified by caller - so we
 *       transmit dummy data (typically 0)
 * @param[in] rx_buf:
 * @param[in] xlen:
 * @return
 *        None
 *******************************************************************/
static void receiveDMA(uint8_t * rx_buf, uint16_t xlen)
{
    /*
     * The caller wants to receive data -
     * The xmit can be dummy data (no src increment for tx)
     */
    dummy_data = 0;
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, (void *) rx_buf, (void *) &(MY_USART->RXDATA), true, xlen,
                            dmadrvDataSize1, rx_dma_complete, NULL);

    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) &(dummy_data), false, xlen,
                            dmadrvDataSize1, NULL, NULL);
}

/*****************************************************************************
 *@fn static void transmitDMA(void *rx_buf, void *tx_buf, uint8_t xlen)
 *@brief
 *    we have a tx_buf. There are some instances where
 *    a rx_buf is not specifed. If one is specified then
 *    the caller wants results (auto increment src)
 * @param[in] rx_buf:
 * @param[in] tx_buf:
 * @param[in] xlen:
 * @return
 *     None
 ******************************************************************************/
static void transmitDMA(uint8_t * rx_buf, uint8_t * tx_buf, uint16_t xlen)
{
    void * buf;
    bool srcinc;
    /*
     * we have a tx_buf. There are some instances where
     * a rx_buf is not specifed. If one is specified then
     * the caller wants results (auto increment src)
     * TODO - the caller specified 8/32 bit - we should use this
     * instead of dmadrvDataSize1 always
     */
    if (rx_buf == NULL)
    {
        buf    = &dummy_data;
        srcinc = false;
    }
    else
    {
        buf                   = rx_buf;
        srcinc                = true;
        /* DEBUG */ rx_buf[0] = 0xAA;
        rx_buf[1]             = 0x55;
    }
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, buf, (void *) &(MY_USART->RXDATA), srcinc, xlen, dmadrvDataSize1,
                            rx_dma_complete, buf);
    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) tx_buf, true, xlen,
                            dmadrvDataSize1, NULL, NULL);
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
    // WFX_RSI_LOG ("SPI: Xfer: tx=%x,rx=%x,len=%d",(uint32_t)tx_buf, (uint32_t)rx_buf, xlen);
    if (xlen > MIN_XLEN)
    {
        MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
        if (xSemaphoreTake(spi_sem, portMAX_DELAY) != pdTRUE)
        {
            return RSI_FALSE;
        }
        if (tx_buf == NULL)
        {
            receiveDMA(rx_buf, xlen);
        }
        else
        {
            transmitDMA(rx_buf, tx_buf, xlen);
        }

        /*
         * receiveDMA() and transmitDMA() are asynchronous
         * Our application design assumes that this function is synchronous
         * To make it synchronous, we wait to re-acquire the semaphore before exiting this function
         * rx_dma_complete() gives back the semaphore when the SPI transfer is done
         */
        if (xSemaphoreTake(spi_sem, pdMS_TO_TICKS(RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS)) == pdTRUE)
        {
            // Transfer complete
            // Give back the semaphore before exiting, so that it may be re-acquired
            // in this function, just before the next transfer
            xSemaphoreGive(spi_sem);
        }
        // Temporary patch
        // Sometimes the xSemaphoreTake() above is getting stuck indefinitely
        // As a workaround, if the transfer is not done within RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS
        // stop and start it again
        // No need to re-acquire the semaphore since this is the function that acquired it
        // TODO: Remove this after a permanent solution is found to the problem of the transfer getting stuck
        else
        {
            uint32_t ldma_flags = 0;
            uint32_t rem_len    = 0;
            rem_len             = LDMA_TransferRemainingCount(RSI_LDMA_TRANSFER_CHANNEL_NUM);
            LDMA_StopTransfer(RSI_LDMA_TRANSFER_CHANNEL_NUM);
            ldma_flags = LDMA_IntGet();
            LDMA_IntClear(ldma_flags);
            receiveDMA(rx_buf, rem_len);
            if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE)
            {
                xSemaphoreGive(spi_sem);
            }
        }
    }

    return RSI_ERROR_NONE;
}
