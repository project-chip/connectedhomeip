/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file declares an implementation of WatchableEventManager using select().
 */

#pragma once

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#error "This file should only be included from <system/WatchableEventManager.h>"
#include <system/WatchableEventManager.h>
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE

#include <sys/select.h>

#include <support/BitFlags.h>
#include <support/logging/CHIPLogging.h>
#include <system/WakeEvent.h>

namespace chip {
namespace System {

class WatchableSocket;

class WatchableEventManager
{
public:
    CHIP_ERROR Init(System::Layer & systemLayer);
    CHIP_ERROR Shutdown();
    CHIP_ERROR Signal();

    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

    // TODO(#5556): Some unit tests supply a timeout at low level, due to originally using select(); these should a proper timer.
    void PrepareEventsWithTimeout(timeval & nextTimeout);

    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

protected:
    friend class WatchableSocket;

    CHIP_ERROR Set(int fd, fd_set * fds);
    CHIP_ERROR Clear(int fd, fd_set * fds);

    Layer * mSystemLayer               = nullptr;
    WatchableSocket * mAttachedSockets = nullptr;

    // TODO(#5556): Integrate timer platform details with WatchableEventManager.
    struct timeval mNextTimeout;

    // Members for select loop
    struct SelectSets
    {
        fd_set mReadSet;
        fd_set mWriteSet;
        fd_set mErrorSet;
    };
    SelectSets mRequest;
    SelectSets mSelected;
    int mMaxFd;
    int mSelectResult; ///< return value from select()

    WakeEvent mWakeEvent;

private:
    bool HasAny(int fd);
    void MaybeLowerMaxFd();
    void Reset(int fd);
};

} // namespace System
} // namespace chip
