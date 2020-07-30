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
 *      Source implementation of an input / output stream for stdio targets.
 */

#include "shell.h"

namespace chip {
namespace Shell {

#ifdef NRF_SHELL_STREAMER

int streamer_nrf5_init(streamer_t * streamer)
{
    int ret = 0;
    return ret;
}

int streamer_nrf5_read(streamer_t * streamer, char * buf, size_t len)
{
    return 0;
}

int streamer_nrf5_write(streamer_t * streamer, const char * buf, size_t len)
{
    return len;
}

static streamer_t streamer_nrf5 = {
    .init_cb  = streamer_nrf5_init,
    .read_cb  = streamer_nrf5_read,
    .write_cb = streamer_nrf5_write,
};

streamer_t * streamer_get(void)
{
    return &streamer_nrf5;
}

#endif //#ifdef NRF_SHELL_STREAMER

} // namespace Shell
} // namespace chip
