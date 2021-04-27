/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_NET_FD_CONTROL_BLOCK_H
#define MBED_NET_FD_CONTROL_BLOCK_H

#include "platform/FileHandle.h"
#include "rtos/EventFlags.h"
#include <memory>

namespace mbed {

struct FdControlBlock
{
    FdControlBlock() = default;

    FdControlBlock(int index, fd_set * readfds, fd_set * writefds, fd_set * errorfds)
    {
        if (readfds && FD_ISSET(index, readfds))
        {
            handle = mbed_file_handle(index);
            fd     = index;
            read   = true;
        }

        if (writefds && FD_ISSET(index, writefds))
        {
            if (!handle)
            {
                handle = mbed_file_handle(index);
                fd     = index;
            }
            write = true;
        }

        if (writefds && FD_ISSET(index, errorfds))
        {
            if (!handle)
            {
                handle = mbed_file_handle(index);
                fd     = index;
            }
            err = false;
        }
    }

    short poll()
    {
        // Description of the poll function event is strange it may or may not
        // use the event list in input ...
        return handle->poll(events()) & events();
    }

    short events()
    {
        short e = 0;
        if (read)
        {
            e |= POLLIN;
        }
        if (write)
        {
            e |= POLLOUT;
        }
        if (err)
        {
            e |= POLLERR;
        }
        return e;
    }

    FileHandle * handle = nullptr;
    int fd              = 0xFFFFFFFF;
    bool read           = false;
    bool write          = false;
    bool err            = false;
};

} // namespace mbed

#endif // MBED_NET_FD_CONTROL_BLOCK_H
