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

#include <lib/shell/streamer.h>

#include <cassert>

#include <shell/shell.h>
#include <shell/shell_uart.h>

namespace chip {
namespace Shell {
namespace {

int streamer_zephyr_init(streamer_t * streamer)
{
    ARG_UNUSED(streamer);
    return 0;
}

ssize_t streamer_zephyr_read(streamer_t * streamer, char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    return 0;
}

ssize_t streamer_zephyr_write(streamer_t * streamer, const char * buffer, size_t length)
{
    ARG_UNUSED(streamer);
    for (size_t i = 0; i < length; ++i)
        // TODO: Don't assume that UART backend is used.
        shell_fprintf(shell_backend_uart_get_ptr(), SHELL_NORMAL, "%c", buffer[i]);
    return length;
}

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
