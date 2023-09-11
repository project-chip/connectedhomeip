/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

int streamer_qpg_init(streamer_t * streamer)
{
    (void) streamer;
    uartConsoleInit();
    return 0;
}

ssize_t streamer_qpg_read(streamer_t * streamer, char * buffer, size_t length)
{
    (void) streamer;
    return (ssize_t) uartConsoleRead(buffer, (uint16_t) length);
}

ssize_t streamer_qpg_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    return uartConsoleWrite(buffer, (uint16_t) length);
}

static streamer_t streamer_qpg = {
    .init_cb  = streamer_qpg_init,
    .read_cb  = streamer_qpg_read,
    .write_cb = streamer_qpg_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_qpg;
}

} // namespace Shell
} // namespace chip
