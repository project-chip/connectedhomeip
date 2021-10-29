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
 *      This file implements LayerImplLwIP using LwIP.
 */

#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplLwIP.h>

namespace chip {
namespace System {

LayerImplLwIP::LayerImplLwIP() : mHandlingTimerComplete(false), mEventDelegateList(nullptr) {}

CHIP_ERROR LayerImplLwIP::Init()
{
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    RegisterLwIPErrorFormatter();

    ReturnErrorOnFailure(mTimerList.Init());

    VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLwIP::Shutdown()
{
    VerifyOrReturnError(mLayerState.SetShuttingDown(), CHIP_ERROR_INCORRECT_STATE);
    mLayerState.SetShutdown();
    mLayerState.Reset(); // Return to uninitialized state to permit re-initialization.
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLwIP::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = Clock::Zero);

    CancelTimer(onComplete, appState);

    Timer * timer = Timer::New(*this, delay, onComplete, appState);
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

void LayerImplLwIP::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturn(mLayerState.IsInitialized());

    Timer * timer = mTimerList.Remove(onComplete, appState);
    VerifyOrReturn(timer != nullptr);

    timer->Clear();
    timer->Release();
}

CHIP_ERROR LayerImplLwIP::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    Timer * timer = Timer::New(*this, System::Clock::Zero, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    return ScheduleLambda([timer] { timer->HandleComplete(); });
}

bool LayerLwIP::EventHandlerDelegate::IsInitialized() const
{
    return mFunction != nullptr;
}

void LayerLwIP::EventHandlerDelegate::Init(EventHandlerFunction aFunction)
{
    mFunction     = aFunction;
    mNextDelegate = nullptr;
}

void LayerLwIP::EventHandlerDelegate::Prepend(const LayerLwIP::EventHandlerDelegate *& aDelegateList)
{
    mNextDelegate = aDelegateList;
    aDelegateList = this;
}

CHIP_ERROR LayerImplLwIP::AddEventHandlerDelegate(EventHandlerDelegate & aDelegate)
{
    LwIPEventHandlerDelegate & lDelegate = static_cast<LwIPEventHandlerDelegate &>(aDelegate);
    VerifyOrReturnError(lDelegate.GetFunction() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    lDelegate.Prepend(mEventDelegateList);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLwIP::PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR lReturn = PlatformEventing::PostEvent(*this, aTarget, aEventType, aArgument);
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "Failed to queue CHIP System Layer event (type %d): %s", aEventType, ErrorStr(lReturn));
    }
    return lReturn;
}

/**
 * This implements the actual dispatch and handling of a CHIP System Layer event.
 *
 *  @param[in,out]  aTarget     A reference to the layer object to which the event is targeted.
 *  @param[in]      aEventType  The event / message type to handle.
 *  @param[in]      aArgument   The argument associated with the event / message.
 *
 *  @retval   CHIP_NO_ERROR                 On success.
 *  @retval   CHIP_ERROR_INCORRECT_STATE    If the state of the InetLayer object is incorrect.
 *  @retval   CHIP_ERROR_UNEXPECTED_EVENT   If the event type is unrecognized.
 */
CHIP_ERROR LayerImplLwIP::HandleEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Prevent the target object from being freed while dispatching the event.
    aTarget.Retain();

    CHIP_ERROR lReturn                              = CHIP_ERROR_UNEXPECTED_EVENT;
    const LwIPEventHandlerDelegate * lEventDelegate = static_cast<const LwIPEventHandlerDelegate *>(mEventDelegateList);

    while (lReturn == CHIP_ERROR_UNEXPECTED_EVENT && lEventDelegate != nullptr)
    {
        lReturn        = lEventDelegate->GetFunction()(aTarget, aEventType, aArgument);
        lEventDelegate = lEventDelegate->GetNextDelegate();
    }

    if (lReturn == CHIP_ERROR_UNEXPECTED_EVENT)
    {
        ChipLogError(chipSystemLayer, "Unexpected event type %d", aEventType);
    }

    /*
      Release the reference to the target object. When the object's lifetime finally comes to an end, in most cases this will be
      the release call that decrements the ref count to zero.
      */
    aTarget.Release();

    return lReturn;
}

/**
 * Start the platform timer with specified millsecond duration.
 *
 *  @brief
 *      Calls the Platform specific API to start a platform timer. This API is called by the chip::System::Timer class when
 *      one or more timers are active and require deferred execution.
 *
 *  @param[in]  aDelay  The timer duration in milliseconds.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR LayerImplLwIP::StartPlatformTimer(System::Clock::Timeout aDelay)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR status = PlatformEventing::StartTimer(*this, aDelay);
    return status;
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
CHIP_ERROR LayerImplLwIP::HandlePlatformTimer()
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Expire each timer in turn until an unexpired timer is reached or the timerlist is emptied.  We set the current expiration
    // time outside the loop; that way timers set after the current tick will not be executed within this expiration window
    // regardless how long the processing of the currently expired timers took.
    // The platform timer API has MSEC resolution so expire any timer with less than 1 msec remaining.
    Clock::Timestamp expirationTime = SystemClock().GetMonotonicTimestamp() + Clock::Timeout(1);

    // limit the number of timers handled before the control is returned to the event queue.  The bound is similar to
    // (though not exactly same) as that on the sockets-based systems.

    size_t timersHandled = 0;
    Timer * timer        = nullptr;
    while ((timersHandled < CHIP_SYSTEM_CONFIG_NUM_TIMERS) && ((timer = mTimerList.PopIfEarlier(expirationTime)) != nullptr))
    {
        mHandlingTimerComplete = true;
        timer->HandleComplete();
        mHandlingTimerComplete = false;

        timersHandled++;
    }

    if (!mTimerList.Empty())
    {
        // timers still exist so restart the platform timer.
        Clock::Timeout delay{ 0 };

        Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();

        if (currentTime < mTimerList.Earliest()->mAwakenTime)
        {
            // the next timer expires in the future, so set the delay to a non-zero value
            delay = mTimerList.Earliest()->mAwakenTime - currentTime;
        }

        StartPlatformTimer(delay);
    }

    return CHIP_NO_ERROR;
}

} // namespace System
} // namespace chip
