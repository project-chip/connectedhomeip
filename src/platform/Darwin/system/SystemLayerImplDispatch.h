/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file declares an implementation of System::Layer using dispatch.
 */

#pragma once

#include "system/SystemClock.h"
#include "system/SystemConfig.h"

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

#include <vector>
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
#include <mutex>
#endif

namespace chip {
namespace System {

class LayerImplDispatch : public LayerDispatch
{
public:
    LayerImplDispatch() = default;
    ~LayerImplDispatch() override { VerifyOrDie(mLayerState.Destroy()); }

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

    // LayerDispatch overrides.
    void SetDispatchQueue(dispatch_queue_t dispatchQueue) override { mDispatchQueue = dispatchQueue; };
    void HandleDispatchQueueEvents(Clock::Timeout timeout) override;
    CHIP_ERROR ScheduleWorkWithBlock(dispatch_block_t block) override;
    CHIP_ERROR StartTimerWithBlock(dispatch_block_t block, Clock::Timeout delay) override;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // LayerSockets overrides.
    CHIP_ERROR StartWatchingSocket(int fd, SocketWatchToken * tokenOut) override;
    CHIP_ERROR SetCallback(SocketWatchToken token, SocketWatchCallback callback, intptr_t data) override;
    CHIP_ERROR RequestCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR RequestCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingRead(SocketWatchToken token) override;
    CHIP_ERROR ClearCallbackOnPendingWrite(SocketWatchToken token) override;
    CHIP_ERROR StopWatchingSocket(SocketWatchToken * tokenInOut) override;
    SocketWatchToken InvalidSocketWatchToken() override { return reinterpret_cast<SocketWatchToken>(nullptr); }
#endif

protected:
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    struct SelectSets
    {
        fd_set mReadSet;
        fd_set mWriteSet;
        fd_set mErrorSet;
    };

    struct SocketWatch
    {
        int mFD;
        SocketEvents mPendingIO;
        SocketWatchCallback mCallback;
        intptr_t mCallbackData;

        dispatch_source_t mRdSource = nullptr;
        dispatch_source_t mWrSource = nullptr;

        bool IsActive() const { return mFD != kInvalidFd; }
        void PrepareEvents(SelectSets & sets, int & maxFd) const;
        void HandleEvents(const SelectSets & sets) const;
        void Clear();
    };

    class SocketWatchPool
    {
    public:
        CHIP_ERROR Allocate(int fd, SocketWatch *& outWatch);
        CHIP_ERROR Release(SocketWatch * watch);
        void Clear();

        bool PrepareEvents(SelectSets & sets, timeval timeout) const;
        void HandleEvents(const SelectSets & sets) const;

    private:
        static constexpr int kSocketWatchMax = (INET_CONFIG_ENABLE_TCP_ENDPOINT ? INET_CONFIG_NUM_TCP_ENDPOINTS : 0) +
            (INET_CONFIG_ENABLE_UDP_ENDPOINT ? INET_CONFIG_NUM_UDP_ENDPOINTS : 0);
        SocketWatch mPool[kSocketWatchMax];
    };

    SocketWatchPool mSocketWatchPool;
    void HandleSocketsAndTimerEvents(Clock::Timeout timeout);
    CHIP_ERROR RequestCallback(SocketWatchToken token, SocketEventFlags flag);
    CHIP_ERROR ClearCallback(SocketWatchToken token, SocketEventFlags flag);
#endif

    dispatch_queue_t GetDispatchQueue()
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        //  We create a suspended serial dispatch queue
        if (!mDispatchQueue)
        {
            mDispatchQueue = dispatch_queue_create("com.chip.unit_test_fallback", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            mHasSuspendedDispatchQueue = true;
            dispatch_suspend(mDispatchQueue);
        }
#endif
        VerifyOrDie(nullptr != mDispatchQueue);
        return mDispatchQueue;
    };

    // Struct to hold some data for every timer (the timer source object, the time that the timer was supposed to fire, the callback
    // and the state to pass to the callback).
    struct TimerData
    {
        TimerData(dispatch_source_t timerSource, Clock::Timestamp awakenTime, TimerCompleteCallback onComplete, void * appState);
        ~TimerData();

        dispatch_source_t timerSource;
        // Sadly, there doesn't seem to be a way to get the remaining or the
        // awaken time of a timer from its dispatch_source_t. Also, dispatch_time_t
        // is an opaque value that can't be used for computations or comparisons.
        // So we store the time here calculated from SystemClock's monotonic timer.
        Clock::Timestamp awakenTime;
        TimerCompleteCallback onComplete;
        void * appState;
    };

    std::vector<TimerData> mTimers;
    void HandleTimerEvents(Clock::Timeout timeout);

private:
    static void EnableTimer(const char * source, dispatch_source_t timerSource);
    static void DisableTimer(const char * source, dispatch_source_t timerSource);
    static std::vector<TimerData>::const_iterator CancelTimer(std::vector<TimerData> & timers, TimerCompleteCallback onComplete,
                                                              void * appState);

    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState, bool shouldCancel);

    ObjectLifeCycle mLayerState;

    dispatch_queue_t mDispatchQueue = nullptr;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    std::vector<TimerData> mExpiredTimers;
    bool mHasSuspendedDispatchQueue = false;

    std::mutex mTestQueueMutex;
    std::vector<dispatch_block_t> mTestQueuedBlocks;
#endif
};

using LayerImpl = LayerImplDispatch;

} // namespace System
} // namespace chip
