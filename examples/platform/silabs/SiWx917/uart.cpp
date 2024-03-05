/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
#include "AppConfig.h"
#include "USART.h"
#include "matter_shell.h"
#include "rsi_rom_egpio.h"
#include "silabs_utils.h"
#include "sl_si91x_usart.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "assert.h"
#include "rsi_board.h"
#include "rsi_debug.h"
#include "uart.h"
#include <stddef.h>
#include <string.h>

#define USART_BAUDRATE 115200 // Baud rate <9600-7372800>
#define UART_CONSOLE_ERR -1   // Negative value in case of UART Console action failed. Triggers a failure for PW_RPC

sl_usart_handle_t usart_handle;

void callback_event(uint32_t event);

/*******************************************************************************
 * Callback function triggered on data Transfer and reception
 ******************************************************************************/
void callback_event(uint32_t event)
{
    switch (event)
    {
    case SL_USART_EVENT_SEND_COMPLETE:
        break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
#ifdef ENABLE_CHIP_SHELL
        chip::NotifyShellProcessFromISR();
#endif
    case SL_USART_EVENT_TRANSFER_COMPLETE:
        break;
    }
}

void uartConsoleInit(void)
{
    int32_t status = 0;

    sl_si91x_usart_control_config_t usart_config;
    usart_config.baudrate      = USART_BAUDRATE;
    usart_config.mode          = SL_USART_MODE_ASYNCHRONOUS;
    usart_config.parity        = SL_USART_NO_PARITY;
    usart_config.stopbits      = SL_USART_STOP_BITS_1;
    usart_config.hwflowcontrol = SL_USART_FLOW_CONTROL_NONE;
    usart_config.databits      = SL_USART_DATA_BITS_8;
    usart_config.misc_control  = SL_USART_MISC_CONTROL_NONE;
    usart_config.usart_module  = USART_0;
    usart_config.config_enable = ENABLE;
    usart_config.synch_mode    = DISABLE;
    sl_si91x_usart_control_config_t get_config;

    // Initialize the UART
    status = sl_si91x_usart_init((usart_peripheral_t) usart_config.usart_module, &usart_handle);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("sl_si91x_usart_initialize: Error Code : %lu \n", status);
    }

    // Configure the USART configurations
    status = sl_si91x_usart_set_configuration(usart_handle, &usart_config);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("sl_si91x_usart_set_configuration: Error Code : %lu \n", status);
    }

    // Register user callback function
    status = sl_si91x_usart_register_event_callback(callback_event);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("sl_si91x_usart_register_event_callback: Error Code : %lu \n", status);
    }

    NVIC_EnableIRQ(USART0_IRQn);
    NVIC_SetPriority(USART0_IRQn, 7);
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer that contains the data to write, number bytes to write.
 *   @return Amount of bytes written or ERROR (-1)
 */
int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    int32_t status = 0;
    if (Buf == NULL || BufLength < 1)
    {
        return UART_CONSOLE_ERR;
    }

    status = sl_si91x_usart_send_data(usart_handle, Buf, BufLength);
    if (status != SL_STATUS_OK)
    {
        return status;
    }
    return BufLength;
}

/**
 * @brief Write Logs to the Uart. Appends a return character
 *
 * @param log pointer to the logs
 * @param length number of bytes to write
 * @return int16_t Amount of bytes written or ERROR (-1)
 */
int16_t uartLogWrite(const char * log, uint16_t length)
{
    if (log == NULL || length == 0)
    {
        return UART_CONSOLE_ERR;
    }
    for (uint16_t i = 0; i < length; i++)
    {
        Board_UARTPutChar(log[i]);
    }
    // To print next log in new line with proper formatting
    Board_UARTPutChar('\r');
    Board_UARTPutChar('\n');

    return length + 2;
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer for the data to be read, number bytes to read.
 *   @return Amount of bytes that was read from the rx fifo or ERROR (-1)
 */
int16_t uartConsoleRead(char * Buf, uint16_t NbBytesToRead)
{
    int32_t status = 0;
    if (Buf == NULL || NbBytesToRead < 1)
    {
        return UART_CONSOLE_ERR;
    }

    status = sl_si91x_usart_receive_data(usart_handle, Buf, NbBytesToRead);
    if (status != SL_STATUS_OK)
    {
        return status;
    }
    return NbBytesToRead;
}

#ifdef __cplusplus
}
#endif
