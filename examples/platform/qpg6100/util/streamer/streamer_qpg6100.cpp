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
 *      Source implementation of an input / output stream for QPG6100 targets.
 */

#include <lib/shell/shell.h>

#ifdef QPG6100_SHELL_STREAMER

#include "qvCHIP.h"

namespace chip {
namespace Shell {

int streamer_qpg6100_init(streamer_t * streamer)
{
    qvCHIP_UartInit();
    return 0;
}

int streamer_qpg6100_read(streamer_t * streamer, char * buf, size_t len)
{
    return qvCHIP_UartReadRxData(len, buf);
}

int streamer_qpg6100_write(streamer_t * streamer, const char * buf, size_t len)
{
    qvCHIP_UartTxData(len, buf);
    return len;
}

static streamer_t streamer_qpg6100 = {
    .init_cb  = streamer_qpg6100_init,
    .read_cb  = streamer_qpg6100_read,
    .write_cb = streamer_qpg6100_write,
};

streamer_t * streamer_get(void)
{
    return &streamer_qpg6100;
}

} // namespace Shell
} // namespace chip

#endif //#ifdef QPG6100_SHELL_STREAMER
