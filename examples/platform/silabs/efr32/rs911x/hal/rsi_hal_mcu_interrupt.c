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
#include "em_usart.h"
#include "gpiointerrupt.h"
#include "sl_device_init_clocks.h"
#include "sl_status.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include "sl_board_configuration.h"

#include "sl_rsi_utility.h"
#include "sl_si91x_host_interface.h"

void gpio_interrupt(uint8_t interrupt_number);
#else
#include "rsi_board_configuration.h"
#include "rsi_driver.h"
#endif

typedef void (*UserIntCallBack_t)(void);
UserIntCallBack_t call_back, gpio_callback;
#ifdef LOGGING_STATS
uint8_t current_pin_set, prev_pin_set;
#endif /* LOGGING_STATS */

/* ARGSUSED */
void rsi_gpio_irq_cb(uint8_t irqnum)
{
    if (irqnum != SL_WFX_HOST_PINOUT_SPI_IRQ)
        return;
#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
    sl_si91x_host_set_bus_event(NCP_HOST_BUS_RX_EVENT);
#else
    GPIO_IntClear(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
    if (call_back != NULL)
        (*call_back)();
#endif
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_config(void (* rsi_interrupt_handler)())
 * @brief        Starts and enables the SPI interrupt
 * @param[in]    rsi_interrupt_handler() ,call back function to handle interrupt
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to initialize the register/pins
 *               related to interrupts and enable the interrupts.
 */
void rsi_hal_intr_config(void (*rsi_interrupt_handler)(void))
{
    call_back = rsi_interrupt_handler;
}

/*===================================================*/
/**
 * @fn           void rsi_hal_log_stats_intr_config(void (* rsi_give_wakeup_indication)())
 * @brief        Checks the interrupt and map/set gpio callback function
 * @param[in]    rsi_give_wakeup_indication() ,gpio call back function to handle interrupt
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code
 *               related to mapping of gpio callback function.
 */
#ifdef LOGGING_STATS
void rsi_hal_log_stats_intr_config(void (*rsi_give_wakeup_indication)())
{
    gpio_callback = rsi_give_wakeup_indication;
}
#endif /* LOGGING_STATS */

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_mask(void)
 * @brief        Disables the SPI Interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to mask/disable interrupts.
 */
void rsi_hal_intr_mask(void)
{
    GPIO_IntDisable(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_unmask(void)
 * @brief        Enables the SPI interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to enable interrupts.
 */
void rsi_hal_intr_unmask(void)
{
    // Unmask/Enable the interrupt
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    GPIO_IntEnable(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
}

/*===================================================*/
/**
 * @fn           void rsi_hal_intr_clear(void)
 * @brief        Clears the pending interrupt
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to clear the handled interrupts.
 */
void rsi_hal_intr_clear(void)
{
    GPIO_IntClear(1 << SL_WFX_HOST_PINOUT_SPI_IRQ);
}

/*===================================================*/
/**
 * @fn          void rsi_hal_intr_pin_status(void)
 * @brief       Checks the SPI interrupt at pin level
 * @param[in]   none
 * @param[out]  uint8_t, interrupt status
 * @return      none
 * @description This API is used to check interrupt pin status(pin level whether it is high/low).
 */
uint8_t rsi_hal_intr_pin_status(void)
{
    uint32_t mask;
    // Return interrupt pin  status(high(1) /low (0))
    mask = GPIO_PinInGet(WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin);

    return !!mask;
}
