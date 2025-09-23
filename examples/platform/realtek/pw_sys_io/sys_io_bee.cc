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

#include "matter_uart.h"

#ifdef __cplusplus
extern "C" {
#endif
int bee_putchar(const uint8_t * aBuf);
int bee_getchar(uint8_t * read_byte);
#ifdef __cplusplus
}
#endif

extern "C" void pw_sys_io_Init()
{
    MatterPlatUartEnable();
}

namespace pw::sys_io {

Status ReadByte(std::byte * dest)
{
    if (!dest)
        return Status::InvalidArgument();

    int ret = bee_getchar(reinterpret_cast<uint8_t *>(dest));
    return ret < 0 ? Status::FailedPrecondition() : OkStatus();
}

Status WriteByte(std::byte b)
{
    int ret = bee_putchar(reinterpret_cast<const uint8_t *>(&b));
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
