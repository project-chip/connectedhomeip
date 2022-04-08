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

#include <lib/core/CHIPSafeCasts.h>
#include <lib/shell/streamer.h>
#include <platform/PlatformManager.h>
#include <wiced_hal_puart.h>
#include <wiced_platform.h>

namespace chip {
namespace Shell {

void ProcessInput(intptr_t args);

static int streamer_cyw30739_init(streamer_t * streamer);
static ssize_t streamer_cyw30739_read(streamer_t * streamer, char * buf, size_t len);
static ssize_t streamer_cyw30739_write(streamer_t * streamer, const char * buf, size_t len);
static void streamer_cyw30739_uart_rx_handler(void * arg);

static streamer_t streamer_stdio = {
    .init_cb  = streamer_cyw30739_init,
    .read_cb  = streamer_cyw30739_read,
    .write_cb = streamer_cyw30739_write,
};

streamer_t * streamer_get()
{
    return &streamer_stdio;
}

int streamer_cyw30739_init(streamer_t * streamer)
{
    wiced_platform_puart_init(streamer_cyw30739_uart_rx_handler);
    return 0;
}

ssize_t streamer_cyw30739_read(streamer_t * streamer, char * buf, size_t len)
{
    size_t count;
    for (count = 0; count < len; count++)
    {
        if (!wiced_hal_puart_read(Uint8::from_char(buf) + count))
            break;
    }
    wiced_hal_puart_reset_puart_interrupt();
    return count;
}

ssize_t streamer_cyw30739_write(streamer_t * streamer, const char * buf, size_t len)
{
    wiced_hal_puart_print(const_cast<char *>(buf));
    return 0;
}

void streamer_cyw30739_uart_rx_handler(void * arg)
{
    DeviceLayer::PlatformMgr().LockChipStack();
    DeviceLayer::PlatformMgr().ScheduleWork(ProcessInput, 0);
    DeviceLayer::PlatformMgr().UnlockChipStack();
}

} // namespace Shell
} // namespace chip
