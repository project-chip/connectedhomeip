/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "NamedPipeCommands.h"

#include <chrono>
#include <errno.h>
#include <fcntl.h>
#include <lib/support/CodeUtils.h>
#include <poll.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

static constexpr const size_t kChipEventCmdBufSize = 256;

CHIP_ERROR NamedPipeCommands::Start(const std::string & path, NamedPipeCommandDelegate * delegate)
{
    VerifyOrReturnError(!mStarted, CHIP_NO_ERROR);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStarted              = true;
    mDelegate             = delegate;
    mChipEventFifoPath    = path;
    mChipEventFifoPathOut = "";

    // Creating the named file(FIFO)
    VerifyOrReturnError((mkfifo(path.c_str(), 0666) == 0) || (errno == EEXIST), CHIP_ERROR_OPEN_FAILED);
    VerifyOrReturnError(
        pthread_create(&mChipEventCommandListener, nullptr, EventCommandListenerTask, reinterpret_cast<void *>(this)) == 0,
        CHIP_ERROR_UNEXPECTED_EVENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NamedPipeCommands::Start(const std::string & path, const std::string & path_out, NamedPipeCommandDelegate * delegate)
{
    VerifyOrReturnError(!mStarted, CHIP_NO_ERROR);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStarted              = true;
    mDelegate             = delegate;
    mChipEventFifoPath    = path;
    mChipEventFifoPathOut = path_out;

    // Creating the named file(FIFO)
    VerifyOrReturnError((mkfifo(path.c_str(), 0666) == 0) || (errno == EEXIST), CHIP_ERROR_OPEN_FAILED);
    VerifyOrReturnError(
        pthread_create(&mChipEventCommandListener, nullptr, EventCommandListenerTask, reinterpret_cast<void *>(this)) == 0,
        CHIP_ERROR_UNEXPECTED_EVENT);

    VerifyOrReturnError((mkfifo(path_out.c_str(), 0666) == 0) || (errno == EEXIST), CHIP_ERROR_OPEN_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR NamedPipeCommands::Stop()
{
    VerifyOrReturnError(mStarted, CHIP_NO_ERROR);

    mStarted  = false;
    mDelegate = nullptr;

    VerifyOrReturnError(pthread_cancel(mChipEventCommandListener) == 0, CHIP_ERROR_CANCELLED);

    // Wait further for the thread to terminate if we had previously created it.
    VerifyOrReturnError(pthread_join(mChipEventCommandListener, nullptr) == 0, CHIP_ERROR_SHUT_DOWN);

    VerifyOrReturnError(unlink(mChipEventFifoPath.c_str()) == 0, CHIP_ERROR_WRITE_FAILED);
    mChipEventFifoPath.clear();

    if (!mChipEventFifoPathOut.empty())
    {
        VerifyOrReturnError(unlink(mChipEventFifoPathOut.c_str()) == 0, CHIP_ERROR_WRITE_FAILED);
        mChipEventFifoPathOut.clear();
    }

    return CHIP_NO_ERROR;
}

void NamedPipeCommands::WriteToOutPipe(const std::string & json)
{
    // Opening a FIFO for write blocks until a reader connects.
    // Use non-blocking open with a bounded retry to avoid deadlocking the app
    // if the test side has not opened the pipe yet.
    constexpr int kOpenTimeoutMs = 2000;
    constexpr int kRetrySleepMs  = 50;

    int fd           = -1;
    const auto start = std::chrono::steady_clock::now();

    while (true)
    {
        fd = open(mChipEventFifoPathOut.c_str(), O_WRONLY | O_NONBLOCK);
        if (fd >= 0)
        {
            break;
        }

        if (errno != ENXIO) // ENXIO == no reader
        {
            ChipLogError(NotSpecified, "Failed to open out FIFO '%s': errno=%d", mChipEventFifoPathOut.c_str(), errno);
            return;
        }

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        if (elapsed >= kOpenTimeoutMs)
        {
            ChipLogError(NotSpecified, "Timed out waiting for reader on out FIFO '%s'", mChipEventFifoPathOut.c_str());
            return;
        }

        usleep(kRetrySleepMs * 1000);
    }

    ssize_t written = write(fd, json.data(), json.size());
    if (written < 0 || static_cast<size_t>(written) != json.size())
    {
        ChipLogError(DeviceLayer, "Failed to write full JSON to out pipe");
    }

    written = write(fd, "\n", 1);
    if (written != 1)
    {
        ChipLogError(DeviceLayer, "Failed to write newline to out pipe");
    }

    close(fd);
}

void * NamedPipeCommands::EventCommandListenerTask(void * arg)
{
    char readbuf[kChipEventCmdBufSize];

    NamedPipeCommands * self = reinterpret_cast<NamedPipeCommands *>(arg);

    for (;;)
    {
        int fd = open(self->mChipEventFifoPath.c_str(), O_RDONLY);
        if (fd == -1)
        {
            ChipLogError(NotSpecified, "Failed to open Event FIFO");
            break;
        }

        ssize_t readBytes = read(fd, readbuf, kChipEventCmdBufSize);
        if (readBytes > 0)
        {
            readbuf[readBytes - 1] = '\0';
            ChipLogProgress(NotSpecified, "Received payload: \"%s\"", readbuf);

            // Process the received command request from event fifo
            self->mDelegate->OnEventCommandReceived(readbuf);
        }

        close(fd);
    }

    return nullptr;
}
