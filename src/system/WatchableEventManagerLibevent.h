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
 *      This file declares an implementation of WatchableEventManager using libevent.
 */

#pragma once

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE
#error "This file should only be included from <system/WatchableEventManager.h>"
#include <system/WatchableEventManager.h>
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_EVENT_MANAGER_CONFIG_FILE

#include <system/WakeEvent.h>

#include <event2/event.h>

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

namespace chip {

namespace System {

class WatchableEventManager
{
public:
    WatchableEventManager() : mActiveSockets(nullptr), mSystemLayer(nullptr), mEventBase(nullptr), mTimeoutEvent(nullptr) {}

    // Core ‘overrides’.
    CHIP_ERROR Init(Layer & systemLayer);
    CHIP_ERROR Shutdown();
    void Signal();

    // Timer ‘overrides’.
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, Timers::OnCompleteFunct onComplete, void * appState);
    void CancelTimer(Timers::OnCompleteFunct onComplete, void * appState);
    CHIP_ERROR ScheduleWork(Timers::OnCompleteFunct onComplete, void * appState) { return StartTimer(0, onComplete, appState); }

    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

private:
    /*
     * In this implementation, libevent invokes LibeventCallbackHandler from beneath WaitForEvents(),
     * which means that the CHIP stack is unlocked. LibeventCallbackHandler adds the WatchableSocket
     * to a queue (implemented as a simple intrusive list to avoid dynamic memory allocation), and
     * then HandleEvents() invokes the WatchableSocket callbacks.
     */
    friend class WatchableSocket;
    static void LibeventCallbackHandler(evutil_socket_t fd, short eventFlags, void * data);
    void RemoveFromQueueIfPresent(WatchableSocket * watcher);

    WatchableSocket * mActiveSockets; ///< List of sockets activated by libevent.

    Layer * mSystemLayer;
    event_base * mEventBase; ///< libevent shared state.
    event * mTimeoutEvent;

    Timer::MutexedList mTimerList;
    WakeEvent mWakeEvent;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
};

} // namespace System
} // namespace chip
