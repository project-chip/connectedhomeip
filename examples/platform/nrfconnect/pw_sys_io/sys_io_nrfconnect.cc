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

#include "pw_span/span.h"
#include "pw_sys_io/sys_io.h"
#include <cassert>
#include <zephyr/console/console.h>
#include <zephyr/kernel.h>

// #if defined(CONFIG_USB_DEVICE_STACK) && defined(CONFIG_USB_CDC_ACM)
// #include <zephyr/drivers/uart.h>
// #include <zephyr/usb/usb_device.h>
// #endif

#ifdef CONFIG_USB
#include <zephyr/usb/usb_device.h>
#endif

namespace {

constexpr size_t kConsoleWriteThreadStackSize = 2048;
constexpr int kConsoleWriteThreadPriority     = 5;
constexpr size_t kConsoleWriteMsgqSize        = 1024;

K_THREAD_STACK_DEFINE(sConsoleWriteStack, kConsoleWriteThreadStackSize);
k_thread sConsoleWriteThread;

K_MSGQ_DEFINE(sConsoleWriteMsgq, sizeof(char), kConsoleWriteMsgqSize, 1);

void ConsoleWriteThreadMain(void *, void *, void *)
{
    char tx_char;

    while (true)
    {
        // Wait indefinitely for a character to be available in the message queue.
        if (k_msgq_get(&sConsoleWriteMsgq, &tx_char, K_FOREVER) == 0)
        {
            // This call will block if the USB host is not connected or not reading.
            // This is acceptable because we are in a dedicated writer thread, so other
            // application threads will not be affected.
            console_putchar(tx_char);
        }
    }
}

} // namespace

extern "C" void pw_sys_io_Init()
{
    int err;

#ifdef CONFIG_USB
    err = usb_enable(nullptr);
    assert(err == 0 || err == (-EALREADY));
#endif

    err = console_init();
    assert(err == 0);
    k_thread_create(&sConsoleWriteThread, sConsoleWriteStack, K_THREAD_STACK_SIZEOF(sConsoleWriteStack), ConsoleWriteThreadMain,
                    nullptr, nullptr, nullptr, kConsoleWriteThreadPriority, 0, K_NO_WAIT);
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
    (void) b;
    char c = static_cast<char>(b);

    // k_msgq_put with K_NO_WAIT will return -ENOMSG if the queue is full.
    // This makes the write non-blocking and lossy if the consumer (console)
    // is too slow or not connected, which is the desired behavior for logs.
    k_msgq_put(&sConsoleWriteMsgq, &c, K_NO_WAIT);
    return OkStatus();
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

    // Write trailing newline.
    result = WriteBytes(pw::as_bytes(pw::span("\r\n", 2)));
    chars_written += result.size();

    return StatusWithSize(result.status(), chars_written);
}

} // namespace pw::sys_io
