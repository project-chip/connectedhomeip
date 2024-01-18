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
 *      Source implementation of an input / output stream for zehpyr targets.
 */

#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>
#include <platform/CHIPDeviceLayer.h>

#include <stdio.h>
#include <string.h>

extern QueueHandle_t shell_queue;

namespace chip {
namespace Shell {
namespace {

int streamer_ameba_init(streamer_t * streamer)
{
    // freertos queue should be initialized in ameba sdk
    if (shell_queue == NULL)
    {
        // queue not initialized
        return -1;
    }

    return 0;
}

ssize_t streamer_ameba_read(streamer_t * streamer, char * buffer, size_t length)
{
    BaseType_t lineReceived = xQueueReceive(shell_queue, buffer, pdMS_TO_TICKS(10));
    if (lineReceived == pdTRUE)
    {
        return length;
    }

    return 0;
}

ssize_t streamer_ameba_write(streamer_t * streamer, const char * buffer, size_t length)
{
    (void) streamer;
    printf("[shell] %s\r\n", buffer);
    return length;
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
