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

#include <system/WakeEvent.h>

namespace chip {
namespace System {

class WatchableEventManager
{
private:
    // Transitionally, ensure that these ‘overrides’ can only be called via the System::Layer equivalents.
    friend class Layer;

    // Core ‘overrides’.
    CHIP_ERROR Init(System::Layer & systemLayer);
    CHIP_ERROR Shutdown();

    // Timer ‘overrides’.
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState);
    void CancelTimer(TimerCompleteCallback onComplete, void * appState);
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState);

    // Socket watch ‘overrides’.
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut);
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data);
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token);
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token);
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token);
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token);
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut);
    SocketWatchToken InvalidSocketWatchToken() { return reinterpret_cast<SocketWatchToken>(nullptr); }

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void SetDispatchQueue(dispatch_queue_t dispatchQueue) { mDispatchQueue = dispatchQueue; };
    dispatch_queue_t GetDispatchQueue() { return mDispatchQueue; };
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

public:
    // Platform implementation.
    void Signal();
    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void HandleTimerComplete(Timer * timer);
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

protected:
    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

    static constexpr int kSocketWatchMax = (INET_CONFIG_ENABLE_RAW_ENDPOINT ? INET_CONFIG_NUM_RAW_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_TCP_ENDPOINT ? INET_CONFIG_NUM_TCP_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_UDP_ENDPOINT ? INET_CONFIG_NUM_UDP_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_DNS_RESOLVER ? INET_CONFIG_NUM_DNS_RESOLVERS : 0);

    struct SocketWatch
    {
        void Clear();
        int mFD;
        SocketEvents mPendingIO;
        SocketWatchCallback mCallback;
        intptr_t mCallbackData;
    };
    SocketWatch mSocketWatchPool[kSocketWatchMax];

    Layer * mSystemLayer = nullptr;
    Timer::MutexedList mTimerList;
    timeval mNextTimeout;

    // Members for select loop
    struct SelectSets
    {
        fd_set mReadSet;
        fd_set mWriteSet;
        fd_set mErrorSet;
    };
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
};

} // namespace System
} // namespace chip
