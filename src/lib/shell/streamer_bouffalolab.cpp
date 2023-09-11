/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of an input / output stream for bl702 targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <stdio.h>
#include <string.h>
#include <uart.h>

namespace chip {
namespace Shell {
namespace {

int streamer_bl702_init(streamer_t * streamer)
{
    (void) streamer;
    // uartInit();
    return 0;
}

ssize_t streamer_bl702_read(streamer_t * streamer, char * buffer, size_t length)
{
    (void) streamer;
    return (ssize_t) uartRead(buffer, (uint16_t) length);
}

ssize_t streamer_bl702_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    return uartWrite(buffer, (uint16_t) length);
}

static streamer_t streamer_bl702 = {
    .init_cb  = streamer_bl702_init,
    .read_cb  = streamer_bl702_read,
    .write_cb = streamer_bl702_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_bl702;
}

} // namespace Shell
} // namespace chip
