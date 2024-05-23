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

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

namespace chip {
namespace Shell {

#ifndef SHELL_STREAMER_APP_SPECIFIC

static struct termios the_original_stdin_termios;

static void streamer_restore_termios()
{
    int in_fd = fileno(stdin);
    tcsetattr(in_fd, TCSAFLUSH, &the_original_stdin_termios);
}

int streamer_stdio_init(streamer_t * streamer)
{
    int ret   = 0;
    int in_fd = fileno(stdin);
    struct termios termios;

    if (isatty(in_fd))
    {
        ret = tcgetattr(in_fd, &the_original_stdin_termios);
        atexit(&streamer_restore_termios);

        ret = tcgetattr(in_fd, &termios);
        termios.c_lflag &= ~static_cast<tcflag_t>(ECHO);   // Disable echo mode
        termios.c_lflag &= ~static_cast<tcflag_t>(ICANON); // Disable canonical line editing mode
        ret = tcsetattr(in_fd, TCSANOW, &termios);
    }

    return ret;
}

ssize_t streamer_stdio_read(streamer_t * streamer, char * buf, size_t len)
{
    return read(STDIN_FILENO, buf, len);
}

ssize_t streamer_stdio_write(streamer_t * streamer, const char * buf, size_t len)
{
    return write(STDOUT_FILENO, buf, len);
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

#endif // #ifndef SHELL_STREAMER_APP_SPECIFIC

} // namespace Shell
} // namespace chip
