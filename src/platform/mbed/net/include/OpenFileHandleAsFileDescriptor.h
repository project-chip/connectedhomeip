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

#ifndef MBED_NET_OPEN_FILE_HANDLE_AS_FILE_DESCRIPTOR_H
#define MBED_NET_OPEN_FILE_HANDLE_AS_FILE_DESCRIPTOR_H

#include "platform/FileHandle.h"
#include <utility>

namespace mbed {

/**
 * Open a FileHandle as a file descriptor.
 *
 * The FileHandle is bound to a file descriptor from mbed-os. Closing the file
 * descriptor using close release the file handle too.
 *
 * @tparam FH Type of the file handle
 * @tparam Args Type of the FileHandle constructor arguments
 * @param args arguments to pass to the FH constructor
 * @return If positive, the file descriptor created otherwise an appropriate error.
 */
template <typename FH, typename... Args>
int open_fh_as_fd(Args &&... args)
{
    struct ReleaseOnCloseFileHandle : FH
    {
        ReleaseOnCloseFileHandle(Args &&... args) : FH(std::forward<Args &&>(args)...) {}

        int close() override
        {
            auto result = FH::close();
            if (result == 0)
            {
                delete this;
            }
            return result;
        }
    };

    auto * fh = new (std::nothrow) ReleaseOnCloseFileHandle(std::forward<Args &&>(args)...);
    if (!fh)
    {
        return -1;
    }
    auto fd = mbed::bind_to_fd(fh);
    if (fd < 0)
    {
        delete fh;
    }
    return fd;
}

} // namespace mbed

#endif // MBED_NET_OPEN_FILE_HANDLE_AS_FILE_DESCRIPTOR_H
