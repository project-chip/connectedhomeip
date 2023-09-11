/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Source implementation of an input / output stream for k32w targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include <stdio.h>
#include <string.h>

#include "SerialManager.h"
extern uint8_t gShellSerMgrIf;

namespace chip {
namespace Shell {
namespace {
extern "C" void K32WWriteBlocking(const uint8_t * aBuf, uint32_t len);
extern "C" serialStatus_t Serial_Read(uint8_t InterfaceId, uint8_t * pData, uint16_t dataSize, uint16_t * bytesRead);

int streamer_k32w_init(streamer_t * streamer)
{
    (void) streamer;

    return 0;
}

ssize_t streamer_k32w_read(streamer_t * streamer, char * buffer, size_t length)
{
    uint16_t bytesRead = 0;

    (void) streamer;
    Serial_Read(gShellSerMgrIf, (uint8_t *) buffer, length, &bytesRead);

    return bytesRead;
}

ssize_t streamer_k32w_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    K32WWriteBlocking((uint8_t *) buffer, length);

    return length;
}

static streamer_t streamer_k32w = {
    .init_cb  = streamer_k32w_init,
    .read_cb  = streamer_k32w_read,
    .write_cb = streamer_k32w_write,
};
} // namespace

streamer_t * streamer_get(void)
{
    return &streamer_k32w;
}

} // namespace Shell
} // namespace chip
