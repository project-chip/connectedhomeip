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

#include "system/SystemClock.h"
#include <algorithm>
#include <chrono>
#include <dispatch/dispatch.h>
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
// Certain unit tests are executed without a main dispatch queue. When
// CONFIG_BUILD_FOR_HOST_UNIT_TEST is defined, we will create a serial
// dispatch queue and immediately suspend it. We will then use the normal
// dispatch-based timer code, but manually gather and run timers from
// HandleDispatchQueueEvents().

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

    // These are not inlined in the header, because we need ARC to be enabled, and that header is included in files that don't have it enabled.
    LayerImplDispatch::TimerData::TimerData(dispatch_source_t timerSource, Clock::Timestamp awakenTime, TimerCompleteCallback onComplete, void * appState)
        : timerSource(timerSource)
        , awakenTime(awakenTime)
        , onComplete(onComplete)
        , appState(appState)
    {
    }
    LayerImplDispatch::TimerData::~TimerData() {}

    void LayerImplDispatch::EnableTimer(const char * source, dispatch_source_t timerSource)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (%s) : source=%p", __func__, source, timerSource);
#endif
        dispatch_resume(timerSource);
    }

    void LayerImplDispatch::DisableTimer(const char * source, dispatch_source_t timerSource)
    {
        bool isCancelled = dispatch_source_testcancel(timerSource);

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

        for (const TimerData & timer : mTimers) {
            DisableTimer(__func__, timer.timerSource);
        };
        mTimers.clear();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (mHasSuspendedDispatchQueue) {
            dispatch_resume(mDispatchQueue);
            mDispatchQueue = nullptr;
            mHasSuspendedDispatchQueue = false;
        }
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

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (mHasSuspendedDispatchQueue) {
            std::lock_guard<std::mutex> lock(mTestQueueMutex);
            mTestQueuedBlocks.emplace_back(block);
            return CHIP_NO_ERROR;
        }
#endif

        __auto_type dispatchQueue = GetDispatchQueue();
        dispatch_async(dispatchQueue, ^{
            block();
        });
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

        return StartTimer(delay, TimerCompleteBlockCallback, ctx, false);
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
        __auto_type timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, dispatchQueue);
        VerifyOrDie(timerSource != nullptr);
        mTimers.emplace_back(timerSource, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);

        // The timers are supposed to use a monotonic time that continues ticking during sleep.
        // Ideally we'd use DISPATCH_MONOTONICTIME_NOW, but that's not available yet.
        // DISPATCH_TIME_NOW is monotonic, but doesn't continue ticking during sleep.
        dispatch_source_set_timer(timerSource, dispatch_time(DISPATCH_WALLTIME_NOW, std::chrono::nanoseconds(Clock::Milliseconds64(delay)).count()), DISPATCH_TIME_FOREVER, std::chrono::nanoseconds(2).count());

        LayerImplDispatch * self = this;
        dispatch_source_set_event_handler(timerSource, ^{
            mTimers.erase(std::find_if(mTimers.begin(), mTimers.end(), [onComplete, appState](const TimerData & data) { return data.onComplete == onComplete && data.appState == appState; }));
            onComplete(self, appState);
        });

        if (onComplete == TimerCompleteBlockCallback) {
            dispatch_source_set_cancel_handler(timerSource, ^{
                delete static_cast<TimerCompleteBlockCallbackContext *>(appState);
            });
        }

        EnableTimer(__func__, timerSource);

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
        return (GetRemainingTime(onComplete, appState) > Clock::kZero);
    }

    Clock::Timeout LayerImplDispatch::GetRemainingTime(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        const Clock::Timestamp time = SystemClock().GetMonotonicTimestamp();
        auto data = std::find_if(mTimers.cbegin(), mTimers.cend(), [onComplete, appState](const TimerData & data) {
            return data.onComplete == onComplete && data.appState == appState;
        });
        if (data != mTimers.cend() && time < data->awakenTime) {
            return data->awakenTime - time;
        }

        return Clock::kZero;
    }

    std::vector<LayerImplDispatch::TimerData>::const_iterator LayerImplDispatch::CancelTimer(std::vector<TimerData> & timers, TimerCompleteCallback onComplete, void * appState)
    {
        auto data = std::find_if(timers.cbegin(), timers.cend(), [onComplete, appState](const TimerData & data) {
            return data.onComplete == onComplete && data.appState == appState;
        });
        if (data != timers.cend()) {
            LayerImplDispatch::DisableTimer(__func__, data->timerSource);
        }
        return data;
    }

    void LayerImplDispatch::CancelTimer(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mLayerState.IsInitialized());

        auto data = CancelTimer(mTimers, onComplete, appState);
        if (data != mTimers.cend()) {
            mTimers.erase(data);
            return;
        }
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        // We need mExpiredTimers to be stable while we're running expired
        // timers, so we don't erase here but instead clear it after running
        // all the timers in it.
        CancelTimer(mExpiredTimers, onComplete, appState);
#endif
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

        VerifyOrDie(mExpiredTimers.empty());

        // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
        // since that could result in infinite handling of new timers blocking any other progress.
        const Clock::Timestamp time
            = chip::System::SystemClock().GetMonotonicTimestamp() + Clock::Timeout(1);
        for (auto data = mTimers.begin(); data != mTimers.end();) {
            if (data->awakenTime < time) {
                mExpiredTimers.insert(std::upper_bound(mExpiredTimers.begin(), mExpiredTimers.end(), data->awakenTime, [](const Clock::Timestamp a, const TimerData & b) {
                    return a < b.awakenTime;
                }), std::move(*data));
                mTimers.erase(data);
            } else {
                ++data;
            }
        }
        for (const TimerData & d : mExpiredTimers) {
            if (!dispatch_source_testcancel(d.timerSource)) {
                d.onComplete(this, d.appState);
            }
        }
        mExpiredTimers.clear();
#endif
    }
} // namespace System
} // namespace chip
