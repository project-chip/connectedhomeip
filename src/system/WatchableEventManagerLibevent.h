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
#include <list>
#include <memory>
#include <vector>

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

namespace chip {

namespace System {

class WatchableEventManager
{
public:
    WatchableEventManager() : mSystemLayer(nullptr), mEventBase(nullptr), mMdnsTimeoutEvent(nullptr) {}

private:
    // Transitionally, ensure that these ‘overrides’ can only be called via the System::Layer equivalents.
    friend class Layer;

    // Core ‘overrides’.
    CHIP_ERROR Init(Layer & systemLayer);
    CHIP_ERROR Shutdown();

    // Timer ‘overrides’.
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState);
    void CancelTimer(TimerCompleteCallback onComplete, void * appState);
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) { return StartTimer(0, onComplete, appState); }

    // Socket watch ‘overrides’.
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut);
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data);
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token);
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token);
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token);
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token);
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut);
    SocketWatchToken InvalidSocketWatchToken() { return reinterpret_cast<SocketWatchToken>(nullptr); }

public:
    void Signal();

    // Platform implementation.
    void EventLoopBegins() {}
    void PrepareEvents();
    void WaitForEvents();
    void HandleEvents();
    void EventLoopEnds() {}

private:
    /*
     * In this implementation, libevent invokes TimerCallbackHandler and SocketCallbackHandler from beneath WaitForEvents(),
     * which means that the CHIP stack is unlocked. These handles add the LibeventTimer or SocketWatch respectively to a list,
     * then HandleEvents() invokes the client callbacks.
     */
    struct LibeventTimer
    {
        LibeventTimer(WatchableEventManager * layer, TimerCompleteCallback onComplete, void * data) :
            mEventManager(layer), mOnComplete(onComplete), mCallbackData(data), mEvent(nullptr)
        {}
        ~LibeventTimer();
        WatchableEventManager * mEventManager;
        TimerCompleteCallback mOnComplete;
        void * mCallbackData;
        event * mEvent;
    };
    static void TimerCallbackHandler(evutil_socket_t fd, short eventFlags, void * data);

    struct SocketWatch
    {
        SocketWatch(WatchableEventManager * layer, int fd) :
            mEventManager(layer), mFD(fd), mCallback(nullptr), mCallbackData(0), mEvent(nullptr)
        {}
        ~SocketWatch();
        WatchableEventManager * mEventManager;
        int mFD;
        SocketEvents mPendingIO;
        SocketWatchCallback mCallback;
        intptr_t mCallbackData;
        event * mEvent;
    };
    CHIP_ERROR SetWatch(SocketWatchToken token, short eventFlags);
    CHIP_ERROR ClearWatch(SocketWatchToken token, short eventFlags);
    CHIP_ERROR UpdateWatch(SocketWatch * watch, short eventFlags);
    static void SocketCallbackHandler(evutil_socket_t fd, short eventFlags, void * data);

    Layer * mSystemLayer;
    event_base * mEventBase; ///< libevent shared state.

    std::list<std::unique_ptr<LibeventTimer>> mTimers;
    std::list<LibeventTimer *> mActiveTimers;
    Mutex mTimerListMutex;

    std::list<std::unique_ptr<SocketWatch>> mSocketWatches;
    std::list<SocketWatch *> mActiveSocketWatches;

    WakeEvent mWakeEvent;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
    static void MdnsTimeoutCallbackHandler(evutil_socket_t fd, short eventFlags, void * data);
    void MdnsTimeoutCallbackHandler();
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS && !__ZEPHYR__
    event * mMdnsTimeoutEvent;
};

} // namespace System
} // namespace chip
