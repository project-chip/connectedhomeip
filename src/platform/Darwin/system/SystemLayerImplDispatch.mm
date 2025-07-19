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

        for (const TimerData & timer : mTimers) {
            DisableTimer(__func__, timer.timerSource);
        };
        mTimers.clear();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        if (mHasSuspendedDispatchQueue) {
            dispatch_resume(mDispatchQueue);
            mHasSuspendedDispatchQueue = false;
        }
#endif
        mDispatchQueue = nullptr;

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

        dispatch_time_t walltime = dispatch_time(DISPATCH_WALLTIME_NOW, std::chrono::nanoseconds(Clock::Milliseconds64(delay)).count());

        mTimers.emplace_back(timerSource, walltime, onComplete, appState);

        dispatch_source_set_timer(timerSource, walltime, DISPATCH_TIME_FOREVER, std::chrono::nanoseconds(2).count());

        LayerImplDispatch * self = this;
        dispatch_source_set_event_handler(timerSource, ^{
            auto data = std::find_if(mTimers.begin(), mTimers.end(), [timerSource](const TimerData & data) { return data.timerSource == timerSource; });
            TimerCompleteCallback callback = data->onComplete;
            void * state = data->appState;
            mTimers.erase(data);
            callback(self, state);
        });

        dispatch_source_set_cancel_handler(timerSource, ^{
            auto data = std::find_if(mTimers.begin(), mTimers.end(), [timerSource](const TimerData & data) { return data.timerSource == timerSource; });
            if (data->onComplete == TimerCompleteBlockCallback) {
                delete static_cast<TimerCompleteBlockCallbackContext *>(data->appState);
            }
        });

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
        dispatch_time_t walltime = dispatch_time(DISPATCH_WALLTIME_NOW, 0);
        const TimerData * data = FindTimerData(onComplete, appState);
        if (data && walltime < data->walltime) {
            return std::chrono::duration_cast<Clock::Timeout>(std::chrono::duration<uint64_t, std::nano>(data->walltime - walltime));
        }

        return Clock::kZero;
    }

    void LayerImplDispatch::CancelTimer(TimerCompleteCallback onComplete, void * appState)
    {
#if SYSTEM_LAYER_IMPL_DISPATCH_DEBUG
        ChipLogError(Inet, "%s (onComplete: %p - appState: %p)", __func__, onComplete, appState);
#endif
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mLayerState.IsInitialized());

        auto data = std::find_if(mTimers.cbegin(), mTimers.cend(), [onComplete, appState](const TimerData & data) {
            return data.onComplete == onComplete && data.appState == appState;
        });
        if (data != mTimers.cend()) {
            DisableTimer(__func__, data->timerSource);
            mTimers.erase(data);
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

        std::vector<TimerData> timerData;
        dispatch_time_t walltime = dispatch_time(DISPATCH_WALLTIME_NOW, 0);
        for (auto data = mTimers.begin(); data != mTimers.end();) {
            if (walltime >= data->walltime) {
                timerData.insert(std::lower_bound(timerData.begin(), timerData.end(), data->walltime, [](const TimerData & a, dispatch_time_t b) {
                    return a.walltime < b;
                }), std::move(*data));
                mTimers.erase(data);
            } else {
                ++data;
            }
        }
        for (const TimerData & d : timerData) {
            d.onComplete(this, d.appState);
        }
#endif
    }
} // namespace System
} // namespace chip
