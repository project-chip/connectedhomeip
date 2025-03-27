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
 *      This file declares an implementation of System::Layer using select().
 */

#pragma once

#include "system/SystemConfig.h"

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/select.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <inet/ZephyrSocket.h> // nogncheck
#endif

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <atomic>
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_LIBEV
#include <ev.h>
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#error "CHIP_SYSTEM_CONFIG_USE_LIBEV and CHIP_SYSTEM_CONFIG_USE_DISPATCH are mutually exclusive"
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LIBEV

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>
#include <system/WakeEvent.h>

namespace chip {
namespace System {

class LayerImplSelect : public LayerSocketsLoop
{
public:
    LayerImplSelect() = default;
    ~LayerImplSelect() override { VerifyOrDie(mLayerState.Destroy()); }

    // Layer overrides.
    CHIP_ERROR Init() override;
    void Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override;
    Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) override;
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

#if !CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void AddLoopHandler(EventLoopHandler & handler) override;
    void RemoveLoopHandler(EventLoopHandler & handler) override;
#endif // !CHIP_SYSTEM_CONFIG_USE_DISPATCH

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    void SetDispatchQueue(dispatch_queue_t dispatchQueue) override { mDispatchQueue = dispatchQueue; };
    dispatch_queue_t GetDispatchQueue() override { return mDispatchQueue; };
    void HandleTimerComplete(TimerList::Node * timer);
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    virtual void SetLibEvLoop(struct ev_loop * aLibEvLoopP) override { mLibEvLoopP = aLibEvLoopP; };
    virtual struct ev_loop * GetLibEvLoop() override { return mLibEvLoopP; };
    static void HandleLibEvTimer(EV_P_ struct ev_timer * t, int revents);
    static void HandleLibEvIoWatcher(EV_P_ struct ev_io * i, int revents);
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH/LIBEV

    // Expose the result of WaitForEvents() for non-blocking socket implementations.
    bool IsSelectResultValid() const { return mSelectResult >= 0; }

protected:
    static SocketEvents SocketEventsFromFDs(int socket, const fd_set & readfds, const fd_set & writefds, const fd_set & exceptfds);

    static constexpr int kSocketWatchMax = (INET_CONFIG_ENABLE_TCP_ENDPOINT ? INET_CONFIG_NUM_TCP_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_UDP_ENDPOINT ? INET_CONFIG_NUM_UDP_ENDPOINTS : 0);

    struct SocketWatch
    {
        void Clear();
        int mFD;
        SocketEvents mPendingIO;
        SocketWatchCallback mCallback;
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
        dispatch_source_t mRdSource;
        dispatch_source_t mWrSource;
        void DisableAndClear();
#endif
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
        struct ev_io mIoWatcher;
        LayerImplSelect * mLayerImplSelectP;
        void DisableAndClear();
#endif

        intptr_t mCallbackData;
    };
    SocketWatch mSocketWatchPool[kSocketWatchMax];

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    // List of expired timers being processed right now.  Stored in a member so
    // we can cancel them.
    TimerList mExpiredTimers;
    timeval mNextTimeout;

#if !CHIP_SYSTEM_CONFIG_USE_DISPATCH
    IntrusiveList<EventLoopHandler> mLoopHandlers;
#endif

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

    ObjectLifeCycle mLayerState;
#if !CHIP_SYSTEM_CONFIG_USE_LIBEV && !CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    WakeEvent mWakeEvent;
#endif

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    std::atomic<pthread_t> mHandleSelectThread;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t mDispatchQueue = nullptr;
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    struct ev_loop * mLibEvLoopP;
#endif
};

using LayerImpl = LayerImplSelect;

} // namespace System
} // namespace chip
