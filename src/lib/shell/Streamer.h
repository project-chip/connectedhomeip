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
 *      Header that defines a generic input / output stream abstraction.
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace Shell {

struct streamer;
typedef const struct streamer streamer_t;

typedef int streamer_init_fn(streamer_t * self);
typedef int streamer_read_fn(streamer_t * self, char * buf, size_t len);
typedef int streamer_write_fn(streamer_t * self, const char * buf, size_t len);

struct streamer
{
    streamer_init_fn * init_cb;
    streamer_read_fn * read_cb;
    streamer_write_fn * write_cb;
};

int streamer_init(streamer_t * self);
int streamer_read(streamer_t * self, char * buf, size_t len);
int streamer_write(streamer_t * self, const char * buf, size_t len);
int streamer_vprintf(streamer_t * self, const char * fmt, va_list ap);
int streamer_printf(streamer_t * self, const char * fmt, ...);
void streamer_print_hex(streamer_t * self, const uint8_t * data, int len);
streamer_t * streamer_get(void);

} // namespace Shell
} // namespace chip
