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

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <io_def.h>

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Shell {
namespace {

int streamer_mt793x_init(streamer_t * streamer)
{
    (void) streamer;
    return 0;
}

ssize_t streamer_mt793x_read(streamer_t * streamer, char * buffer, size_t length)
{
    int i = 0;
    (void) streamer;

    while (i < length)
        buffer[i++] = (char) bsp_io_def_uart_getchar();
    return length;
}

ssize_t streamer_mt793x_write(streamer_t * streamer, const char * buffer, size_t length)
{
    int i = 0;
    (void) streamer;
    while (i < length)
        bsp_io_def_uart_putchar(buffer[i++]);
    return length;
}

static streamer_t streamer_mt793x = {
    .init_cb  = streamer_mt793x_init,
    .read_cb  = streamer_mt793x_read,
    .write_cb = streamer_mt793x_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_mt793x;
}

} // namespace Shell
} // namespace chip
