/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Apache Software Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header that defines a generic input / output stream abstraction.
 */

#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

namespace chip {
namespace Shell {

struct streamer;
typedef const struct streamer streamer_t;

typedef int streamer_init_fn(streamer_t * self);
typedef ssize_t streamer_read_fn(streamer_t * self, char * buf, size_t len);
typedef ssize_t streamer_write_fn(streamer_t * self, const char * buf, size_t len);

struct streamer
{
    streamer_init_fn * init_cb;
    streamer_read_fn * read_cb;
    streamer_write_fn * write_cb;
};

int streamer_init(streamer_t * self);
ssize_t streamer_read(streamer_t * self, char * buf, size_t len);
ssize_t streamer_write(streamer_t * self, const char * buf, size_t len);
ssize_t streamer_vprintf(streamer_t * self, const char * fmt, va_list ap);
ssize_t streamer_printf(streamer_t * self, const char * fmt, ...);
void streamer_print_hex(streamer_t * self, const uint8_t * data, int len);
streamer_t * streamer_get();

} // namespace Shell
} // namespace chip
