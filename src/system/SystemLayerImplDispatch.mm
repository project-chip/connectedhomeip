/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file implements Layer using select().
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <platform/LockTracker.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplDispatch.h>

#include <algorithm>
#include <errno.h>

namespace chip {
namespace System {
    dispatch_queue_t LayerImplDispatch::GetDispatchQueue()
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (nullptr == mDispatchQueue) {
            mDispatchQueue = dispatch_queue_create("com.csa.matter.testing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
            dispatch_suspend(mDispatchQueue);
        }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return mDispatchQueue;
    };

    void LayerImplDispatch::EnableTimer(TimerList::Node * timer)
    {
        VerifyOrDo(nullptr != timer->mTimerSource, chipDie());
        timer->mIsResumed = true;
        dispatch_resume(timer->mTimerSource);
    }

    void LayerImplDispatch::DisableTimer(TimerList::Node * timer)
    {
        VerifyOrDo(nullptr != timer->mTimerSource, chipDie());

        dispatch_source_cancel(timer->mTimerSource);
        if (timer->mIsResumed == false) {
            dispatch_resume(timer->mTimerSource);
        }
        timer->mTimerSource = nullptr;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        auto it = mTimerInDrainMode.find(timer);
        if (it != mTimerInDrainMode.end()) {
            mTimerInDrainMode.erase(it);
        }
#endif
    }

    CHIP_ERROR LayerImplDispatch::Init()
    {
        VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

        RegisterPOSIXErrorFormatter();

        VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return CHIP_NO_ERROR;
    }

    void LayerImplDispatch::Shutdown()
    {
        VerifyOrReturn(mLayerState.SetShuttingDown());

        TimerList::Node * timer;
        while ((timer = mTimerList.PopEarliest()) != nullptr) {
            DisableTimer(timer);
        }
        mTimerPool.ReleaseAll();

        mLayerState.ResetFromShuttingDown(); // Return to uninitialized state to permit re-initialization.
    }

    CHIP_ERROR LayerImplDispatch::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
    {
        return StartTimer(System::Clock::kZero, onComplete, appState, false /* shouldCancel */);
    }

    CHIP_ERROR LayerImplDispatch::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
    {
        return StartTimer(delay, onComplete, appState, true /* shouldCancel */);
    }

    CHIP_ERROR LayerImplDispatch::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState, bool shouldCancel)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = System::Clock::kZero);

        if (shouldCancel) {
            CancelTimer(onComplete, appState);
        }

        __auto_type * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
        VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

        dispatch_queue_t dispatchQueue = GetDispatchQueue();
        VerifyOrReturnError(nullptr != dispatchQueue, CHIP_ERROR_INTERNAL);

        (void) mTimerList.Add(timer);
        dispatch_source_t timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, dispatchQueue);
        VerifyOrDie(timerSource != nullptr);

        timer->mTimerSource = timerSource;
        dispatch_source_set_timer(
            timerSource, dispatch_walltime(nullptr, static_cast<int64_t>(Clock::Milliseconds64(delay).count() * NSEC_PER_MSEC)),
            DISPATCH_TIME_FOREVER, 2 * NSEC_PER_MSEC);
        dispatch_source_set_event_handler(timerSource, ^{
            DisableTimer(timer);
            mTimerList.Remove(timer);
            mTimerPool.Invoke(timer);
        });

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (mInDrainMode) {
            mTimerInDrainMode.insert(timer);
        } else {
#endif
            EnableTimer(timer);
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        }
#endif
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
    {
        VerifyOrReturnError(delay.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);

        assertChipStackLockedByCurrentThread();

        Clock::Timeout remainingTime = mTimerList.GetRemainingTime(onComplete, appState);
        if (remainingTime.count() < delay.count()) {
            if (remainingTime == Clock::kZero) {
                // If remaining time is Clock::kZero, it might possible that our timer is in
                // the mExpiredTimers list and about to be fired. Remove it from that list, since we are extending it.
                __auto_type * timer = mExpiredTimers.Remove(onComplete, appState);
                if (nullptr != timer) {
                    DisableTimer(timer);
                    mTimerPool.Release(timer);
                }
            }
            return StartTimer(delay, onComplete, appState);
        }

        return CHIP_NO_ERROR;
    }

    bool LayerImplDispatch::IsTimerActive(TimerCompleteCallback onComplete, void * appState)
    {
        bool timerIsActive = (mTimerList.GetRemainingTime(onComplete, appState) > Clock::kZero);

        if (!timerIsActive) {
            // check if the timer is in the mExpiredTimers list about to be fired.
            for (TimerList::Node * timer = mExpiredTimers.Earliest(); timer != nullptr; timer = timer->mNextTimer) {
                if (timer->GetCallback().GetOnComplete() == onComplete && timer->GetCallback().GetAppState() == appState) {
                    return true;
                }
            }
        }

        return timerIsActive;
    }

    Clock::Timeout LayerImplDispatch::GetRemainingTime(TimerCompleteCallback onComplete, void * appState)
    {
        return mTimerList.GetRemainingTime(onComplete, appState);
    }

    void LayerImplDispatch::CancelTimer(TimerCompleteCallback onComplete, void * appState)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mLayerState.IsInitialized());

        __auto_type * timer = mTimerList.Remove(onComplete, appState);
        if (timer == nullptr) {
            // The timer was not in our "will fire in the future" list, but it might
            // be in the "we're about to fire these" chunk we already grabbed from
            // that list.  Check for it there too, and if found there we still want
            // to cancel it.
            timer = mExpiredTimers.Remove(onComplete, appState);
        }
        VerifyOrReturn(timer != nullptr);

        DisableTimer(timer);
        mTimerPool.Release(timer);
    }

    void LayerImplDispatch::HandleDispatchQueueEvents()
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        assertChipStackLockedByCurrentThread();

        for (__auto_type * t : mTimerInDrainMode) {
            EnableTimer(t);
        }
        mTimerInDrainMode.clear();
        mInDrainMode = true;

        // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
        // since that could result in infinite handling of new timers blocking any other progress.
        VerifyOrDieWithMsg(mExpiredTimers.Empty(), DeviceLayer, "Re-entry into HandleEvents from a timer callback?");
        mExpiredTimers = mTimerList.ExtractEarlier(Clock::Timeout(1) + SystemClock().GetMonotonicTimestamp());
        TimerList::Node * timer = nullptr;
        while ((timer = mExpiredTimers.PopEarliest()) != nullptr) {
            DisableTimer(timer);
            mTimerPool.Invoke(timer);
        }

        dispatch_queue_t dispatchQueue = GetDispatchQueue();
        dispatch_resume(dispatchQueue);

        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        dispatch_async(dispatchQueue, ^{
            dispatch_semaphore_signal(sem);
        });

        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

        dispatch_suspend(dispatchQueue);
        mInDrainMode = false;
#endif
    }

} // namespace System
} // namespace chip
