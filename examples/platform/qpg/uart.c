/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "uart.h"
#include "FreeRTOS.h"
#include "qvIO.h"
#include "task.h"
#include <stddef.h>
#include <stdio.h>

#define UART_CONSOLE_ERR -1 // Negative value in case of UART Console action failed. Triggers a failure for PW_RPC

void uartConsoleInit(void)
{
    qvIO_UartInit();
}

int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return UART_CONSOLE_ERR;
    }

    qvIO_UartTxData((uint8_t) BufLength, Buf);
    return (int16_t) BufLength;
}

int16_t uartConsoleRead(char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return UART_CONSOLE_ERR;
    }
    vTaskDelay(1); // taskYIELD or vTaskDelay(0) is not sufficient
    uint8_t rxLen = qvIO_UartReadRxData((uint8_t) BufLength, Buf);
    return (int16_t) rxLen;
}
