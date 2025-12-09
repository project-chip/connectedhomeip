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
 *      This file implements System::Layer dedicated to Zephyr RTOS, which does not use select().
 */

#include <lib/support/CodeUtils.h>
#include <platform/LockTracker.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplZephyr.h>

namespace chip {
namespace System {

LayerImplZephyr::LayerImplZephyr() {}

CHIP_ERROR LayerImplZephyr::Init()
{
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void LayerImplZephyr::Shutdown()
{
    mLayerState.ResetFromInitialized();
}

CHIP_ERROR LayerImplZephyr::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = System::Clock::kZero);

    CancelTimer(onComplete, appState);

    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        TriggerPlatformTimerUpdate();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplZephyr::ExtendTimerTo(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(delay.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);

    assertChipStackLockedByCurrentThread();

    Clock::Timeout remainingTime = mTimerList.GetRemainingTime(onComplete, appState);
    if (remainingTime.count() < delay.count())
    {
        return StartTimer(delay, onComplete, appState);
    }

    return CHIP_NO_ERROR;
}

bool LayerImplZephyr::IsTimerActive(TimerCompleteCallback onComplete, void * appState)
{
    bool timerIsActive = (mTimerList.GetRemainingTime(onComplete, appState) > Clock::kZero);

    if (!timerIsActive)
    {
        // check if the timer is in the mExpiredTimers list about to be fired.
        for (TimerList::Node * timer = mExpiredTimers.Earliest(); timer != nullptr; timer = timer->mNextTimer)
        {
            if (timer->GetCallback().GetOnComplete() == onComplete && timer->GetCallback().GetAppState() == appState)
            {
                return true;
            }
        }
    }

    return timerIsActive;
}

Clock::Timeout LayerImplZephyr::GetRemainingTime(TimerCompleteCallback onComplete, void * appState)
{
    return mTimerList.GetRemainingTime(onComplete, appState);
}

void LayerImplZephyr::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mLayerState.IsInitialized());

    TimerList::Node * timer = mTimerList.Remove(onComplete, appState);
    if (timer == nullptr)
    {
        // The timer was not in our "will fire in the future" list, but it might
        // be in the "we're about to fire these" chunk we already grabbed from
        // that list.  Check for it there too, and if found there we still want
        // to cancel it.
        timer = mExpiredTimers.Remove(onComplete, appState);
    }

    VerifyOrReturn(timer != nullptr);
    mTimerPool.Release(timer);
}

CHIP_ERROR LayerImplZephyr::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Ideally we would not use a timer here at all, but if we try to just
    // ScheduleLambda the lambda needs to capture the following:
    // 1) onComplete
    // 2) appState
    // 3) The `this` pointer, because onComplete needs to be passed a pointer to
    //    the System::Layer.
    //
    // On a 64-bit system that's 24 bytes, but lambdas passed to ScheduleLambda
    // are capped at CHIP_CONFIG_LAMBDA_EVENT_SIZE which is 16 bytes.
    //
    // So for now use a timer as a poor-man's closure that captures `this` and
    // onComplete and appState in a single pointer, so we fit inside the size
    // limit.
    //
    // TODO: We could do something here where we compile-time condition on the
    // sizes of things and use a direct ScheduleLambda if it would fit and this
    // setup otherwise.
    //
    // TODO: But also, unit tests seem to do SystemLayer::ScheduleWork without
    // actually running a useful event loop (in the PlatformManager sense),
    // which breaks if we use ScheduleLambda here, since that does rely on the
    // PlatformManager event loop. So for now, keep scheduling an expires-ASAP
    // timer, but just make sure we don't cancel existing timers with the same
    // callback and appState, so ScheduleWork invocations don't stomp on each
    // other.
    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp(), onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        TriggerPlatformTimerUpdate();
    }

    return CHIP_NO_ERROR;
}

void LayerImplZephyr::TriggerPlatformTimerUpdate()
{
    CHIP_ERROR status = PlatformEventing::StartTimer(*this, Clock::kZero);
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error starting platform timer: %" CHIP_ERROR_FORMAT, status.Format());
    }
}

/**
 * Get the next awaken time from the timer list.
 *
 * @return The next awaken time or nullopt if there are no timers in the list.
 */
std::optional<Clock::Timestamp> LayerImplZephyr::GetNextAwakenTime()
{
    TimerList::Node * timer = mTimerList.Earliest();
    if (timer)
    {
        return timer->AwakenTime();
    }

    return std::nullopt;
}

/**
 * Handle the platform timer expiration event. Completes any timers that have expired.
 *
 * A static API that gets called when the platform timer expires. Any expired timers are completed and removed from the list
 * of active timers in the layer object.
 *
 * It is assumed that this API is called only while on the thread which owns the CHIP System Layer object.
 *
 * @note
 *      It's harmless if this API gets called and there are no expired timers.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR LayerImplZephyr::HandlePlatformTimer()
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Obtain the list of currently expired timers. Any new timers added by timer callback are NOT handled on this pass,
    // since that could result in infinite handling of new timers blocking any other progress.
    VerifyOrDieWithMsg(mExpiredTimers.Empty(), DeviceLayer, "Re-entry into HandleEvents from a timer callback?");
    Clock::Timestamp now    = SystemClock().GetMonotonicTimestamp();
    mExpiredTimers          = mTimerList.ExtractEarlier(Clock::Timeout(1) + now);
    TimerList::Node * timer = nullptr;
    while ((timer = mExpiredTimers.PopEarliest()) != nullptr)
    {
        mTimerPool.Invoke(timer);
    }

    return CHIP_NO_ERROR;
}

} // namespace System
} // namespace chip
