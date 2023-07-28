/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "rsi_board.h"
#include "rsi_chip.h"
#include "rsi_driver.h"
#include "rsi_m4.h"
#ifdef COMMON_FLASH_EN
#include "rsi_power_save.h"
#endif

uint8_t btn0 = 1;
uint8_t btn1 = 1;

void sl_button_on_change(uint8_t btn, uint8_t btnAction);

uint32_t NVIC_GetIRQEnable(IRQn_Type IRQn)
{
    return ((NVIC->ICER[((uint32_t)(IRQn) >> 5)] & (1 << ((uint32_t)(IRQn) &0x1F))) ? 1 : 0);
}

void rsi_assertion(uint16_t assertion_val, const char * string)
{
    uint16_t i;
    if (assertion_val == 0)
    {
        for (i = 0; i < strlen(string); i++)
        {
#ifdef DEBUG_UART
#ifdef DEBUG_ASSERTION
            Board_UARTPutChar(string[i]);
#endif
#endif
        }

        return;
    }
    else
    {
        for (i = 0; i < strlen(string); i++)
        {
#ifdef DEBUG_UART
#ifdef DEBUG_ASSERTION
            Board_UARTPutChar(string[i]);
#endif
#endif
        }

        while (1)
            ;
    }
}

void IRQ021_Handler(void)
{
    // TODO: Replace with rsi_delay once that is fixed
    for (int i = 0; i < 10000; i++)
        __asm__("nop;");
    /* clear NPSS GPIO interrupt*/
    RSI_NPSSGPIO_ClrIntr(NPSS_GPIO_0_INTR);
    RSI_NPSSGPIO_ClrIntr(NPSS_GPIO_2_INTR);
    // if the btn is not pressed setting the state to 1
    if (RSI_NPSSGPIO_GetPin(NPSS_GPIO_2))
    {
        btn1 = 1;
    }
    // geting the state of the gpio 2 pin and checking if the btn is already pressed or not
    if (!RSI_NPSSGPIO_GetPin(NPSS_GPIO_2) && btn1)
    {
        btn1 = 0;
        sl_button_on_change(1, 1);
    }
    if (RSI_NPSSGPIO_GetPin(NPSS_GPIO_0) && (!btn0))
    {
        btn0 = 1;
        sl_button_on_change(0, 0);
    }
    if (!RSI_NPSSGPIO_GetPin(NPSS_GPIO_0) && btn0)
    {
        btn0 = 0;
        sl_button_on_change(0, 1);
    }
}