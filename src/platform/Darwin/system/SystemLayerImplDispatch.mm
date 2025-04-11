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

#include <algorithm>
#include <chrono>
#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include <system/SystemLayerImplDispatch.h>

#include <lib/support/CodeUtils.h>
#include <platform/LockTracker.h>
#include <system/SystemFaultInjection.h>

#define SYSTEM_LAYER_IMPL_DISPATCH_DEBUG 0

// Note: CONFIG_BUILD_FOR_HOST_UNIT_TEST
//
// Certain unit tests are executed without a main dispatch queue, relying instead on a mock clock
// to manually trigger timer callbacks at specific times.
// Under normal conditions, the absence of a dispatch queue would cause the tests to fail. However,
// when CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined, this constraint is relaxed, allowing such tests
// to run successfully.
//
// Consequently, the StartTimer method conditionally skips installing dispatch sources when no dispatch
// queue is available, since these dispatch sources would never trigger during these specific unit tests.
// In these scenarios, timer events are explicitly triggered via calls to HandleDispatchQueueEvents(), using
// the mock clock provided by the test environment.
// Creating a dispatch queue in test mode does not work because dispatch_source timers always follow
// the real system clock, not our mock clock, so we must rely on mTimerList + HandleDispatchQueueEvents() instead.

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
        }
    }

    void LayerImplDispatch::EnableTimer(const char * source, dispatch_source_t timerSource)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (%s) : source=%p", __func__, source, timerSource);
#endif
        dispatch_resume(timerSource);
    }

    void LayerImplDispatch::DisableTimer(const char * source, dispatch_source_t timerSource)
    {
        bool isCancelled = dispatch_testcancel(timerSource);

#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (%s) : source=%p - cancelled=%d", __func__, source, timerSource, cancelled);
#endif

        if (!isCancelled) {
            dispatch_source_cancel(timerSource);
        }
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

        for (dispatch_source_t timerSource : mTimers) {
            DisableTimer(__func__, timerSource);
        };
        mTimers.clear();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        mTimerPool.ReleaseAll();
#endif

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
        if (HasDispatchQueue(dispatchQueue)) {
            dispatch_async(dispatchQueue, ^{
                block();
            });
        }
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        else {
            std::lock_guard<std::mutex> lock(mTestQueueMutex);
            mTestQueuedBlocks.emplace_back(block);
        }
#endif
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        return StartTimer(System::Clock::kZero, onComplete, appState, false /* shouldCancel */);
    }

    CHIP_ERROR LayerImplDispatch::StartTimerWithBlock(dispatch_block_t block, Clock::Timeout delay)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

        __auto_type * ctx = new TimerCompleteBlockCallbackContext { block };
        VerifyOrReturnError(nullptr != ctx, CHIP_ERROR_NO_MEMORY);

        return StartTimer(delay, TimerCompleteBlockCallback, ctx);
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

        __auto_type dispatchQueue = GetDispatchQueue();
        if (HasDispatchQueue(dispatchQueue)) {
            __auto_type timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, dispatchQueue);
            VerifyOrDie(timerSource != nullptr);

            dispatch_time_t walltime = dispatch_time(DISPATCH_WALLTIME_NOW, std::chrono::nanoseconds(Clock::Milliseconds64(delay)).count());

            TimerData::Set(timerSource, new TimerData { walltime, onComplete, appState });
            dispatch_source_set_timer(timerSource, walltime, DISPATCH_TIME_FOREVER, std::chrono::nanoseconds(2).count());

            LayerImplDispatch * self = this;
            dispatch_source_set_event_handler(timerSource, ^{
                std::unique_ptr<TimerData> data(TimerData::GetAndClear(timerSource));
                mTimers.erase(std::find(mTimers.begin(), mTimers.end(), timerSource));
                data->onComplete(self, data->appState);
            });

            dispatch_source_set_cancel_handler(timerSource, ^{
                std::unique_ptr<TimerData> data(TimerData::GetAndClear(timerSource));

                VerifyOrReturn(data->onComplete == TimerCompleteBlockCallback);

                delete static_cast<TimerCompleteBlockCallbackContext *>(data->appState);
            });

            EnableTimer(__func__, timerSource);

            mTimers.push_back(timerSource);
        } else {
#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
            __auto_type * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
            VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

            mTimerList.Add(timer);
#endif
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR LayerImplDispatch::ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        VerifyOrReturnError(delay.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);

        assertChipStackLockedByCurrentThread();

        Clock::Timeout remainingTime = GetRemainingTime(onComplete, appState);
        if (remainingTime.count() < delay.count()) {
            return StartTimer(delay, onComplete, appState);
        }

        return CHIP_NO_ERROR;
    }

    bool LayerImplDispatch::IsTimerActive(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        bool timerIsActive = (GetRemainingTime(onComplete, appState) > Clock::kZero);

#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (!timerIsActive) {
            // check if the timer is in the mExpiredTimers list about to be fired.
            for (TimerList::Node * timer = mExpiredTimers.Earliest(); timer != nullptr; timer = timer->mNextTimer) {
                if (timer->GetCallback().GetOnComplete() == onComplete && timer->GetCallback().GetAppState() == appState) {
                    return true;
                }
            }
        }
#endif

        return timerIsActive;
    }

    Clock::Timeout LayerImplDispatch::GetRemainingTime(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        dispatch_time_t walltime = DISPATCH_WALLTIME_NOW;
        const TimerData * data = FindTimerData(onComplete, appState);
        if (data && walltime < data->walltime) {
            return std::chrono::duration_cast<Clock::Timeout>(std::chrono::duration<uint64_t, std::nano>(data->walltime - walltime));
        }

#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return mTimerList.GetRemainingTime(onComplete, appState);
#else
        return Clock::kZero;
#endif
    }

    void LayerImplDispatch::CancelTimer(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mLayerState.IsInitialized());

        auto timerSource = std::find_if(mTimers.cbegin(), mTimers.cend(), [onComplete, appState](dispatch_source_t timerSource) {
            const TimerData * data = TimerData::Get(timerSource);
            return data->onComplete == onComplete && data->appState == appState;
        });
        if (timerSource != mTimers.cend()) {
            dispatch_source_t t = *timerSource;
            mTimers.erase(timerSource);
            DisableTimer(__func__, t);
        } else {
#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
            __auto_type * timer = mTimerList.Remove(onComplete, appState);
            if (timer == nullptr) {
                // The timer was not in our "will fire in the future" list, but it might
                // be in the "we're about to fire these" chunk we already grabbed from
                // that list.  Check for it there too, and if found there we still want
                // to cancel it.
                timer = mExpiredTimers.Remove(onComplete, appState);
            }
            VerifyOrReturn(timer != nullptr);

            mTimerPool.Release(timer);
#endif
        }
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
        std::vector<dispatch_block_t> queuedBlocks;
        {
            std::lock_guard<std::mutex> lock(mTestQueueMutex);
            queuedBlocks.swap(mTestQueuedBlocks);
        }

        for (auto & block : queuedBlocks) {
            __auto_type * ctx = new TimerCompleteBlockCallbackContext { .block = block };
            VerifyOrDie(nullptr != ctx);
            CHIP_ERROR error = ScheduleWork(TimerCompleteBlockCallback, ctx);
            LogErrorOnFailure(error);
            VerifyOrDo(CHIP_NO_ERROR == error, delete ctx);
        }

        // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
        // since that could result in infinite handling of new timers blocking any other progress.
        VerifyOrDieWithMsg(mExpiredTimers.Empty(), DeviceLayer, "Re-entry into HandleEvents from a timer callback?");
        mExpiredTimers = mTimerList.ExtractEarlier(Clock::Timeout(1) + SystemClock().GetMonotonicTimestamp());
        TimerList::Node * timer = nullptr;
        while ((timer = mExpiredTimers.PopEarliest()) != nullptr) {
            TimerCompleteBlockCallbackContext * context = nullptr;
            __auto_type & cb = timer->GetCallback();
            if (cb.GetOnComplete() == TimerCompleteBlockCallback) {
                context = static_cast<TimerCompleteBlockCallbackContext *>(cb.GetAppState());
            }

            mTimerPool.Invoke(timer);

            delete context;
        }
#endif
    }
} // namespace System
} // namespace chip
