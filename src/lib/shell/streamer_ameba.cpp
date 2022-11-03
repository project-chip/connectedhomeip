/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of an input / output stream for zehpyr targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <stdio.h>
#include <string.h>

#include "serial_api.h"

#if defined(CONFIG_PLATFORM_8721D)
#define UART_TX PA_18 // UART0  TX
#define UART_RX PA_19 // UART0  RX
#elif defined(CONFIG_PLATFORM_8710C)
#define UART_TX PA_14 // UART0  TX
#define UART_RX PA_13 // UART0  RX
#endif

namespace chip {
namespace Shell {
namespace {

serial_t sobj;

int streamer_ameba_init(streamer_t * streamer)
{
    (void) streamer;
    serial_init(&sobj, UART_TX, UART_RX);
    serial_baud(&sobj, 115200);
    serial_format(&sobj, 8, ParityNone, 1);
    return 0;
}

ssize_t streamer_ameba_read(streamer_t * streamer, char * buffer, size_t length)
{
    (void) streamer;
    uint16_t len_read = 0;

    while (len_read < length)
    {
        *(buffer + len_read) = (char) serial_getc(&sobj);
        len_read++;
    }
    return len_read;
}

ssize_t streamer_ameba_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    uint16_t len_written = 0;

    while (len_written < length)
    {
        serial_putc(&sobj, *(buffer + len_written));
        len_written++;
    }
    return len_written;
}

static streamer_t streamer_ameba = {
    .init_cb  = streamer_ameba_init,
    .read_cb  = streamer_ameba_read,
    .write_cb = streamer_ameba_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_ameba;
}

} // namespace Shell
} // namespace chip
