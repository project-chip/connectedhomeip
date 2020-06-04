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
 *      Header that defines a generic shell API for CHIP examples
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct streamer;

typedef int streamer_init_fn(const struct streamer * streamer);
typedef int streamer_read_fn(const struct streamer * streamer, char * buf, size_t len);
typedef int streamer_write_fn(const struct streamer * streamer, const char * buf, size_t len);

struct streamer
{
    streamer_init_fn * init_cb;
    streamer_read_fn * read_cb;
    streamer_write_fn * write_cb;
};

int streamer_init(const struct streamer * streamer);
int streamer_read(const struct streamer * streamer, void * buf, size_t len);
int streamer_write(const struct streamer * streamer, const void * buf, size_t len);
int streamer_vprintf(const struct streamer * streamer, const char * fmt, va_list ap);
int streamer_printf(const struct streamer * streamer, const char * fmt, ...);
const struct streamer * streamer_get(void);

#ifdef __cplusplus
}
#endif
