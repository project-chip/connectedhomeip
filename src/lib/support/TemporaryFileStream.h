/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <cstdlib>
#include <ostream>
#include <streambuf>
#include <string>
#include <unistd.h>

#include <lib/support/FileDescriptor.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class FileDescriptorStreamBuf : public std::streambuf
{
public:
    FileDescriptorStreamBuf() = default;
    explicit FileDescriptorStreamBuf(int fd) : mFd(fd) {}

    FileDescriptorStreamBuf(FileDescriptorStreamBuf &)             = delete;
    FileDescriptorStreamBuf & operator=(FileDescriptorStreamBuf &) = delete;

    FileDescriptorStreamBuf(FileDescriptorStreamBuf && other)             = default;
    FileDescriptorStreamBuf & operator=(FileDescriptorStreamBuf && other) = default;

protected:
    int overflow(int c) override
    {
        if (c != EOF)
        {
            char z = c;
            if (write(mFd, &z, 1) != 1)
            {
                return EOF;
            }
        }
        return c;
    }

    std::streamsize xsputn(const char * s, std::streamsize n) override { return write(mFd, s, static_cast<size_t>(n)); }

private:
    int mFd = -1;
};

/// File stream for a temporary file compatible with std::ostream.
class TemporaryFileStream : public std::ostream
{
public:
    TemporaryFileStream() : std::ostream(&mBuf) {}
    explicit TemporaryFileStream(std::string nameTemplate) : std::ostream(&mBuf) { Open(std::move(nameTemplate)); };

    /// Disallow copy and assignment.
    TemporaryFileStream(const TemporaryFileStream &)             = delete;
    TemporaryFileStream & operator=(const TemporaryFileStream &) = delete;

    /// Open a temporary file with a given name template.
    ///
    /// In order to check if the file was opened successfully, use IsOpen().
    void Open(std::string nameTemplate)
    {
        mFileName = std::move(nameTemplate);
        mFd       = FileDescriptor(mkstemp(mFileName.data()));
        mBuf      = FileDescriptorStreamBuf(mFd.Get());
    }

    /// Check if the file was opened successfully.
    ///
    /// In case of failure, the error can be retrieved using errno.
    bool IsOpen() const { return mFd.Get() != -1; };

    /// Synchronize the file's contents with the underlying storage device.
    ///
    /// In case of failure, the error can be retrieved using errno.
    bool DataSync() { return fdatasync(mFd.Get()) == 0; }

    /// Get the name of created temporary file.
    const std::string & GetFileName() const { return mFileName; }

private:
    FileDescriptor mFd;
    FileDescriptorStreamBuf mBuf;
    std::string mFileName;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
