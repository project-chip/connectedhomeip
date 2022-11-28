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

#ifdef _WFX_NOT_USED_USING_HAL_INSTEAD_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "gpiointerrupt.h"

/* Need Lwip stuff before rsi is included */
#include "wfx_host_events.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "rsi_common_apis.h"
#include "rsi_data_types.h"
#include "rsi_error.h"
#include "rsi_nwk.h"
#include "rsi_socket.h"
#include "rsi_utils.h"
#include "rsi_wlan.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan_config.h"
#include "task.h"

#include "wfx_host_pinout.h"
#include "wfx_rsi.h"

/* The following stuff comes from hal/rsi_hal_mcu_interrupt.c */
static void (*rsi_intr_cb)(void);
/*********************************************************************
 * @fn  void rsi_hal_intr_config(void (*rsi_interrupt_handler)(void))
 * @brief
 *      get the hal intr configuration
 * @param[in]  rsi_interrupt_handler:
 * @return
 *      None
 ***********************************************************************/
void rsi_hal_intr_config(void (*rsi_interrupt_handler)(void))
{
    rsi_intr_cb = rsi_interrupt_handler;
}

/***********************************************************************
 * @fn  static void wfx_spi_wakeup_irq_callback(uint8_t irqNumber)
 * @brief
 *       end of stuff from hal/rsi_hal_mcu_interrupt.c
 * @param[in]  irqNumber:
 * @return  None
 * **********************************************************************/
static void wfx_spi_wakeup_irq_callback(uint8_t irqNumber)
{
    BaseType_t bus_task_woken;
    uint32_t interrupt_mask;

    if (irqNumber != SL_WFX_HOST_PINOUT_SPI_IRQ)
        return;
    // Get and clear all pending GPIO interrupts
    interrupt_mask = GPIO_IntGet();
    GPIO_IntClear(interrupt_mask);
    if (rsi_intr_cb)
        (*rsi_intr_cb)();
}

/***********************************************************************
 * @fn   static void wfx_host_gpio_init(void)
 * @brief
 *       function called when host gpio intialization
 * @param[in]  None
 * @return  None
 * **********************************************************************/
static void wfx_host_gpio_init(void)
{
    // Enable GPIO clock.
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure WF200 reset pin.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN, gpioModePushPull, PINOUT_CLEAR);
    // Configure WF200 WUP pin.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_WUP_PORT, SL_WFX_HOST_PINOUT_WUP_PIN, gpioModePushPull, PINOUT_CLEAR);

    // GPIO used as IRQ.
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, gpioModeInputPull, PINOUT_CLEAR);
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    // Set up interrupt based callback function - trigger on both edges.
    GPIOINT_Init();
    GPIO_ExtIntConfig(SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, SL_WFX_HOST_PINOUT_SPI_IRQ, true, false,
                      true);
    GPIOINT_CallbackRegister(SL_WFX_HOST_PINOUT_SPI_IRQ, wfx_spi_wakeup_irq_callback);

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_SetPriority(GPIO_EVEN_IRQn, WFX_GPIO_NVIC_PRIORITY);
    NVIC_SetPriority(GPIO_ODD_IRQn, WFX_GPIO_NVIC_PRIORITY);
}

#define USART SL_WFX_HOST_PINOUT_SPI_PERIPHERAL

/***********************************************************************
 * @fn   static int sl_wfx_host_spi_set_config(void *usart)
 * @brief
 *       set the configuration of spi
 * @param[in]  usart:
 * @return  returns 0 if successful,
 *         -1 otherwise
 * **********************************************************************/
static int sl_wfx_host_spi_set_config(void * usart)
{
    int ret = -1;

    if (0)
    {
#if defined(USART0)
    }
    else if (usart == USART0)
    {
        usart_clock     = cmuClock_USART0;
        usart_tx_signal = dmadrvPeripheralSignal_USART0_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART0_RXDATAV;
        ret             = 0;
#endif
#if defined(USART1)
    }
    else if (usart == USART1)
    {
        usart_clock     = cmuClock_USART1;
        usart_tx_signal = dmadrvPeripheralSignal_USART1_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART1_RXDATAV;
        ret             = 0;
#endif
#if defined(USART2)
    }
    else if (usart == USART2)
    {
        usart_clock     = cmuClock_USART2;
        usart_tx_signal = dmadrvPeripheralSignal_USART2_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART2_RXDATAV;
        ret             = 0;
#endif
#if defined(USART3)
    }
    else if (usart == USART3)
    {
        usart_clock     = cmuClock_USART3;
        usart_tx_signal = dmadrvPeripheralSignal_USART3_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART3_RXDATAV;
        ret             = 0;
#endif
#if defined(USART4)
    }
    else if (usart == USART4)
    {
        usart_clock     = cmuClock_USART4;
        usart_tx_signal = dmadrvPeripheralSignal_USART4_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART4_RXDATAV;
        ret             = 0;
#endif
#if defined(USART5)
    }
    else if (usart == USART5)
    {
        usart_clock     = cmuClock_USART5;
        usart_tx_signal = dmadrvPeripheralSignal_USART5_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART5_RXDATAV;
        ret             = 0;
#endif
#if defined(USARTRF0)
    }
    else if (usart == USARTRF0)
    {
        usart_clock     = cmuClock_USARTRF0;
        usart_tx_signal = dmadrvPeripheralSignal_USARTRF0_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USARTRF0_RXDATAV;
        ret             = 0;
#endif
#if defined(USARTRF1)
    }
    else if (usart == USARTRF1)
    {
        usart_clock     = cmuClock_USARTRF1;
        usart_tx_signal = dmadrvPeripheralSignal_USARTRF1_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USARTRF1_RXDATAV;
        ret             = 0;
#endif
    }

    return ret;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_init_bus(void)
 * @brief
 * Initialize SPI peripheral
 * @param[in] None
 * @return  returns SL_STATUS_OK if successful,
 *        SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_init_bus(void)
{
    int res;

    // Initialize and enable the USART
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

    res = sl_wfx_host_spi_set_config(USART);
    if (res != SPI_CONFIG_SUCCESS)
    {
        return SL_STATUS_FAIL;
    }

    spi_enabled        = true;
    dummy_tx_data      = 0;
    usartInit.baudrate = 36000000u;
    usartInit.msbf     = true;
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(usart_clock, true);
    USART_InitSync(USART, &usartInit);
    USART->CTRL |= (1u << _USART_CTRL_SMSDELAY_SHIFT);
    USART->ROUTELOC0 =
        (USART->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK)) |
        (SL_WFX_HOST_PINOUT_SPI_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN;
    GPIO_DriveStrengthSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, gpioDriveStrengthStrongAlternateStrong);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_TX_PORT, SL_WFX_HOST_PINOUT_SPI_TX_PIN, gpioModePushPull, PINOUT_CLEAR);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_RX_PORT, SL_WFX_HOST_PINOUT_SPI_RX_PIN, gpioModeInput, PINOUT_CLEAR);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, SL_WFX_HOST_PINOUT_SPI_CLK_PIN, gpioModePushPull, PINOUT_CLEAR);

    DMADRV_Init();
    DMADRV_AllocateChannel(&tx_dma_channel, NULL);
    DMADRV_AllocateChannel(&rx_dma_channel, NULL);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN, gpioModePushPull, PINOUT_SET);
    USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    return SL_STATUS_OK;
}

/***********************************************************************
 * @fn   void wfx_rsidev_init(void)
 * @brief
 *       function called when driver rsidev intialization
 * @param[in]  None
 * @return  None
 * **********************************************************************/
void wfx_rsidev_init(void)
{
    wfx_host_gpio_init();
}
#endif /* _NOT_USED */
