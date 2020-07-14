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
 *      This file declares the abstraction of one-directional, anonymous
 *      data stream built on top of two file descriptors.
 */

#include <system/SystemWakeEvent.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

// Include additional CHIP headers
#include <support/CodeUtils.h>

// Include system and language headers
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#if !CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
#include <sys/eventfd.h>
#endif

namespace chip {
namespace System {

#if CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE

namespace {
inline int SetNonBlockingMode(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
} // anonymous namespace

Error SystemWakeEvent::Open()
{
    mFDs[FD_READ] = mFDs[FD_WRITE] = -1;

    if (::pipe(mFDs) < 0)
        return chip::System::MapErrorPOSIX(errno);

    if (SetNonBlockingMode(mFDs[FD_READ]) < 0)
        return chip::System::MapErrorPOSIX(errno);

    if (SetNonBlockingMode(mFDs[FD_WRITE]) < 0)
        return chip::System::MapErrorPOSIX(errno);

    return CHIP_SYSTEM_NO_ERROR;
}

void SystemWakeEvent::Close()
{
    ::close(mFDs[FD_WRITE]);
    ::close(mFDs[FD_READ]);
    mFDs[FD_READ] = mFDs[FD_WRITE] = -1;
}

void SystemWakeEvent::Confirm()
{
    uint8_t buffer[128];

    while (::read(mFDs[FD_READ], buffer, sizeof(buffer)) == sizeof(buffer))
        continue;
}

void SystemWakeEvent::Notify()
{
    char byte = 1;
    ::write(mFDs[FD_WRITE], &byte, 1);
}

#else // CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE

Error SystemWakeEvent::Open()
{
    mFD = eventfd(0, EFD_NONBLOCK);

    if (mFD == -1)
    {
        return chip::System::MapErrorPOSIX(errno);
    }

    return CHIP_SYSTEM_NO_ERROR;
}

void SystemWakeEvent::Close()
{
    ::close(mFD);
    mFD = -1;
}

void SystemWakeEvent::Confirm()
{
    uint64_t value;
    ::read(mFD, &value, sizeof(value));
}

void SystemWakeEvent::Notify()
{
    uint64_t value = 1;
    ::write(mFD, &value, sizeof(value));
}

#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
