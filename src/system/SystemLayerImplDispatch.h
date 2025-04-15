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
 *      This file declares an implementation of System::Layer using select().
 */

#pragma once

#include "system/SystemConfig.h"

#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

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

protected:
    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    // List of expired timers being processed right now.  Stored in a member so
    // we can cancel them.
    TimerList mExpiredTimers;
    timeval mNextTimeout;
    void EnableTimer(const char * source, TimerList::Node *);
    void DisableTimer(const char * source, TimerList::Node *);
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState, bool shouldCancel);

    ObjectLifeCycle mLayerState;

    dispatch_queue_t mDispatchQueue = nullptr;
};

using LayerImpl = LayerImplDispatch;

} // namespace System
} // namespace chip
