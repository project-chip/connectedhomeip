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
