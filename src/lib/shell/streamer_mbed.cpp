/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of an input / output stream for stdio targets.
 */

#include <lib/shell/streamer.h>

#include <stdio.h>
#include <stdlib.h>

namespace chip {
namespace Shell {

#ifndef SHELL_STREAMER_APP_SPECIFIC

int streamer_stdio_init(streamer_t * streamer)
{
    return 0;
}

ssize_t streamer_stdio_read(streamer_t * streamer, char * buf, size_t len)
{
    return fread(buf, 1, len, stdin);
}

ssize_t streamer_stdio_write(streamer_t * streamer, const char * buf, size_t len)
{
    return fwrite(buf, 1, len, stdout);
}

static streamer_t streamer_stdio = {
    .init_cb  = streamer_stdio_init,
    .read_cb  = streamer_stdio_read,
    .write_cb = streamer_stdio_write,
};

streamer_t * streamer_get()
{
    return &streamer_stdio;
}

#endif //#ifndef SHELL_STREAMER_APP_SPECIFIC

} // namespace Shell
} // namespace chip
