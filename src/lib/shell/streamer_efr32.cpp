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

#include "uart.h"
#include <stdio.h>
#include <string.h>

namespace chip {
namespace Shell {
namespace {

int streamer_efr_init(streamer_t * streamer)
{
    (void) streamer;
    uartConsoleInit();
    return 0;
}

ssize_t streamer_efr_read(streamer_t * streamer, char * buffer, size_t length)
{
    (void) streamer;
    return (ssize_t) uartConsoleRead(buffer, (uint16_t) length);
}

ssize_t streamer_efr_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    return uartConsoleWrite(buffer, (uint16_t) length);
}

static streamer_t streamer_efr = {
    .init_cb  = streamer_efr_init,
    .read_cb  = streamer_efr_read,
    .write_cb = streamer_efr_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_efr;
}

} // namespace Shell
} // namespace chip
