/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "pw_sys_io/sys_io.h"
#include <cassert>
#include <cinttypes>
#include <stdio.h>
#include <string.h>

#include "uart.h"

int16_t console_getchar(char * chr)
{
    return uartConsoleRead(chr, 1);
}

int16_t console_putchar(const char * chr)
{
    return uartConsoleWrite(chr, 1);
}

extern "C" void pw_sys_io_Init()
{
    uartConsoleInit();
}

namespace pw::sys_io {

Status ReadByte(std::byte * dest)
{
    if (!dest)
    {
        return Status::InvalidArgument();
    }

    int16_t ret = console_getchar(reinterpret_cast<char *>(dest));
    return ret < 0 ? Status::FailedPrecondition() : OkStatus();
}

Status WriteByte(std::byte b)
{
    int16_t ret = console_putchar(reinterpret_cast<const char *>(&b));
    return ret < 0 ? Status::FailedPrecondition() : OkStatus();
}

// Writes a string using pw::sys_io, and add newline characters at the end.
StatusWithSize WriteLine(std::string_view s)
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
