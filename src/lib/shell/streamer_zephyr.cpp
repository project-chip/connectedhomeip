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

#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>

namespace chip {
namespace Shell {
namespace {
const shell * sShellInstance;

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

    if (sShellInstance)
        shell_fprintf(sShellInstance, SHELL_NORMAL, "%.*s", length, buffer);

    return length;
}

streamer_t sStreamer = {
    .init_cb  = streamer_zephyr_init,
    .read_cb  = streamer_zephyr_read,
    .write_cb = streamer_zephyr_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &sStreamer;
}

void streamer_set_shell(const shell * shellInstance)
{
    sShellInstance = shellInstance;
}

} // namespace Shell
} // namespace chip
