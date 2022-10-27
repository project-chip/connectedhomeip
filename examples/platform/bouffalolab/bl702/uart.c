/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may nCHIP use this file except in compliance with the License.
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

#include <FreeRTOS.h>
#include <semphr.h>

#include <bl_uart.h>
#include <hosal_uart.h>

#include <board.h>

extern hosal_uart_dev_t uart_stdio;

#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED

#define CHIP_UART_RX_TIMER_ID 1
#define MAX_BUFFER_SIZE 256

typedef struct _chipUart
{
    SemaphoreHandle_t sema;
    StaticSemaphore_t mutx;
    uint32_t head;
    uint32_t tail;
    uint8_t rxbuf[MAX_BUFFER_SIZE];
} chipUart_t;

static chipUart_t chipUart_var;

static int uartTxCallback(void * p_arg)
{
    hosal_uart_ioctl(&uart_stdio, HOSAL_UART_TX_TRIGGER_OFF, NULL);

    return 0;
}

static int uartRxCallback(void * p_arg)
{
    uint32_t len = 0, readlen = 0;
    BaseType_t xHigherPriorityTaskWoken = 1;

    if (chipUart_var.head >= chipUart_var.tail)
    {
        if (chipUart_var.head < MAX_BUFFER_SIZE)
        {
            readlen = len =
                hosal_uart_receive(&uart_stdio, chipUart_var.rxbuf + chipUart_var.head, MAX_BUFFER_SIZE - chipUart_var.head);
            chipUart_var.head = (chipUart_var.head + len) % MAX_BUFFER_SIZE;
        }

        if (0 == chipUart_var.head)
        {
            len = hosal_uart_receive(&uart_stdio, chipUart_var.rxbuf, chipUart_var.tail - 1);
            chipUart_var.head += len;
            readlen += len;
        }
    }
    else
    {
        readlen =
            hosal_uart_receive(&uart_stdio, chipUart_var.rxbuf + chipUart_var.head, chipUart_var.tail - chipUart_var.head - 1);
        chipUart_var.head += readlen;
    }

    if (0 == readlen || (chipUart_var.head + 1) % MAX_BUFFER_SIZE == chipUart_var.tail)
    {
        xSemaphoreGiveFromISR(chipUart_var.sema, &xHigherPriorityTaskWoken);
    }

    return 0;
}

void uartInit(void)
{
    memset(&chipUart_var, 0, offsetof(chipUart_t, rxbuf));

    chipUart_var.sema = xSemaphoreCreateBinaryStatic(&chipUart_var.mutx);

    hosal_uart_finalize(&uart_stdio);
    hosal_uart_init(&uart_stdio);
    hosal_uart_callback_set(&uart_stdio, HOSAL_UART_RX_CALLBACK, uartRxCallback, NULL);
    hosal_uart_callback_set(&uart_stdio, HOSAL_UART_TX_CALLBACK, uartTxCallback, NULL);
    hosal_uart_ioctl(&uart_stdio, HOSAL_UART_MODE_SET, (void *) HOSAL_UART_MODE_INT);
}

int16_t uartRead(char * Buf, uint16_t NbBytesToRead)
{
    int16_t len = 0;

    xSemaphoreTake(chipUart_var.sema, portMAX_DELAY);

    taskENTER_CRITICAL();

    for (len = 0; len < NbBytesToRead && chipUart_var.head != chipUart_var.tail; len++)
    {
        Buf[len]          = chipUart_var.rxbuf[chipUart_var.tail];
        chipUart_var.tail = (chipUart_var.tail + 1) % MAX_BUFFER_SIZE;
    }

    taskEXIT_CRITICAL();

    if (chipUart_var.head != chipUart_var.tail)
    {
        xSemaphoreGive(chipUart_var.sema);
    }

    return len;
}
#endif

int16_t uartWrite(const char * Buf, uint16_t BufLength)
{
    return hosal_uart_send(&uart_stdio, Buf, BufLength);
}
