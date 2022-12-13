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
#include "matter_shell.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "assert.h"
#include "rsi_board.h"
#include "uart.h"
#include "uartdrv.h"
#include <stddef.h>
#include <string.h>

#if !defined(MIN)
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

#define UART_CONSOLE_ERR -1 // Negative value in case of UART Console action failed. Triggers a failure for PW_RPC

void uartConsoleInit(void)
{
    // UART init is already done
}

void USART_IRQHandler(void)
{
#ifdef ENABLE_CHIP_SHELL
    chip::NotifyShellProcessFromISR();
#endif
#if defined(SL_WIFI)
    /* TODO */
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif

#if (defined(EFR32MG24) || defined(MGM24))
    EUSART_IntClear(SL_UARTDRV_EUSART_VCOM_PERIPHERAL, EUSART_IF_RXFL);
#endif
}

/*
 *   @brief Callback triggered when a UARTDRV DMA buffer is full
 */
static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t * data, UARTDRV_Count_t transferCount)
{
    (void) transferStatus;

    uint8_t writeSize = (transferCount - lastCount);
    if (RemainingSpace(&sReceiveFifo) >= writeSize)
    {
        WriteToFifo(&sReceiveFifo, data + lastCount, writeSize);
        lastCount = 0;
    }

    UARTDRV_Receive(vcom_handle, data, transferCount, UART_rx_callback);

#ifdef ENABLE_CHIP_SHELL
    chip::NotifyShellProcessFromISR();
#endif
#if defined(SL_WIFI)
    /* TODO */
#elif !defined(PW_RPC_ENABLED)
    otSysEventSignalPending();
#endif
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer that contains the data to write, number bytes to write.
 *   @return Amount of bytes written or ERROR (-1)
 */
int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return UART_CONSOLE_ERR;
    }

    // Add Terminating char at the end of buffer
    Buf[BufLength] = '\0';

    Board_UARTPutSTR((uint8_t *) Buf);

    return BufLength;
}

/*
 *   @brief Read the data available from the console Uart
 *   @param Buffer for the data to be read, number bytes to read.
 *   @return Amount of bytes that was read from the rx fifo or ERROR (-1)
 */
int16_t uartConsoleRead(char * Buf, uint16_t NbBytesToRead)
{
    uint32_t data;

    if (Buf == NULL || NbBytesToRead < 1)
    {
        return UART_CONSOLE_ERR;
    }

    while (--NbBytesToRead >= 0)
    {
        data   = Board_UARTGetChar();
        *Buf++ = (char) data;
    }

    return NbBytesToRead;
}

#ifdef __cplusplus
}
#endif
