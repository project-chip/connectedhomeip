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
 *      This file implements Layer using dispatch.
 */

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <system/SystemLayerImplDispatch.h>

#include <lib/support/CodeUtils.h>
#include <platform/LockTracker.h>
#include <system/SystemFaultInjection.h>

#define SYSTEM_LAYER_IMPL_DISPATCH_DEBUG 0

namespace chip {
namespace System {
    namespace {
        struct TimerCompleteBlockCallbackContext {
            dispatch_block_t block;
        };

        static void TimerCompleteBlockCallback(Layer * aLayer, void * appState)
        {
            __auto_type * ctx = static_cast<TimerCompleteBlockCallbackContext *>(appState);
            if (ctx->block) {
                ctx->block();
            }
            delete ctx;
        }
    }

    void LayerImplDispatch::EnableTimer(const char * source, TimerList::Node * timer)
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrReturn(nullptr != timer->mTimerSource);
#else
        VerifyOrDie(nullptr != timer->mTimerSource);
#endif

#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        bool resumed = timer->mTimerSource ? dispatch_testcancel(timer->mTimerSource) : false;
        ChipLogError(Inet, "%s (%s) : timer=%p - source=%p - resumed=%d", __func__, source, timer, timer->mTimerSource, resumed);
#endif
        dispatch_resume(timer->mTimerSource);
    }

    void LayerImplDispatch::DisableTimer(const char * source, TimerList::Node * timer)
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrReturn(nullptr != timer->mTimerSource);
#else
        VerifyOrDie(nullptr != timer->mTimerSource);
#endif

#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        bool resumed = timer->mTimerSource ? dispatch_testcancel(timer->mTimerSource) : false;
        ChipLogError(Inet, "%s (%s) : timer=%p - source=%p - resumed=%d", __func__, source, timer, timer->mTimerSource, resumed);
#endif

        if (!dispatch_testcancel(timer->mTimerSource)) {
            dispatch_source_cancel(timer->mTimerSource);
        }
        timer->mTimerSource = nullptr;
    }

    CHIP_ERROR LayerImplDispatch::Init()
    {
        VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

        RegisterPOSIXErrorFormatter();

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        mSocketWatchPool.Clear();
#endif

        VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
        return CHIP_NO_ERROR;
    }

    void LayerImplDispatch::Shutdown()
    {
        VerifyOrReturn(mLayerState.SetShuttingDown());

        TimerList::Node * timer;
        while ((timer = mTimerList.PopEarliest()) != nullptr) {
            DisableTimer(__func__, timer);
        }
        mTimerPool.ReleaseAll();

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        mSocketWatchPool.Clear();
#endif

        mLayerState.ResetFromShuttingDown(); // Return to uninitialized state to permit re-initialization.
    }

    CHIP_ERROR LayerImplDispatch::ScheduleWorkWithBlock(dispatch_block_t block)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (block: %p)", __func__, block);
#endif

        VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        __auto_type dispatchQueue = GetDispatchQueue();
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrDie(nullptr != dispatchQueue);
#endif

        if (dispatchQueue) {
            dispatch_async(dispatchQueue, ^{
                block();
            });
            return CHIP_NO_ERROR;
        }

        TimerCompleteBlockCallbackContext * ctx = new TimerCompleteBlockCallbackContext { .block = block };
        VerifyOrReturnError(ctx != nullptr, CHIP_ERROR_NO_MEMORY);
        return ScheduleWork(TimerCompleteBlockCallback, ctx);
    }

    CHIP_ERROR LayerImplDispatch::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        return StartTimer(System::Clock::kZero, onComplete, appState, false /* shouldCancel */);
    }

    CHIP_ERROR LayerImplDispatch::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
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

        dispatch_queue_t dispatchQueue = GetDispatchQueue();
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
        VerifyOrReturnError(nullptr != dispatchQueue, CHIP_ERROR_INTERNAL);
#endif

        __auto_type * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
        VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

        if (dispatchQueue) {
            __auto_type timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, dispatchQueue);
            VerifyOrDie(timerSource != nullptr);

            timer->mTimerSource = timerSource;

            dispatch_source_set_timer(
                timerSource, dispatch_walltime(nullptr, static_cast<int64_t>(Clock::Milliseconds64(delay).count() * NSEC_PER_MSEC)),
                DISPATCH_TIME_FOREVER, 2 * NSEC_PER_MSEC);

            dispatch_source_set_event_handler(timerSource, ^{
                DisableTimer(__func__, timer);
                mTimerList.Remove(timer);
                mTimerPool.Invoke(timer);
            });

            EnableTimer(__func__, timer);
        }

        (void) mTimerList.Add(timer);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        VerifyOrReturnError(delay.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);

        assertChipStackLockedByCurrentThread();

        Clock::Timeout remainingTime = mTimerList.GetRemainingTime(onComplete, appState);
        if (remainingTime.count() < delay.count()) {
            if (remainingTime == Clock::kZero) {
                // If remaining time is Clock::kZero, it might possible that our timer is in
                // the mExpiredTimers list and about to be fired. Remove it from that list, since we are extending it.
                __auto_type * timer = mExpiredTimers.Remove(onComplete, appState);
                if (nullptr != timer) {
                    DisableTimer(__func__, timer);
                    mTimerPool.Release(timer);
                }
            }
            return StartTimer(delay, onComplete, appState);
        }

        return CHIP_NO_ERROR;
    }

    bool LayerImplDispatch::IsTimerActive(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
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
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        return mTimerList.GetRemainingTime(onComplete, appState);
    }

    void LayerImplDispatch::CancelTimer(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
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

        DisableTimer(__func__, timer);
        mTimerPool.Release(timer);
    }

    void LayerImplDispatch::HandleDispatchQueueEvents(Clock::Timeout timeout)
    {
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        HandleSocketsAndTimerEvents(timeout);
#else
        HandleTimerEvents(timeout);
#endif
    }

    void LayerImplDispatch::HandleTimerEvents(Clock::Timeout timeout)
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
        // since that could result in infinite handling of new timers blocking any other progress.
        VerifyOrDieWithMsg(mExpiredTimers.Empty(), DeviceLayer, "Re-entry into HandleEvents from a timer callback?");
        mExpiredTimers = mTimerList.ExtractEarlier(Clock::Timeout(1) + SystemClock().GetMonotonicTimestamp());
        TimerList::Node * timer = nullptr;
        while ((timer = mExpiredTimers.PopEarliest()) != nullptr) {
            DisableTimer(__func__, timer);
            mTimerPool.Invoke(timer);
        }
#endif
    }
} // namespace System
} // namespace chip
