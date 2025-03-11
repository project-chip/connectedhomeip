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
#include "FreeRTOS.h"
#include "dmadrv.h"
#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "gpiointerrupt.h"
#include "semphr.h"
#include "sl_spidrv_exp_config.h"
#include "sl_spidrv_instances.h"
#include "sl_wfx.h"
#include "sl_wfx_board.h"
#include "sl_wfx_configuration_defaults.h"
#include "sl_wfx_host.h"
#include "sl_wfx_host_api.h"
#include "sl_wfx_task.h"
#include "spidrv.h"
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#include <platform/silabs/wifi/wf200/ncp/efr_spi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if SL_LCDCTRL_MUX
#include "sl_memlcd_display.h"
#define SL_SPIDRV_LCD_BITRATE SL_MEMLCD_SCLK_FREQ
#endif // SL_LCDCTRL_MUX
#if SL_MX25CTRL_MUX
#include "sl_mx25_flash_shutdown_usart_config.h"
#define SL_SPIDRV_MX25_FLASH_BITRATE 16000000
#endif // SL_MX25CTRL_MUX

// TODO: (MATTER-1906) Investigate why using SL_SPIDRV_EXP_BITRATE is causing WF200 init failure
// REF: sl_spidrv_exp_config.h
#define SL_SPIDRV_EXP_BITRATE_MULTIPLEXED 10000000
#define SL_SPIDRV_UART_CONSOLE_BITRATE SL_UARTDRV_USART_VCOM_BAUDRATE
#define SL_SPIDRV_FRAME_LENGTH SL_SPIDRV_EXP_FRAME_LENGTH
#define SL_SPIDRV_HANDLE sl_spidrv_exp_handle

#define USART SL_WFX_HOST_PINOUT_SPI_PERIPHERAL

#if SL_SPICTRL_MUX
StaticSemaphore_t spi_sem_peripheral;
static SemaphoreHandle_t spi_sem_sync_hdl;
#endif // SL_SPICTRL_MUX

StaticSemaphore_t xEfrSpiSemaBuffer;
static SemaphoreHandle_t spi_sem;

static unsigned int tx_dma_channel;
static unsigned int rx_dma_channel;

static uint32_t dummy_rx_data;
static uint32_t dummy_tx_data;
static bool spi_enabled = false;

uint8_t wirq_irq_nb = SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN; // SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN;

#define PIN_OUT_SET 1
#define PIN_OUT_CLEAR 0

sl_status_t sl_wfx_host_init_bus(void)
{
    spi_enabled = true;

    /* Assign allocated DMA channel */
    tx_dma_channel = SL_SPIDRV_HANDLE->txDMACh;
    rx_dma_channel = SL_SPIDRV_HANDLE->rxDMACh;

    /*
     * Route EUSART1 MOSI, MISO, and SCLK to the specified pins.  CS is
     * not controlled by EUSART so there is no write to the corresponding
     * EUSARTROUTE register to do this.
     */

    GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | // MISO
        GPIO_USART_ROUTEEN_TXPEN |                           // MOSI
        GPIO_USART_ROUTEEN_CLKPEN;

    spi_sem = xSemaphoreCreateBinaryStatic(&xEfrSpiSemaBuffer);
    xSemaphoreGive(spi_sem);

#if SL_SPICTRL_MUX
    if (spi_sem_sync_hdl == NULL)
    {
        spi_sem_sync_hdl = xSemaphoreCreateBinaryStatic(&spi_sem_peripheral);
    }
    configASSERT(spi_sem_sync_hdl);
    xSemaphoreGive(spi_sem_sync_hdl);
#endif /* SL_SPICTRL_MUX */
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_deinit_bus(void)
{
    vSemaphoreDelete(spi_sem);
#if SL_SPICTRL_MUX
    vSemaphoreDelete(spi_sem_sync_hdl);
#endif /* SL_SPICTRL_MUX */
    // Stop DMAs.
    DMADRV_StopTransfer(rx_dma_channel);
    DMADRV_StopTransfer(tx_dma_channel);
    DMADRV_FreeChannel(tx_dma_channel);
    DMADRV_FreeChannel(rx_dma_channel);
    DMADRV_DeInit();
    USART_Reset(MY_USART);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_assert()
{
#if SL_SPICTRL_MUX
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
    SPIDRV_SetBaudrate(SL_SPIDRV_EXP_BITRATE_MULTIPLEXED);
#endif /* SL_SPICTRL_MUX */
    GPIO_PinOutClear(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_deassert()
{
    GPIO_PinOutSet(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN);
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif /* SL_SPICTRL_MUX */
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

sl_status_t sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type, uint8_t * header, uint16_t header_length,
                                                  uint8_t * buffer, uint16_t buffer_length)
{
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
            MY_USART->TXDATA = (uint32_t) (*buffer_ptr);

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
        // Reset the semaphore
        configASSERT(spi_sem);
        xSemaphoreTake(spi_sem, portMAX_DELAY);
        if (is_read)
        {
            receiveDMA(buffer, buffer_length);
        }
        else
        {
            transmitDMA(buffer, buffer_length);
        }
        // wait for dma_complete by using the same spi_semaphore
        xSemaphoreTake(spi_sem, portMAX_DELAY);
        xSemaphoreGive(spi_sem);
    }
    return SL_STATUS_OK;
}

void sl_wfx_host_start_platform_interrupt(void)
{
    // Enable (and clear) the bus interrupt
    GPIO_ExtIntConfig(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, wirq_irq_nb, true, false, true);
}

sl_status_t sl_wfx_host_disable_platform_interrupt(void)
{
    GPIO_IntDisable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_enable_platform_interrupt(void)
{
    GPIO_IntEnable(1 << wirq_irq_nb);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_enable_spi(void)
{
    if (spi_enabled == false)
    {
        spi_enabled = true;
    }
    return SL_STATUS_OK;
}

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

void sl_wfx_host_gpio_init(void)
{
    // Enable GPIO clock.
    CMU_ClockEnable(cmuClock_GPIO, true);
    // configure WF200 CS pin.
    GPIO_PinModeSet(SL_SPIDRV_EXP_CS_PORT, SL_SPIDRV_EXP_CS_PIN, gpioModePushPull, 1);
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

#if SL_SPICTRL_MUX
void SPIDRV_SetBaudrate(uint32_t baudrate)
{
    if (USART_BaudrateGet(MY_USART) == baudrate)
    {
        // USART synced to baudrate already
        return;
    }
    // USART is used in MG24 + WF200 combination
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.msbf                   = true;
    usartInit.clockMode              = usartClockMode0;
    usartInit.baudrate               = baudrate;
    uint32_t databits                = SL_SPIDRV_FRAME_LENGTH - 4U + _USART_FRAME_DATABITS_FOUR;
    usartInit.databits               = (USART_Databits_TypeDef) databits;
    usartInit.autoCsEnable           = true;

    USART_InitSync(MY_USART, &usartInit);
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
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
#endif // SL_SPICTRL_MUX
    /*
     * Assert CS pin for EXT SPI Flash
     */
    SPIDRV_SetBaudrate(SL_SPIDRV_MX25_FLASH_BITRATE);
#if SL_MX25CTRL_MUX
    sl_wfx_host_spiflash_cs_assert();
#endif // SL_MX25CTRL_MUX
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_bootloader_spi_transfer(void)
{
    /*
     * De-Assert CS pin for EXT SPI Flash
     */
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
    SPIDRV_SetBaudrate(SL_SPIDRV_LCD_BITRATE);
    /*LCD CS is handled as part of LCD gsdk*/
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_lcd_spi_transfer(void)
{
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    return SL_STATUS_OK;
}
#endif // SL_LCDCTRL_MUX

#if SL_UARTCTRL_MUX
sl_status_t sl_wfx_host_pre_uart_transfer(void)
{
#if SL_SPICTRL_MUX
    if (spi_sem_sync_hdl == NULL)
    {
#endif // SL_SPICTRL_MUX
       // UART is initialized before host SPI interface
       // spi_sem_sync_hdl will not be initalized during execution
        GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1);
#if SL_SPICTRL_MUX
        return SL_STATUS_OK;
    }
#endif // SL_SPICTRL_MUX
    sl_wfx_disable_irq();
    sl_wfx_host_disable_platform_interrupt();
#if SL_SPICTRL_MUX
    xSemaphoreTake(spi_sem_sync_hdl, portMAX_DELAY);
#endif // SL_SPICTRL_MUX
    GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_post_uart_transfer(void)
{
#if SL_SPICTRL_MUX
    if (spi_sem_sync_hdl == NULL)
    {
        return SL_STATUS_OK;
    }
#endif // SL_SPICTRL_MUX
    GPIO_PinModeSet(gpioPortA, 8, gpioModeInputPull, 1);
#if SL_SPICTRL_MUX
    xSemaphoreGive(spi_sem_sync_hdl);
#endif // SL_SPICTRL_MUX
    sl_wfx_host_enable_platform_interrupt();
    sl_wfx_enable_irq();
    return SL_STATUS_OK;
}
#endif // SL_UARTCTRL_MUX
