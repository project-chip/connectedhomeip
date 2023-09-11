/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of an input / output stream for Open IoT SDK platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

#include <lib/shell/streamer.h>

namespace chip {
namespace Shell {

int streamer_openiotsdk_init(streamer_t * streamer)
{
    (void) streamer;
    return 0;
}

ssize_t streamer_openiotsdk_read(streamer_t * streamer, char * buf, size_t len)
{
    (void) streamer;
    return read(STDIN_FILENO, buf, len);
}

ssize_t streamer_openiotsdk_write(streamer_t * streamer, const char * buf, size_t len)
{
    (void) streamer;
    return write(STDOUT_FILENO, buf, len);
}

static streamer_t streamer_openiotsdk = {
    .init_cb  = streamer_openiotsdk_init,
    .read_cb  = streamer_openiotsdk_read,
    .write_cb = streamer_openiotsdk_write,
};

streamer_t * streamer_get()
{
    return &streamer_openiotsdk;
}

} // namespace Shell
} // namespace chip
