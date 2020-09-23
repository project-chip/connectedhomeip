/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Apache Software Foundation (ASF)
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
 *      Source implementation for an input / output stream abstraction.
 */

#include "streamer.h"

#include <stdio.h>
#include <support/logging/CHIPLogging.h>

#ifndef CONSOLE_DEFAULT_MAX_LINE
#define CONSOLE_DEFAULT_MAX_LINE 256
#endif

namespace chip {
namespace Shell {

int streamer_init(streamer_t * self)
{
    return self->init_cb(self);
}

int streamer_read(streamer_t * self, char * buf, size_t len)
{
    return self->read_cb(self, buf, len);
}

int streamer_write(streamer_t * self, const char * buf, size_t len)
{
    return self->write_cb(self, buf, len);
}

int streamer_vprintf(streamer_t * self, const char * fmt, va_list ap)
{
    char buf[CONSOLE_DEFAULT_MAX_LINE];
    unsigned len;

    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (len >= sizeof(buf))
    {
        len = sizeof(buf) - 1;
    }
    return streamer_write(self, buf, len);
}

int streamer_printf(streamer_t * self, const char * fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = streamer_vprintf(self, fmt, ap);
    va_end(ap);

    return rc;
}

void streamer_print_hex(streamer_t * self, const uint8_t * bytes, int len)
{
    for (int i = 0; i < len; i++)
    {
        streamer_printf(streamer_get(), "%02x", bytes[i]);
    }
}

} // namespace Shell
} // namespace chip
