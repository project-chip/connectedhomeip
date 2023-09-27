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
 *      This file implements LayerImplRTThread.
 */

#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include "SystemLayerImplRTThread.h"

namespace chip {
namespace System {

LayerImplRTThread::LayerImplRTThread() : mHandlingTimerComplete(false) {}

CHIP_ERROR LayerImplRTThread::Init()
{
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    // Perform any platform-specific initialization here if needed.
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    RegisterLwIPErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void LayerImplRTThread::Shutdown()
{
    if (mLayerState.IsInitialized())
    {
        // Perform any platform-specific shutdown here if needed.

        mLayerState.ResetFromInitialized();
    }
}

bool LayerImplRTThread::IsInitialized() const
{
    return mLayerState.IsInitialized();
}

CHIP_ERROR LayerImplRTThread::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = Clock::kZero);

    CancelTimer(onComplete, appState);

    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // this is the new earliest timer and so the timer needs (re-)starting provided that
        // the system is not currently processing expired timers, in which case it is left to
        // HandleExpiredTimers() to re-start the timer.
        if (!mHandlingTimerComplete)
        {
            StartPlatformTimer(delay);
        }
    }
    return CHIP_NO_ERROR;
}

void LayerImplRTThread::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturn(mLayerState.IsInitialized());

    TimerList::Node * timer = mTimerList.Remove(onComplete, appState);
    if (timer != nullptr)
    {
        mTimerPool.Release(timer);
    }
}

CHIP_ERROR LayerImplRTThread::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
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
    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp(), onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = ScheduleLambda([this, timer] { this->mTimerPool.Invoke(timer); });
    if (err != CHIP_NO_ERROR)
    {
        mTimerPool.Release(timer);
    }
    return err;
}

/**
 * Handle the platform timer expiration event. Completes any timers that have expired.
 *
 * A static API that gets called when the platform timer expires. Any expired timers are completed and removed from the list
 * of active timers in the layer object. If unexpired timers remain on completion, StartPlatformTimer will be called to
 * restart the platform timer.
 *
 * It is assumed that this API is called only while on the thread which owns the CHIP System Layer object.
 *
 * @note
 *      It's harmless if this API gets called and there are no expired timers.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR LayerImplRTThread::HandlePlatformTimer(void)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Expire each timer in turn until an unexpired timer is reached or the timerlist is emptied.  We set the current expiration
    // time outside the loop; that way timers set after the current tick will not be executed within this expiration window
    // regardless how long the processing of the currently expired timers took.
    // The platform timer API has MSEC resolution so expire any timer with less than 1 msec remaining.
    Clock::Timestamp expirationTime = SystemClock().GetMonotonicTimestamp() + Clock::Timeout(1);

    // limit the number of timers handled before the control is returned to the event queue.  The bound is similar to
    // (though not exactly same) as that on the sockets-based systems.

    size_t timersHandled    = 0;
    TimerList::Node * timer = nullptr;
    while ((timersHandled < CHIP_SYSTEM_CONFIG_NUM_TIMERS) && ((timer = mTimerList.PopIfEarlier(expirationTime)) != nullptr))
    {
        mHandlingTimerComplete = true;
        mTimerPool.Invoke(timer);
        mHandlingTimerComplete = false;
        timersHandled++;
    }

    if (!mTimerList.Empty())
    {
        // timers still exist so restart the platform timer.
        Clock::Timeout delay = System::Clock::kZero;

        Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();

        if (currentTime < mTimerList.Earliest()->AwakenTime())
        {
            // the next timer expires in the future, so set the delay to a non-zero value
            delay = mTimerList.Earliest()->AwakenTime() - currentTime;
        }

        StartPlatformTimer(delay);
    }

    return CHIP_NO_ERROR;
}

/**
 * Start the platform timer with specified millsecond duration.
 */
CHIP_ERROR LayerImplRTThread::StartPlatformTimer(System::Clock::Timeout aDelay)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR status = PlatformEventing::StartTimer(*this, aDelay);
    return status;
}

} // namespace System
} // namespace chip
