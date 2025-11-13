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

#include "system/SystemConfig.h"

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
#include <mutex>
#include <vector>
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
    dispatch_queue_t GetDispatchQueue() override { return mDispatchQueue; };
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

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    // List of expired timers being processed right now.  Stored in a member so
    // we can cancel them.
    TimerList mExpiredTimers;
    void EnableTimer(const char * source, TimerList::Node *);
    void DisableTimer(const char * source, TimerList::Node *);
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState, bool shouldCancel);
    void HandleTimerEvents(Clock::Timeout timeout);

    ObjectLifeCycle mLayerState;

    dispatch_queue_t mDispatchQueue = nullptr;

private:
    inline bool HasTimerSource(TimerList::Node * timer)
    {
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrDie(nullptr != timer->mTimerSource);
#endif
        return nullptr != timer->mTimerSource;
    }

    inline bool HasDispatchQueue(dispatch_queue_t queue)
    {
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrDie(nullptr != queue);
#endif
        return nullptr != queue;
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    std::mutex mTestQueueMutex;
    std::vector<dispatch_block_t> mTestQueuedBlocks;
#endif
};

using LayerImpl = LayerImplDispatch;

} // namespace System
} // namespace chip
