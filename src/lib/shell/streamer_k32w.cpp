/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Source implementation of an input / output stream for k32w targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>

#include "app_config.h"
#include <stdio.h>
#include <string.h>

#include "SerialManager.h"
extern uint8_t mOtSerMgrIfLog;

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
    Serial_Read(mOtSerMgrIfLog, (uint8_t *) buffer, length, &bytesRead);

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
