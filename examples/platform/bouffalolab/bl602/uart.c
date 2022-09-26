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
#include "uart.h"
#include "hosal_uart.h"
#include <string.h>

#define MAX_BUFFER_SIZE 256

extern hosal_uart_dev_t uart_stdio;

typedef struct _uartFifo
{
    uint16_t head;
    uint16_t tail;
    uint8_t rxbuf[MAX_BUFFER_SIZE];
} UartFifo_t;

static UartFifo_t UartFifo_v;

static uint16_t availableDataSize()
{
    if (UartFifo_v.tail >= UartFifo_v.head)
    {
        return UartFifo_v.tail - UartFifo_v.head;
    }
    else
    {
        return MAX_BUFFER_SIZE - UartFifo_v.head + UartFifo_v.tail;
    }
}

static uint16_t readFromFifo(uint8_t * dstBuf, uint16_t NbBytesToRead)
{
    uint16_t currentDataSize = availableDataSize();
    uint16_t sizeToRead      = (NbBytesToRead >= currentDataSize) ? currentDataSize : NbBytesToRead;
    uint16_t bytesBeforeWrap = MAX_BUFFER_SIZE - UartFifo_v.head;

    if (sizeToRead)
    {
        if (bytesBeforeWrap >= sizeToRead)
        {
            memcpy(dstBuf, UartFifo_v.rxbuf + UartFifo_v.head, sizeToRead);
        }
        else
        {
            memcpy(dstBuf, UartFifo_v.rxbuf + UartFifo_v.head, bytesBeforeWrap);
            memcpy(dstBuf + bytesBeforeWrap, UartFifo_v.rxbuf, sizeToRead - bytesBeforeWrap);
        }

        UartFifo_v.head = (UartFifo_v.head + sizeToRead) % MAX_BUFFER_SIZE;
    }

    return sizeToRead;
}

static void writeToFifo(uint8_t * buf, uint16_t NbBytesToWrite)
{
    uint16_t currentCapacity = MAX_BUFFER_SIZE - availableDataSize();

    if (currentCapacity >= NbBytesToWrite)
    {
        uint16_t bytesBeforeWrap = MAX_BUFFER_SIZE - UartFifo_v.tail;
        if (bytesBeforeWrap >= NbBytesToWrite)
        {
            memcpy(UartFifo_v.rxbuf + UartFifo_v.tail, buf, NbBytesToWrite);
        }
        else
        {
            memcpy(UartFifo_v.rxbuf + UartFifo_v.tail, buf, bytesBeforeWrap);
            memcpy(UartFifo_v.rxbuf, buf + bytesBeforeWrap, NbBytesToWrite - bytesBeforeWrap);
        }

        UartFifo_v.tail = (UartFifo_v.tail + NbBytesToWrite) % MAX_BUFFER_SIZE;
    }
}

static int uartRxCallback(void * p_arg)
{
    uint8_t data_buf[32];

    int ret = hosal_uart_receive(&uart_stdio, data_buf, sizeof(data_buf));
    if (ret)
    {
        writeToFifo(data_buf, ret);
    }

    return 0;
}

static int uartTxCallback(void * p_arg)
{
    hosal_uart_ioctl(&uart_stdio, HOSAL_UART_TX_TRIGGER_OFF, NULL);

    return 0;
}

void uartConsoleInit(void)
{
    memset(&UartFifo_v, 0, offsetof(UartFifo_t, rxbuf));

    hosal_uart_finalize(&uart_stdio);
    hosal_uart_init(&uart_stdio);
    hosal_uart_callback_set(&uart_stdio, HOSAL_UART_RX_CALLBACK, uartRxCallback, NULL);
    hosal_uart_callback_set(&uart_stdio, HOSAL_UART_TX_CALLBACK, uartTxCallback, NULL);
    hosal_uart_ioctl(&uart_stdio, HOSAL_UART_MODE_SET, (void *) HOSAL_UART_MODE_INT);
}

int16_t uartConsoleWrite(const char * Buf, uint16_t BufLength)
{
    if (Buf == NULL || BufLength < 1)
    {
        return -1;
    }

    return hosal_uart_send(&uart_stdio, Buf, BufLength);
}

int16_t uartConsoleRead(char * Buf, uint16_t NbBytesToRead)
{
    if (Buf == NULL || NbBytesToRead < 1)
    {
        return -1;
    }

    return readFromFifo(Buf, NbBytesToRead);
}
