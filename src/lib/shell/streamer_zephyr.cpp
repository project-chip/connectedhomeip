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
 *      Source implementation of an input / output stream for zehpyr targets.
 */

#include "shell.h"

#include <cassert>

#if CONFIG_SHELL
#include <shell/shell.h>
#include <shell/shell_uart.h>
#else
#include <console/console.h>
#endif

namespace chip {
namespace Shell {
namespace {

#if CONFIG_SHELL

// NOTE:
// We assume that if Zephyr shell is enabled it will be responsible for reading and parsing
// a command line. Therefore, we only have to provide implentation for streamer_write() and
// may omit streamer_read() part.

int streamer_zephyr_init(streamer_t * streamer)
{
    ARG_UNUSED(streamer);
    return 0;
}

int streamer_zephyr_read(streamer_t * streamer, char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    return 0;
}

int streamer_zephyr_write(streamer_t * streamer, const char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    for (size_t i = 0; i < length; ++i)
        // TODO: Don't assume that UART backend is used.
        shell_fprintf(shell_backend_uart_get_ptr(), SHELL_NORMAL, "%c", buffer[i]);
    return length;
}

#else // CONFIG_SHELL

int streamer_zephyr_init(streamer_t * streamer)
{
    ARG_UNUSED(streamer);
    return console_init();
}

int streamer_zephyr_read(streamer_t * streamer, char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    const ssize_t rc = console_read(nullptr, buffer, length);
    return rc >= 0 ? rc : 0;
}

int streamer_zephyr_write(streamer_t * streamer, const char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    size_t written = 0;

    while (written < length)
    {
        const ssize_t rc = console_write(nullptr, buffer + written, length - written);
        if (rc <= 0) // error
            break;
        written += rc;
    }

    return written;
}

#endif // CONFIG_SHELL

static streamer_t streamer_zephyr = {
    .init_cb  = streamer_zephyr_init,
    .read_cb  = streamer_zephyr_read,
    .write_cb = streamer_zephyr_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_zephyr;
}

} // namespace Shell
} // namespace chip
