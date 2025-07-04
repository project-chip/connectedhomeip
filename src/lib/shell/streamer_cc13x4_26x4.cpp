/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      Source implementation of an input / output stream for cc13xx_26xx targets
 */

#include "streamer.h"
#include "ti_drivers_config.h"
#include <lib/shell/Engine.h>
#include <lib/shell/streamer.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ti/cc13xx_26xx/Logging.h>

#include <ti/drivers/UART2.h>

namespace chip {
namespace Shell {

#ifndef SHELL_STREAMER_APP_SPECIFIC
int streamer_cc13xx_26xx_init(streamer_t * streamer)
{
    uartConsoleInit();
    return 0;
}

ssize_t streamer_cc13xx_26xx_read(streamer_t * streamer, char * buf, size_t len)
{
    (void) streamer;
    size_t ret;

    ret = uartConsoleRead(buf, len);

    return ret;
}

ssize_t streamer_cc13xx_26xx_write(streamer_t * streamer, const char * buf, size_t len)
{
    (void) streamer;
    return uartConsoleWrite(buf, len);
}

static streamer_t streamer_cc13xx_26xx = {
    .init_cb  = streamer_cc13xx_26xx_init,
    .read_cb  = streamer_cc13xx_26xx_read,
    .write_cb = streamer_cc13xx_26xx_write,
};

streamer_t * streamer_get()
{
    return &streamer_cc13xx_26xx;
}

#endif // #ifndef SHELL_STREAMER_APP_SPECIFIC

} // namespace Shell
} // namespace chip
