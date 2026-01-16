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
<<<<<<< HEAD
#include <lib/support/logging/CHIPLogging.h>
#include <limits.h>
=======
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include <string>

namespace {

<<<<<<< HEAD
// Can receive at most how much the pipe infra on the platform supports (PIPE_BUF).
constexpr size_t kChipEventCmdBufSize = PIPE_BUF + 1;

} // namespace

CHIP_ERROR NamedPipeCommands::Start(const std::string & inPath, const std::string & outPath, NamedPipeCommandDelegate * delegate)
=======
CHIP_ERROR NamedPipeCommands::Start(const std::string & path, NamedPipeCommandDelegate * delegate)
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mDone, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mRunning.exchange(true), CHIP_ERROR_INCORRECT_STATE);

<<<<<<< HEAD
    CHIP_ERROR err = CHIP_NO_ERROR;
=======
    mStarted              = true;
    mDelegate             = delegate;
    mChipEventFifoPath    = path;
    mChipEventFifoPathOut = "";
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))

    mDelegate    = delegate;
    mFifoInPath  = inPath;
    mFifoOutPath = outPath;

    // 1. Creating the FIFO.
    VerifyOrExit((mkfifo(inPath.c_str(), 0660) == 0) || (errno == EEXIST), err = CHIP_ERROR_OPEN_FAILED);

    if (!outPath.empty())
    {
        VerifyOrExit((mkfifo(outPath.c_str(), 0660) == 0) || (errno == EEXIST), err = CHIP_ERROR_OPEN_FAILED);
    }

    // 2. Spawn listener thread last to avoid partial init and simplify failure cleanup.
    VerifyOrExit(pthread_create(&mChipEventCommandListener, nullptr, EventCommandListenerTask, reinterpret_cast<void *>(this)) == 0,
                 err = CHIP_ERROR_INTERNAL);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mRunning = false;
        Unlink();
    }
    return err;
}

CHIP_ERROR NamedPipeCommands::Start(const std::string & inPath, NamedPipeCommandDelegate * delegate)
{
    return Start(inPath, /*outPath=*/"", delegate);
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
    if (mRunning.exchange(false))
    {
        mDone = true;

        // Ignore SIGPIPE to prevent process termination if the listener thread closes the read end concurrently.
        signal(SIGPIPE, SIG_IGN);

        // Unblock the listener thread by writing a placeholder byte to the FIFO.
        int fd = open(mFifoInPath.c_str(), O_WRONLY | O_NONBLOCK);
        if (fd != -1)
        {
            char placeholder = '\0';
            if (write(fd, &placeholder, 1) != 1)
            {
                ChipLogError(NotSpecified, "Failed to write placeholder byte to unblock listener");
            }
            close(fd);
        }

        // Prevent deadlock: do not pthread_join if Stop() is called from the listener thread itself. This should
        // never happen, but could happen if the caller is the delegate itself by mistake trying to stop handling.
        if (pthread_equal(pthread_self(), mChipEventCommandListener) == 0)
        {
            // Wait further for the thread to terminate if we had previously created it.
            if (pthread_join(mChipEventCommandListener, nullptr) != 0)
            {
                ChipLogError(NotSpecified, "Failed to join listener thread");
            }
        }
        else
        {
            ChipLogProgress(NotSpecified, "NamedPipeCommands::Stop() called from listener thread; detaching thread.");
            pthread_detach(mChipEventCommandListener);
        }
    }

    mDelegate = nullptr;
    Unlink();

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
<<<<<<< HEAD
        fd = open(mFifoOutPath.c_str(), O_WRONLY | O_NONBLOCK);
=======
        fd = open(mChipEventFifoPathOut.c_str(), O_WRONLY | O_NONBLOCK);
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
        if (fd >= 0)
        {
            break;
        }

        if (errno != ENXIO) // ENXIO == no reader
        {
<<<<<<< HEAD
            ChipLogError(NotSpecified, "Failed to open out FIFO '%s': errno=%d", mFifoOutPath.c_str(), errno);
=======
            ChipLogError(NotSpecified, "Failed to open out FIFO '%s': errno=%d", mChipEventFifoPathOut.c_str(), errno);
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
            return;
        }

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        if (elapsed >= kOpenTimeoutMs)
        {
<<<<<<< HEAD
            ChipLogError(NotSpecified, "Timed out waiting for reader on out FIFO '%s'", mFifoOutPath.c_str());
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kRetrySleepMs));
    }

    std::string payload = json + "\n";
    ssize_t written     = write(fd, payload.data(), payload.size());
    if (written < 0)
    {
        ChipLogError(DeviceLayer, "Failed to write JSON payload to out pipe: errno=%d", errno);
    }
    else if (static_cast<size_t>(written) != payload.size())
    {
        ChipLogError(DeviceLayer,
                     "Failed to write full JSON payload to out pipe: %u written out of %u. Receiver will see truncated message.",
                     static_cast<unsigned>(written), static_cast<unsigned>(payload.size()));
=======
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
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
    }

    close(fd);
}

<<<<<<< HEAD
void NamedPipeCommands::Unlink()
{
    if (!mFifoInPath.empty())
    {
        unlink(mFifoInPath.c_str());
        mFifoInPath.clear();
    }

    if (!mFifoOutPath.empty())
    {
        unlink(mFifoOutPath.c_str());
        mFifoOutPath.clear();
    }
}

=======
>>>>>>> 4f652d73ea (Out of band communication ota su (#41900))
void * NamedPipeCommands::EventCommandListenerTask(void * arg)
{
    char readbuf[kChipEventCmdBufSize];
    NamedPipeCommands * self = reinterpret_cast<NamedPipeCommands *>(arg);

    ChipLogProgress(NotSpecified, "Starting named pipe handling on %s, max message size (%u)", self->mFifoInPath.c_str(),
                    static_cast<unsigned>(sizeof(readbuf) - 1));

    // Open with O_RDWR to prevent read() from returning 0 (EOF) and busy-looping when writers close the FIFO.
    int fd = open(self->mFifoInPath.c_str(), O_RDWR);

    if (fd == -1)
    {
        ChipLogError(NotSpecified, "Failed to open Event FIFO");
        return nullptr;
    }

    while (self->mRunning)
    {
        ssize_t numBytesRead = read(fd, readbuf, sizeof(readbuf) - 1);
        if (numBytesRead <= 0)
        {
            // If the read was interrupted by a signal before any data was available,
            // we should retry the read operation.
            if (numBytesRead < 0 && errno == EINTR)
            {
                continue;
            }

            // For any other read failure (including EOF where numBytesRead == 0), we exit the loop.
            // Note: Since the FIFO is opened with O_RDWR, we don't expect to receive EOF
            // when all external writers disconnect.
            if (numBytesRead < 0)
            {
                ChipLogError(NotSpecified, "Error reading from FIFO: %d", errno);
            }
            break;
        }

        // Null-terminate for processing (not guaranteed by writer).
        readbuf[numBytesRead] = '\0';

        if (numBytesRead == 0)
        {
            continue;
        }

        ChipLogProgress(NotSpecified, "Received payload of %u bytes: '%s'", static_cast<unsigned>(numBytesRead), readbuf);

        // Process the received command request from event fifo
        if (self->mDelegate)
        {
            self->mDelegate->OnEventCommandReceived(readbuf);
        }
    }

    if (fd != -1)
    {
        close(fd);
    }
    ChipLogProgress(NotSpecified, "Done with named pipe handling on %s", self->mFifoInPath.c_str());

    return nullptr;
}
