/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include "lega_rtos_api.h"
#include "printf_uart.h"
#include <stdio.h>
#include <string.h>
#ifdef CFG_PLF_RV32
#include "asr_uart.h"
#define duet_uart_set_callback asr_uart_set_callback
#define duet_uart_callback_func asr_uart_callback_func_t
#elif defined CFG_PLF_DUET
#include "duet_uart.h"
#else
#include "lega_uart.h"
#define duet_uart_set_callback lega_uart_set_callback
#define duet_uart_callback_func lega_uart_callback_func
#define UART1_INDEX LEGA_UART1_INDEX
#endif

namespace chip {
namespace Shell {
namespace {

#define HAL_UART_BUF_QUEUE_BYTES 256
lega_queue_t hal_uart_buf_queue;

void shell_handle_uartirq(char ch)
{
    lega_rtos_push_to_queue(&hal_uart_buf_queue, &ch, LEGA_NEVER_TIMEOUT);
}

int streamer_asr_init(streamer_t * streamer)
{
    (void) streamer;
    duet_uart_set_callback(UART1_INDEX, (duet_uart_callback_func) (shell_handle_uartirq));
    lega_rtos_init_queue(&hal_uart_buf_queue, "shell_buffer_queue", sizeof(char), HAL_UART_BUF_QUEUE_BYTES);
    return 0;
}

ssize_t streamer_asr_read(streamer_t * streamer, char * buffer, size_t length)
{
    (void) streamer;
    uint16_t i        = 0;
    uint32_t rx_count = 0;
    int32_t ret;
    uint8_t * pdata = (uint8_t *) buffer;
    for (i = 0; i < (uint16_t) length; i++)
    {
        ret = lega_rtos_pop_from_queue(&hal_uart_buf_queue, &pdata[i], LEGA_NEVER_TIMEOUT);

        if (!ret)
        {
            rx_count++;
        }
        else
        {
            break;
        }
    }
    return rx_count;
}

ssize_t streamer_asr_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    uart_put_buf(buffer, (uint16_t) length);
    return length;
}

static streamer_t streamer_asr = {
    .init_cb  = streamer_asr_init,
    .read_cb  = streamer_asr_read,
    .write_cb = streamer_asr_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_asr;
}

} // namespace Shell
} // namespace chip
