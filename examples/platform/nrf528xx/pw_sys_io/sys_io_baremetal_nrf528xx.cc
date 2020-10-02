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

#include <cinttypes>

#include "pw_sys_io/sys_io.h"

#include "boards.h"
#include "nrf_uarte.h"
#include "uart/app_uart.h"

namespace {

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

} // namespace

#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */

extern "C" void pw_sys_io_Init()
{
    const app_uart_comm_params_t comm_params = { RX_PIN_NUMBER,
                                                 TX_PIN_NUMBER,
                                                 RTS_PIN_NUMBER,
                                                 CTS_PIN_NUMBER,
                                                 APP_UART_FLOW_CONTROL_DISABLED,
                                                 false,
                                                 NRF_UARTE_BAUDRATE_115200 };
    uint32_t err_code;
    APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOWEST, err_code);
    APP_ERROR_CHECK(err_code);
}

namespace pw::sys_io {

// Wait for a byte to read on UARTE. This blocks until a byte is read. This is
// extremely inefficient as it requires the target to burn CPU cycles polling to
// see if a byte is ready yet.
Status ReadByte(std::byte * dest)
{
    while (app_uart_get(reinterpret_cast<uint8_t *>(dest)) != NRF_SUCCESS)
    {
    }
    return Status::OK;
}

// Send a byte over UARTE. Since this blocks on every byte, it's rather
// inefficient. At the default baud rate of 115200, one byte blocks the CPU for
// ~87 micro seconds. This means it takes only 10 bytes to block the CPU for
// 1ms!
Status WriteByte(std::byte b)
{
    // Wait for TX buffer to be empty. When the buffer is empty, we can write
    // a value to be dumped out of UART.
    while (app_uart_put(*reinterpret_cast<uint8_t *>(&b)) != NRF_SUCCESS)
    {
    }
    return Status::OK;
}

// Writes a string using pw::sys_io, and add newline characters at the end.
StatusWithSize WriteLine(const std::string_view & s)
{
    size_t chars_written  = 0;
    StatusWithSize result = WriteBytes(std::as_bytes(std::span(s)));
    if (!result.ok())
    {
        return result;
    }
    chars_written += result.size();

    // Write trailing newline.
    result = WriteBytes(std::as_bytes(std::span("\r\n", 2)));
    chars_written += result.size();

    return StatusWithSize(result.status(), chars_written);
}

} // namespace pw::sys_io
