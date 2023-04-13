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

#include "sl_wfx_configuration_defaults.h"

#include "sl_wfx.h"
#include "sl_wfx_board.h"
#include "sl_wfx_host_api.h"

#include "dmadrv.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "spidrv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "gpiointerrupt.h"

#include "sl_spidrv_exp_config.h"
#include "sl_wfx_board.h"
#include "sl_wfx_host.h"
#include "sl_wfx_task.h"
#include "wfx_host_events.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if defined(EFR32MG24)
#include "spi_multiplex.h"
StaticSemaphore_t spi_sem_peripharal;
SemaphoreHandle_t spi_sem_sync_hdl;
peripheraltype_t pr_type = EXP_HDR;
#endif
extern SPIDRV_Handle_t sl_spidrv_exp_handle;

#define USART SL_WFX_HOST_PINOUT_SPI_PERIPHERAL

StaticSemaphore_t xEfrSpiSemaBuffer;
static SemaphoreHandle_t spi_sem;

static unsigned int tx_dma_channel;
static unsigned int rx_dma_channel;

static uint32_t dummy_rx_data;
static uint32_t dummy_tx_data;
static bool spi_enabled = false;

#if defined(EFR32MG12)
uint8_t wirq_irq_nb = SL_WFX_HOST_PINOUT_SPI_IRQ;
#elif defined(EFR32MG24)
uint8_t wirq_irq_nb = SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN; // SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN;
#endif

#define PIN_OUT_SET 1
#define PIN_OUT_CLEAR 0

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_init_bus(void)
 * @brief
 *  Initialize SPI peripheral
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_init_bus(void)
{
    spi_enabled = true;

    /* Assign allocated DMA channel */
    tx_dma_channel = sl_spidrv_exp_handle->txDMACh;
    rx_dma_channel = sl_spidrv_exp_handle->rxDMACh;

    /*
     * Route EUSART1 MOSI, MISO, and SCLK to the specified pins.  CS is
     * not controlled by EUSART so there is no write to the corresponding
     * EUSARTROUTE register to do this.
     */

#if defined(EFR32MG12)
    MY_USART->CTRL |= (1u << _USART_CTRL_SMSDELAY_SHIFT);
    MY_USART->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN;
#endif

#if defined(EFR32MG24)
    GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | // MISO
        GPIO_USART_ROUTEEN_TXPEN |                           // MOSI
        GPIO_USART_ROUTEEN_CLKPEN;
#endif

    spi_sem = xSemaphoreCreateBinaryStatic(&xEfrSpiSemaBuffer);
    xSemaphoreGive(spi_sem);

#if defined(EFR32MG24)
    spi_sem_sync_hdl = xSemaphoreCreateBinaryStatic(&spi_sem_peripharal);
    xSemaphoreGive(spi_sem_sync_hdl);
#endif
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_deinit_bus(void)
 * @brief
 *     De-initialize SPI peripheral and DMAs
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_deinit_bus(void)
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
 * @fn  sl_status_t sl_wfx_host_spi_cs_assert()
 * @brief
 *     Assert chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spi_cs_assert()
{
    GPIO_PinOutClear(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spi_cs_deassert()
 * @brief
 *     De-Assert chip select.
 * @param[in] None
 * @return returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_spi_cs_deassert()
{
    GPIO_PinOutSet(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  static bool dma_complete(unsigned int channel, unsigned int sequenceNo, void *userParam)
 * @brief
 *     function called when the DMA complete
 * @param[in] channel:
 * @param[in]  sequenceNo: sequence number
 * @param[in]  userParam: user parameter
 * @return returns true if suucessful,
 *          false otherwise
 *****************************************************************************/
static bool dma_complete(unsigned int channel, unsigned int sequenceNo, void * userParam)
{
    (void) channel;
    (void) sequenceNo;
    (void) userParam;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    return true;
}

/****************************************************************************
 * @fn   void receiveDMA(uint8_t *buffer, uint16_t buffer_length)
 * @brief
 *     start receive DMA
 * @param[in]  buffer:
 * @param[in]  buffer_length:
 * @return  None
 *****************************************************************************/
void receiveDMA(uint8_t * buffer, uint16_t buffer_length)
{
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    // Start receive DMA.
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, (void *) buffer, (void *) &(MY_USART->RXDATA), true, buffer_length,
                            dmadrvDataSize1, dma_complete, NULL);

    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) &(dummy_tx_data), false,
                            buffer_length, dmadrvDataSize1, NULL, NULL);
}

/****************************************************************************
 * @fn   void transmitDMA(uint8_t *buffer, uint16_t buffer_length)
 * @brief
 *     start  transmit DMA
 * @param[in]  buffer:
 * @param[in]  buffer_length:
 * @return  None
 *****************************************************************************/
void transmitDMA(uint8_t * buffer, uint16_t buffer_length)
{
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif

    // Receive DMA runs only to initiate callback
    // Start receive DMA.
    DMADRV_PeripheralMemory(rx_dma_channel, MY_USART_RX_SIGNAL, &dummy_rx_data, (void *) &(MY_USART->RXDATA), false, buffer_length,
                            dmadrvDataSize1, dma_complete, NULL);
    // Start transmit DMA.
    DMADRV_MemoryPeripheral(tx_dma_channel, MY_USART_TX_SIGNAL, (void *) &(MY_USART->TXDATA), (void *) buffer, true, buffer_length,
                            dmadrvDataSize1, NULL, NULL);
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type,
                                                  uint8_t *header,
                                                  uint16_t header_length,
                                                  uint8_t *buffer,
                                                  uint16_t buffer_length)
 * @brief
 * WFX SPI transfer implementation
 * @param[in] type:
 * @param[in] header:
 * @param[in] header_length:
 * @param[in] buffer:
 * @param[in] buffer_length:
 * @return  returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type, uint8_t * header, uint16_t header_length,
                                                  uint8_t * buffer, uint16_t buffer_length)
{
    sl_status_t result = SL_STATUS_FAIL;
#if defined(EFR32MG24)
    if (pr_type != EXP_HDR)
    {
        pr_type = EXP_HDR;
        set_spi_baudrate(pr_type);
    }
    if (xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY) != pdTRUE)
    {
        return SL_STATUS_TIMEOUT;
    }
    sl_wfx_host_spi_cs_assert();
#endif
    const bool is_read = (type == SL_WFX_BUS_READ);

    while (!(MY_USART->STATUS & USART_STATUS_TXBL))
    {
    }
    MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    /* header length should be greater than 0 */
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

    /* buffer length should be greater than 0 */
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
#if defined(EFR32MG24)
    sl_wfx_host_spi_cs_deassert();
    xSemaphoreGive(spi_sem_sync_hdl);
#endif
    return result;
}

/****************************************************************************
 * @fn   void sl_wfx_host_start_platform_interrupt(void)
 * @brief
 * Enable WFX interrupt
 * @param[in]  none
 * @return None
 *****************************************************************************/
void sl_wfx_host_start_platform_interrupt(void)
{
    // Enable (and clear) the bus interrupt
    GPIO_ExtIntConfig(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, wirq_irq_nb, true, false, true);
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_disable_platform_interrupt(void)
 * @brief
 * Disable WFX interrupt
 * @param[in]  None
 * @return  returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_disable_platform_interrupt(void)
{
    GPIO_IntDisable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_enable_platform_interrupt(void)
 * @brief
 *      enable the platform interrupt
 * @param[in]  None
 * @return  returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_enable_platform_interrupt(void)
{
    GPIO_IntEnable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_enable_spi(void)
 * @brief
 *       enable spi
 * @param[in]  None
 * @return  returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_enable_spi(void)
{
    if (spi_enabled == false)
    {
        spi_enabled = true;
    }
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_disable_spi(void)
 * @brief
 *       disable spi
 * @param[in]  None
 * @return  returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_disable_spi(void)
{
    if (spi_enabled == true)
    {
        spi_enabled = false;
    }
    return SL_STATUS_OK;
}

/*
 * IRQ for SPI callback
 * Clear the Interrupt and wake up the task that
 * handles the actions of the interrupt (typically - wfx_bus_task ())
 */
static void sl_wfx_spi_wakeup_irq_callback(uint8_t irqNumber)
{
    BaseType_t bus_task_woken;
    uint32_t interrupt_mask;

    if (irqNumber != wirq_irq_nb)
        return;
    // Get and clear all pending GPIO interrupts
    interrupt_mask = GPIO_IntGet();
    GPIO_IntClear(interrupt_mask);
    bus_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(wfx_wakeup_sem, &bus_task_woken);
    vTaskNotifyGiveFromISR(wfx_bus_task_handle, &bus_task_woken);
    portYIELD_FROM_ISR(bus_task_woken);
}

/****************************************************************************
 * Init some actions pins to the WF-200 expansion board
 *****************************************************************************/
void sl_wfx_host_gpio_init(void)
{
    SILABS_LOG("WIFI: GPIO Init:IRQ=%d", wirq_irq_nb);
    // Enable GPIO clock.
    CMU_ClockEnable(cmuClock_GPIO, true);

#if defined(EFR32MG24)
    // configure WF200 CS pin.
    GPIO_PinModeSet(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN, gpioModePushPull, 1);
#endif
    // Configure WF200 reset pin.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN, gpioModePushPull, 0);
    // Configure WF200 WUP pin.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_WUP_PORT, SL_WFX_HOST_PINOUT_WUP_PIN, gpioModePushPull, 0);

    // GPIO used as IRQ.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, gpioModeInputPull, 0);
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    // Set up interrupt based callback function - trigger on both edges.
    GPIOINT_Init();
    GPIO_ExtIntConfig(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, wirq_irq_nb, true, false,
                      false); /* Don't enable it */

    GPIOINT_CallbackRegister(wirq_irq_nb, sl_wfx_spi_wakeup_irq_callback);

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_ClearPendingIRQ(1 << wirq_irq_nb);
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);
}
