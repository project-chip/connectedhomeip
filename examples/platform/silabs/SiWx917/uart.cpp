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
#ifdef __cplusplus
extern "C" {
#endif
#include "assert.h"
#include "rsi_board.h"
#include "uart.h"
#include <stddef.h>
#include <string.h>

extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * UARTdrv = &Driver_USART0;

ARM_USART_CAPABILITIES drv_capabilities;

#define BAUD_VALUE 115200
#define UART_CONSOLE_ERR -1 // Negative value in case of UART Console action failed. Triggers a failure for PW_RPC

void ARM_USART_SignalEvent(uint32_t event);

void Read_Capabilities(void)
{
    drv_capabilities = UARTdrv->GetCapabilities();
}

void ARM_USART_SignalEvent(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_SEND_COMPLETE:
        break;
    case ARM_USART_EVENT_RECEIVE_COMPLETE:
#ifdef ENABLE_CHIP_SHELL
        chip::NotifyShellProcessFromISR();
#endif;
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
    case ARM_USART_EVENT_TX_COMPLETE:
    case ARM_USART_EVENT_TX_UNDERFLOW:
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_RX_TIMEOUT:
    case ARM_USART_EVENT_RX_BREAK:
    case ARM_USART_EVENT_RX_FRAMING_ERROR:
    case ARM_USART_EVENT_RX_PARITY_ERROR:
    case ARM_USART_EVENT_CTS:
    case ARM_USART_EVENT_DSR:
    case ARM_USART_EVENT_DCD:
    case ARM_USART_EVENT_RI:
        break;
    }
}

void uartConsoleInit(void)
{
    int32_t status = 0;
    Read_Capabilities();

    status = UARTdrv->Initialize(ARM_USART_SignalEvent);
    // Setting the GPIO 30 of the radio board (TX)
    // Setting the GPIO 29 of the radio board (RX)
    RSI_EGPIO_HostPadsGpioModeEnable(30);
    RSI_EGPIO_HostPadsGpioModeEnable(29);

    // Initialized board UART
    DEBUGINIT();
    if (status != ARM_DRIVER_OK)
    {
        DEBUGOUT("\r\n UART Initialization Failed, Error Code : %d\r\n", status);
    }
    else
    {
        DEBUGOUT("\r\n UART Initialization Success\r\n");
    }

    // Power up the UART peripheral
    status = UARTdrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK)
    {
        DEBUGOUT("\r\n Failed to Set Power to UART, Error Code : %d\r\n", status);
    }
    else
    {
        DEBUGOUT("\r\n Configured Power to UART \r\n");
    }

    // Enable Receiver and Transmitter lines
    status = UARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    if (status != ARM_DRIVER_OK)
    {
        DEBUGOUT("\r\n Failed to Set  Transmitter lines to UART, Error Code : %d\r\n", status);
    }
    else
    {
        DEBUGOUT("\r\n Set  Transmitter lines to UART is sucess \r\n");
    }

    status = UARTdrv->Control(ARM_USART_CONTROL_RX, 1);
    if (status != ARM_DRIVER_OK)
    {
        DEBUGOUT("\r\n Failed to Set  Receiver lines to UART, Error Code : %d \r\n", status);
    }
    else
    {
        DEBUGOUT("\r\n Set  Receiver lines to UART\r\n");
    }

    UARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 |
                         ARM_USART_FLOW_CONTROL_NONE,
                     BAUD_VALUE);
    if (status != ARM_DRIVER_OK)
    {
        DEBUGOUT("\r\n Failed to Receive data , Error Code : %d \r\n", status);
    }
    else
    {
        DEBUGOUT("\r\n Receives data success  \r\n");
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

    status = UARTdrv->Send(Buf, BufLength);
    if (status != ARM_DRIVER_OK)
    {
        return status;
    }
    return BufLength;
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
    status = UARTdrv->Receive(Buf, NbBytesToRead);
    if (status != ARM_DRIVER_OK)
    {
        return status;
    }
    return NbBytesToRead;
}

#ifdef __cplusplus
}
#endif
