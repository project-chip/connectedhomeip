/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <cinttypes>

#include "pw_sys_io/sys_io.h"

#include <platform/FileHandle.h>
#include <platform/mbed_retarget.h>

using namespace mbed;

static FileHandle * console_in;
static FileHandle * console_out;

extern "C" void pw_sys_io_Init()
{
    console_in  = mbed_file_handle(STDIN_FILENO);
    console_out = mbed_file_handle(STDOUT_FILENO);
}

namespace pw::sys_io {

Status ReadByte(std::byte * dest)
{
    if (!dest)
    {
        return Status::InvalidArgument();
    }

    char c;
    auto ret = console_in->read(&c, 1);
    *dest    = static_cast<std::byte>(c);

    return ret != 1 ? Status::FailedPrecondition() : OkStatus();
}

Status WriteByte(std::byte b)
{
    auto ret = console_out->write(reinterpret_cast<const char *>(&b), 1);
    return ret != 1 ? Status::FailedPrecondition() : OkStatus();
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

    // Write trailing newline.
    result = WriteBytes(pw::as_bytes(pw::span("\r\n", 2)));
    chars_written += result.size();

    return StatusWithSize(result.status(), chars_written);
}

} // namespace pw::sys_io
