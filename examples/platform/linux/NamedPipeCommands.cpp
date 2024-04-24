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

#include <errno.h>
#include <fcntl.h>
#include <lib/support/CodeUtils.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

static constexpr const size_t kChipEventCmdBufSize = 256;

CHIP_ERROR NamedPipeCommands::Start(std::string & path, NamedPipeCommandDelegate * delegate)
{
    VerifyOrReturnError(!mStarted, CHIP_NO_ERROR);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mStarted           = true;
    mDelegate          = delegate;
    mChipEventFifoPath = path;

    // Creating the named file(FIFO)
    VerifyOrReturnError((mkfifo(path.c_str(), 0666) == 0) || (errno == EEXIST), CHIP_ERROR_OPEN_FAILED);
    VerifyOrReturnError(
        pthread_create(&mChipEventCommandListener, nullptr, EventCommandListenerTask, reinterpret_cast<void *>(this)) == 0,
        CHIP_ERROR_UNEXPECTED_EVENT);

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

    return CHIP_NO_ERROR;
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

        ssize_t readBytes      = read(fd, readbuf, kChipEventCmdBufSize);
        readbuf[readBytes - 1] = '\0';
        ChipLogProgress(NotSpecified, "Received payload: \"%s\"", readbuf);

        // Process the received command request from event fifo
        self->mDelegate->OnEventCommandReceived(readbuf);

        close(fd);
    }

    return nullptr;
}
