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

#include "rsi_board_configuration.h"
#include "rsi_driver.h"
/*===========================================================*/
/**
 * @fn            void rsi_hal_config_gpio(uint8_t gpio_number,uint8_t mode,uint8_t value)
 * @brief         Configures gpio pin in output mode,with a value
 * @param[in]     uint8_t gpio_number, gpio pin number to be configured
 * @param[in]     uint8_t mode , input/output mode of the gpio pin to configure
 *                0 - input mode
 *                1 - output mode
 * @param[in]     uint8_t value, default value to be driven if gpio is configured in output mode
 *                0 - low
 *                1 - high
 * @param[out]    none
 * @return        none
 * @description This API is used to configure host gpio pin in output mode.
 */
void rsi_hal_config_gpio(uint8_t gpio_number, uint8_t mode, uint8_t value)
{

    CMU_ClockEnable(cmuClock_GPIO, true);

    switch (gpio_number)
    {
    case RSI_HAL_SLEEP_CONFIRM_PIN:
    case RSI_HAL_LP_SLEEP_CONFIRM_PIN:
        GPIO_PinModeSet(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin, gpioModeWiredOrPullDown, PINOUT_SET);
        break;
    case RSI_HAL_WAKEUP_INDICATION_PIN:
#ifndef LOGGING_STATS
        GPIO_PinModeSet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin, gpioModeWiredOrPullDown, PINOUT_CLEAR);
#else
        GPIO_PinModeSet(LOGGING_WAKE_INDICATOR_PIN.port, LOGGING_WAKE_INDICATOR_PIN.pin, gpioModeWiredOrPullDown, PINOUT_CLEAR);
#endif
        break;
    case RSI_HAL_RESET_PIN:
        GPIO_PinModeSet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin, gpioModePushPull, PINOUT_SET);
        break;
    default:
        break;
    }
}

/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio  value high
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none
 * @description   This API is used to drives or makes the host gpio value high.
 */
void rsi_hal_set_gpio(uint8_t gpio_number)
{
    switch (gpio_number)
    {
    case RSI_HAL_SLEEP_CONFIRM_PIN:
    case RSI_HAL_LP_SLEEP_CONFIRM_PIN:
        GPIO_PinModeSet(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin, gpioModeWiredOrPullDown, PINOUT_SET);
        break;
    case RSI_HAL_WAKEUP_INDICATION_PIN:
#ifndef LOGGING_STATS
        GPIO_PinModeSet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin, gpioModeInput, PINOUT_SET);
#else
        GPIO_PinModeSet(LOGGING_WAKE_INDICATOR_PIN.port, LOGGING_WAKE_INDICATOR_PIN.pin, gpioModeInput, PINOUT_SET);
#endif
        break;
    case RSI_HAL_RESET_PIN:
        GPIO_PinModeSet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin, gpioModeWiredOrPullDown, PINOUT_SET);
        break;
    default:
        break;
    }
}
/*===========================================================*/
/**
 * @fn          uint8_t rsi_hal_get_gpio(void)
 * @brief       get the gpio pin value
 * @param[in]   uint8_t gpio_number, gpio pin number
 * @param[out]  none
 * @return      gpio pin value
 * @description This API is used to configure get the gpio pin value.
 */
uint8_t rsi_hal_get_gpio(uint8_t gpio_number)
{
    switch (gpio_number)
    {
    case RSI_HAL_SLEEP_CONFIRM_PIN:
    case RSI_HAL_LP_SLEEP_CONFIRM_PIN:
        return GPIO_PinInGet(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin);
    case RSI_HAL_WAKEUP_INDICATION_PIN:
#ifndef LOGGING_STATS
        return GPIO_PinInGet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin);
#else
        return GPIO_PinInGet(LOGGING_WAKE_INDICATOR_PIN.port, LOGGING_WAKE_INDICATOR_PIN.pin);
#endif
    case RSI_HAL_RESET_PIN:
        return GPIO_PinInGet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin);
    case RSI_HAL_MODULE_INTERRUPT_PIN:
        return GPIO_PinInGet(WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin);
    default:
        break;
    }
    return 0;
}
/*===========================================================*/
/**
 * @fn            void rsi_hal_set_gpio(uint8_t gpio_number)
 * @brief         Makes/drives the gpio value to low
 * @param[in]     uint8_t gpio_number, gpio pin number
 * @param[out]    none
 * @return        none
 * @description   This API is used to drives or makes the host gpio value low.
 */
void rsi_hal_clear_gpio(uint8_t gpio_number)
{
    switch (gpio_number)
    {
    case RSI_HAL_SLEEP_CONFIRM_PIN:
    case RSI_HAL_LP_SLEEP_CONFIRM_PIN:
        GPIO_PinOutClear(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin);
        break;
    case RSI_HAL_WAKEUP_INDICATION_PIN:
#ifndef LOGGING_STATS
        GPIO_PinOutClear(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin);
#else
        GPIO_PinOutClear(LOGGING_WAKE_INDICATOR_PIN.port, LOGGING_WAKE_INDICATOR_PIN.pin);
#endif
        break;
    case RSI_HAL_RESET_PIN:
        GPIO_PinOutClear(WFX_RESET_PIN.port, WFX_RESET_PIN.pin);
        break;
    default:
        break;
    }
}
