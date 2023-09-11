/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pw_sys_io/sys_io.h"
#include <cassert>
#include <cinttypes>
#include <stdio.h>
#include <string.h>

#include "serial_api.h"

#define UART_TX PA_18
#define UART_RX PA_19

serial_t sobj;
int console_getchar(uint8_t * chr)
{
    *chr = (uint8_t) serial_getc(&sobj);
    return 1;
}

int console_putchar(const char * chr)
{
    serial_putc(&sobj, *chr);
    return 1;
}

void console_init(void)
{
    serial_init(&sobj, UART_TX, UART_RX);
    serial_baud(&sobj, 115200);
    serial_format(&sobj, 8, ParityNone, 1);
}

extern "C" void pw_sys_io_Init()
{
    console_init();
}

namespace pw::sys_io {

Status ReadByte(std::byte * dest)
{
    if (!dest)
        return Status::InvalidArgument();

    int ret = console_getchar(reinterpret_cast<uint8_t *>(dest));
    return ret < 0 ? Status::FailedPrecondition() : OkStatus();
}

Status WriteByte(std::byte b)
{
    int ret = console_putchar(reinterpret_cast<const char *>(&b));
    return ret < 0 ? Status::FailedPrecondition() : OkStatus();
}

// Writes a string using pw::sys_io, and add newline characters at the end.
StatusWithSize WriteLine(const std::string_view & s)
{
    size_t chars_written  = 0;
    StatusWithSize result = WriteBytes(pw::as_bytes(pw::span(s)));
    if (!result.ok())
    {
        return result;
    }
    chars_written += result.size();
    result = WriteBytes(pw::as_bytes(pw::span("\r\n", 2)));
    chars_written += result.size();

    return StatusWithSize(result.status(), chars_written);
}

} // namespace pw::sys_io
