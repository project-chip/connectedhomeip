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
 *      This file declares an implementation of System::Layer using libevent.
 */

#pragma once

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
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

class LayerImplLibevent : public LayerSocketsLoop
{
public:
    LayerImplLibevent() : mEventBase(nullptr), mMdnsTimeoutEvent(nullptr) {}
    ~LayerImplLibevent() { mLayerState.Destroy(); }

    // Layer overrides.
    CHIP_ERROR Init() override;
    CHIP_ERROR Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CHIP_ERROR StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

    // LayerSocket overrides.
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut) override;
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data) override;
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut) override;
    SocketWatchToken InvalidSocketWatchToken() override { return reinterpret_cast<SocketWatchToken>(nullptr); }

    // LayerSocketLoop overrides.
    void Signal() override;
    void EventLoopBegins() override {}
    void PrepareEvents() override;
    void WaitForEvents() override;
    void HandleEvents() override;
    void EventLoopEnds() override {}

private:
    /*
     * In this implementation, libevent invokes TimerCallbackHandler and SocketCallbackHandler from beneath WaitForEvents(),
     * which means that the CHIP stack is unlocked. These handles add the LibeventTimer or SocketWatch respectively to a list,
     * then HandleEvents() invokes the client callbacks.
     */
    struct LibeventTimer
    {
        LibeventTimer(LayerImplLibevent * layer, TimerCompleteCallback onComplete, void * data) :
            mEventManager(layer), mOnComplete(onComplete), mCallbackData(data), mEvent(nullptr)
        {}
        ~LibeventTimer();
        LayerImplLibevent * mEventManager;
        TimerCompleteCallback mOnComplete;
        void * mCallbackData;
        event * mEvent;
    };
    static void TimerCallbackHandler(evutil_socket_t fd, short eventFlags, void * data);

    struct SocketWatch
    {
        SocketWatch(LayerImplLibevent * layer, int fd) :
            mEventManager(layer), mFD(fd), mCallback(nullptr), mCallbackData(0), mEvent(nullptr)
        {}
        ~SocketWatch();
        LayerImplLibevent * mEventManager;
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

    event_base * mEventBase; ///< libevent shared state.

    std::list<std::unique_ptr<LibeventTimer>> mTimers;
    std::list<LibeventTimer *> mActiveTimers;
    Mutex mTimerListMutex;

    std::list<std::unique_ptr<SocketWatch>> mSocketWatches;
    std::list<SocketWatch *> mActiveSocketWatches;

    ObjectLifeCycle mLayerState;
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

using LayerImpl = LayerImplLibevent;

} // namespace System
} // namespace chip
