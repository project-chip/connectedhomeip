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

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#include <support/BitFlags.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemTimer.h>
#include <system/WakeEvent.h>

namespace chip {
namespace System {

class Layer;
class Timer;
class WatchableSocket;

class WatchableEventManager
{
public:
    // Core ‘overrides’.
    CHIP_ERROR Init(System::Layer & systemLayer);
    CHIP_ERROR Shutdown();
    void Signal();

    // Timer ‘overrides’.
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, Timers::OnCompleteFunct onComplete, void * appState);
    void CancelTimer(Timers::OnCompleteFunct onComplete, void * appState);
    CHIP_ERROR ScheduleWork(Timers::OnCompleteFunct onComplete, void * appState);

    // Platform implementation.
    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void SetDispatchQueue(dispatch_queue_t dispatchQueue) { mDispatchQueue = dispatchQueue; };
    dispatch_queue_t GetDispatchQueue() { return mDispatchQueue; };
    void HandleTimerComplete(Timer * timer);
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

protected:
    friend class WatchableSocket;

    CHIP_ERROR SetRequest(int fd, fd_set * fds);
    CHIP_ERROR ClearRequest(int fd, fd_set * fds);

    Layer * mSystemLayer               = nullptr;
    WatchableSocket * mAttachedSockets = nullptr;
    Timer::MutexedList mTimerList;
    timeval mNextTimeout;

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

    // Return value from select(), carried between WaitForEvents() and HandleEvents().
    int mSelectResult;

    WakeEvent mWakeEvent;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t mDispatchQueue;
#endif

private:
    bool HasAnyRequest(int fd);
    void MaybeLowerMaxFd();
    void ResetRequests(int fd);
};

} // namespace System
} // namespace chip
