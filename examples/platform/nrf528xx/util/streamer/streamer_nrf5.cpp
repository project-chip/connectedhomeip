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

/**
 *    @file
 *      Source implementation of an input / output stream for stdio targets.
 */

#include <lib/shell/shell.h>

#ifdef NRF_SHELL_STREAMER

#include "app_uart.h"
#include "bsp.h"
#include "nrf_uarte.h"

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

namespace chip {
namespace Shell {

const size_t c_rx_buffer_size = 256; /**< UART RX buffer size. */
const size_t c_tx_buffer_size = 512; /**< UART TX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event) {}

void streamer_nrf5_init_internal()
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params = { RX_PIN_NUMBER,
                                                 TX_PIN_NUMBER,
                                                 RTS_PIN_NUMBER,
                                                 CTS_PIN_NUMBER,
                                                 APP_UART_FLOW_CONTROL_DISABLED,
                                                 false,
                                                 NRF_UARTE_BAUDRATE_115200 };

    APP_UART_FIFO_INIT(&comm_params, c_rx_buffer_size, c_tx_buffer_size, uart_error_handle, APP_IRQ_PRIORITY_LOWEST, err_code);
    APP_ERROR_CHECK(err_code);
}

int streamer_nrf5_init(streamer_t * streamer)
{
    return 0;
}

int streamer_nrf5_read(streamer_t * streamer, char * buf, size_t len)
{
    size_t count = 0;
    while (count < len && app_uart_get(reinterpret_cast<uint8_t *>(buf + count)) == NRF_SUCCESS)
    {
        count++;
#ifdef FREERTOS
        taskYIELD();
#endif // FREERTOS
    }

    return count;
}

int streamer_nrf5_write(streamer_t * streamer, const char * buf, size_t len)
{
    size_t count = 0;
    while (count < len && app_uart_put(buf[count]) == NRF_SUCCESS)
        count++;

    return count;
}

static streamer_t streamer_nrf5 = {
    .init_cb  = streamer_nrf5_init,
    .read_cb  = streamer_nrf5_read,
    .write_cb = streamer_nrf5_write,
};

streamer_t * streamer_get(void)
{
    static bool initialized = false;
    if (!initialized)
    {
        streamer_nrf5_init_internal();
        initialized = true;
    }
    return &streamer_nrf5;
}

} // namespace Shell
} // namespace chip

#endif //#ifdef NRF_SHELL_STREAMER
