/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cinttypes>

#include "pw_sys_io/sys_io.h"
#include "zephyr/console/console.h"
#include <cassert>
#include <zephyr/kernel.h>

#ifdef CONFIG_USB_DEVICE_STACK
#include <zephyr/usb/usb_device.h>

static bool output_enable = false;

static void usb_dc_status_change(enum usb_dc_status_code cb_status, const uint8_t * param)
{
    (void) param;
    if (cb_status == USB_DC_CONFIGURED)
    {
        output_enable = true;
    }
}
#endif

extern "C" void pw_sys_io_Init()
{
    int err;

#ifdef CONFIG_USB_DEVICE_STACK
    output_enable = false;
    (void) usb_enable(usb_dc_status_change);
#endif

    err = console_init();
    assert(err == 0);
}

namespace pw::sys_io {

Status ReadByte(std::byte * dest)
{
    if (!dest)
        return Status::InvalidArgument();

    const int c = console_getchar();
    *dest       = static_cast<std::byte>(c);

    return c < 0 ? Status::FailedPrecondition() : OkStatus();
}

Status WriteByte(std::byte b)
{
#ifdef CONFIG_USB_DEVICE_STACK
    if (!output_enable)
        return Status::FailedPrecondition();
#endif
    return console_putchar(static_cast<char>(b)) < 0 ? Status::FailedPrecondition() : OkStatus();
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
