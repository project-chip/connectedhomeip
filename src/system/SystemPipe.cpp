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

#include <system/SystemPipe.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

// Include additional CHIP headers
#include <support/CodeUtils.h>

// Include system and language headers
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#if !CONFIG_HAVE_PIPE
#include <sys/socket.h>
#endif

namespace chip {
namespace System {

namespace {
inline int SetNonBlockingMode(int fd)
{
    int flags = ::fcntl(fd, F_GETFL, 0);
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
} // anonymous namespace

void Pipe::Close()
{
    if (mFDs[FD_WRITE] > 0)
    {
        ::close(mFDs[FD_WRITE]);
        ::close(mFDs[FD_READ]);
        mFDs[FD_READ] = mFDs[FD_WRITE] = -1;
    }
}

#if CONFIG_HAVE_PIPE

Error Pipe::Open()
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

void Pipe::ClearContent()
{
    uint8_t buffer[128];

    while (::read(mFDs[FD_READ], buffer, sizeof(buffer)) == sizeof(buffer))
        continue;
}

void Pipe::WriteByte(const uint8_t byte)
{
    ::write(mFDs[FD_WRITE], &byte, 1);
}

#else // CONFIG_HAVE_PIPE

Error Pipe::Open()
{
    mFDs[FD_READ] = mFDs[FD_WRITE] = 0;

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, mFDs) < 0)
        return chip::System::MapErrorPOSIX(errno);

    if (SetNonBlockingMode(mFDs[FD_READ]) < 0)
        return chip::System::MapErrorPOSIX(errno);

    if (SetNonBlockingMode(mFDs[FD_WRITE]) < 0)
        return chip::System::MapErrorPOSIX(errno);

    return CHIP_SYSTEM_NO_ERROR;
}

void Pipe::ClearContent()
{
    uint8_t buffer[128];

    while (::recv(mFDs[FD_READ], buffer, sizeof(buffer), 0) > 0)
        continue;
}

void Pipe::WriteByte(const uint8_t byte)
{
    ::send(mFDs[FD_WRITE], &byte, 1, 0);
}

#endif // CONFIG_HAVE_PIPE

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS