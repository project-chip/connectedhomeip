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
 *      Source implementation of an input / output stream for stdio targets.
 */

#include <lib/shell/streamer.h>

#include <signal.h>
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
