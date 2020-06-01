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
 *      Source implementation for a output stream abstraction.
 */

#include <stdio.h>

#include "shell.h"

#ifndef CONSOLE_DEFAULT_MAX_LINE
#define CONSOLE_DEFAULT_MAX_LINE 80
#endif

int streamer_init(const struct streamer * streamer)
{
    return streamer->init_cb(streamer);
}

int streamer_read(const struct streamer * streamer, void * buf, size_t len)
{
    return streamer->read_cb(streamer, buf, len);
}

int streamer_write(const struct streamer * streamer, const void * buf, size_t len)
{
    return streamer->write_cb(streamer, buf, len);
}

int streamer_vprintf(const struct streamer * streamer, const char * fmt, va_list ap)
{
    char buf[CONSOLE_DEFAULT_MAX_LINE];
    int len;

    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (len >= sizeof(buf))
    {
        len = sizeof(buf) - 1;
    }
    return streamer_write(streamer, buf, len);
}

int streamer_printf(const struct streamer * streamer, const char * fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = streamer_vprintf(streamer, fmt, ap);
    va_end(ap);

    return rc;
}