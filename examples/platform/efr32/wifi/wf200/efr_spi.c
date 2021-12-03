
/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/***************************************************************************
 * @file
 * @brief SL_WFX SPI interface driver implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in
 *compliance with the License. You may obtain a copy
 *of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in
 *writing, software distributed under the License is
 *distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *OR CONDITIONS OF ANY KIND, either express or
 *implied. See the License for the specific language
 *governing permissions and limitations under the
 *License.
 *****************************************************************************/
#include "sl_wfx_configuration_defaults.h"

#ifdef SL_WFX_USE_SPI

#include "sl_wfx.h"
#include "sl_wfx_host_api.h"
#include "sl_wfx_board.h"

#include "dmadrv.h"
#include "em_cmu.h"
#include "em_usart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#ifdef SLEEP_ENABLED
#include "sl_power_manager.h"
#endif
#include "AppConfig.h"

#define USART SL_WFX_HOST_PINOUT_SPI_PERIPHERAL

static SemaphoreHandle_t spi_sem;
static unsigned int tx_dma_channel;
static unsigned int rx_dma_channel;
static uint32_t dummy_rx_data;
static uint32_t dummy_tx_data;
static bool spi_enabled = false;

uint8_t wirq_irq_nb = SL_WFX_HOST_PINOUT_SPI_IRQ;//SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN;


/****************************************************************************
 * Initialize SPI peripheral
 *****************************************************************************/
sl_status_t
sl_wfx_host_init_bus(void)
{
    // Initialize and enable the USART
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

    EFR32_LOG ("WIFI: Spi Init");
#if 0
    res = sl_wfx_host_spi_set_config(USART);
    if (res != 0)
    {
        return SL_STATUS_FAIL;
    }
#endif
    spi_enabled        = true;
    dummy_tx_data      = 0;
    usartInit.baudrate = 36000000u;
    usartInit.msbf     = true;
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(MY_USART_CLOCK, true);
    USART_InitSync(MY_USART, &usartInit);
    MY_USART->CTRL |= (1u << _USART_CTRL_SMSDELAY_SHIFT);
    MY_USART->ROUTELOC0 =
        (MY_USART->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK)) |
        (SL_WFX_HOST_PINOUT_SPI_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);

    MY_USART->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN;
    GPIO_DriveStrengthSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, gpioDriveStrengthStrongAlternateStrong);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_TX_PORT, SL_WFX_HOST_PINOUT_SPI_TX_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_RX_PORT, SL_WFX_HOST_PINOUT_SPI_RX_PIN, gpioModeInput, 0);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, SL_WFX_HOST_PINOUT_SPI_CLK_PIN, gpioModePushPull, 0);

    spi_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(spi_sem);

    DMADRV_Init();
    DMADRV_AllocateChannel(&tx_dma_channel, NULL);
    DMADRV_AllocateChannel(&rx_dma_channel, NULL);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN, gpioModePushPull, 1);
    MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    return SL_STATUS_OK;
}

/****************************************************************************
 * De-initialize SPI peripheral and DMAs
 *****************************************************************************/
sl_status_t
sl_wfx_host_deinit_bus(void)
{
    vSemaphoreDelete(spi_sem);
    // Stop DMAs.
    DMADRV_StopTransfer(rx_dma_channel);
    DMADRV_StopTransfer(tx_dma_channel);
    DMADRV_FreeChannel(tx_dma_channel);
    DMADRV_FreeChannel(rx_dma_channel);
    DMADRV_DeInit();
    USART_Reset(MY_USART);
    return SL_STATUS_OK;
}

/****************************************************************************
 * Assert chip select.
 *****************************************************************************/
sl_status_t
sl_wfx_host_spi_cs_assert()
{
    GPIO_PinOutClear(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN);
    return SL_STATUS_OK;
}

/****************************************************************************
 * De-assert chip select.
 *****************************************************************************/
sl_status_t
sl_wfx_host_spi_cs_deassert()
{
    GPIO_PinOutSet(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN);
    return SL_STATUS_OK;
}

static bool
rx_dma_complete(unsigned int channel, unsigned int sequenceNo, void * userParam)
{
    (void) channel;
    (void) sequenceNo;
    (void) userParam;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return true;
}

void
receiveDMA (uint8_t * buffer, uint16_t buffer_length)
{
    // Start receive DMA.
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, (void *) buffer, (void *) &(MY_USART->RXDATA), true, buffer_length,
                            dmadrvDataSize1, rx_dma_complete, NULL);

    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) &(dummy_tx_data), false,
                            buffer_length, dmadrvDataSize1, NULL, NULL);
}

void
transmitDMA(uint8_t * buffer, uint16_t buffer_length)
{
    // Receive DMA runs only to initiate callback
    // Start receive DMA.
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, &dummy_rx_data, (void *) &(MY_USART->RXDATA), false, buffer_length,
                            dmadrvDataSize1, rx_dma_complete, NULL);
    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) buffer, true, buffer_length,
                            dmadrvDataSize1, NULL, NULL);
}

/****************************************************************************
 * WFX SPI transfer implementation
 *****************************************************************************/
sl_status_t
sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type, uint8_t * header, uint16_t header_length,
                                                  uint8_t * buffer, uint16_t buffer_length)
{
    sl_status_t result = SL_STATUS_FAIL;
    const bool is_read = (type == SL_WFX_BUS_READ);

    while (!(MY_USART->STATUS & USART_STATUS_TXBL))
    {
    }
    MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    if (header_length > 0)
    {
        for (uint8_t * buffer_ptr = header; header_length > 0; --header_length, ++buffer_ptr)
        {
            MY_USART->TXDATA = (uint32_t)(*buffer_ptr);

            while (!(MY_USART->STATUS & USART_STATUS_TXC))
            {
            }
        }
        while (!(MY_USART->STATUS & USART_STATUS_TXBL))
        {
        }
    }
    if (buffer_length > 0)
    {
        MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
        if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE)
        {
            if (is_read)
            {
                receiveDMA(buffer, buffer_length);
                result = SL_STATUS_OK;
            }
            else
            {
                transmitDMA(buffer, buffer_length);
                result = SL_STATUS_OK;
            }

            if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE)
            {
                xSemaphoreGive(spi_sem);
            }
        }
        else
        {
            result = SL_STATUS_TIMEOUT;
        }
    }

    return result;
}

/****************************************************************************
 * Enable WFX interrupt
 *****************************************************************************/
void
sl_wfx_host_start_platform_interrupt(void)
{
    // Enable (and clear) the bus interrupt
    GPIO_ExtIntConfig(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, wirq_irq_nb, true, false, true);
}

/****************************************************************************
 * Disable WFX interrupt
 *****************************************************************************/
sl_status_t
sl_wfx_host_disable_platform_interrupt(void)
{
    GPIO_IntDisable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

sl_status_t
sl_wfx_host_enable_platform_interrupt(void)
{
    GPIO_IntEnable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

sl_status_t
sl_wfx_host_enable_spi(void)
{
    if (spi_enabled == false) {
#ifdef SLEEP_ENABLED
        // Prevent the host to use lower EM than EM1
        sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        spi_enabled = true;
    }
    return SL_STATUS_OK;
}

sl_status_t
sl_wfx_host_disable_spi(void)
{
    if (spi_enabled == true)
    {
        spi_enabled = false;
#ifdef SLEEP_ENABLED
        // Allow the host to use the lowest allowed EM
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
    }
    return SL_STATUS_OK;
}

#endif
