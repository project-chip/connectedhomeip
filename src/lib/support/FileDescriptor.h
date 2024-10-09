/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <unistd.h>
#include <utility>

namespace chip {

/// Unix file descriptor wrapper with RAII semantics.
class FileDescriptor
{
public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd) : mFd(fd) {}
    ~FileDescriptor() { Close(); }

    /// Disallow copy and assignment.
    FileDescriptor(const FileDescriptor &)             = delete;
    FileDescriptor & operator=(const FileDescriptor &) = delete;

    FileDescriptor(FileDescriptor && other) noexcept : mFd(other.Release()) {}
    FileDescriptor & operator=(FileDescriptor && other) noexcept
    {
        Close();
        mFd = other.Release();
        return *this;
    }

    int Get() const { return mFd; }

    int Release() { return std::exchange(mFd, -1); }

    int Close()
    {
        if (mFd != -1)
        {
            return close(std::exchange(mFd, -1));
        }
        return 0;
    }

private:
    int mFd = -1;
};

} // namespace chip
