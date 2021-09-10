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
#include <system/LwIPEventSupport.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplLwIP.h>

namespace chip {
namespace System {

LayerLwIP::EventHandlerDelegate LayerImplLwIP::sSystemEventHandlerDelegate;

LayerImplLwIP::LayerImplLwIP() : mHandlingTimerComplete(false), mEventDelegateList(nullptr)
{
    if (!sSystemEventHandlerDelegate.IsInitialized())
        sSystemEventHandlerDelegate.Init(HandleSystemLayerEvent);
}

CHIP_ERROR LayerImplLwIP::Init()
{
    VerifyOrReturnError(!mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    RegisterLwIPErrorFormatter();

    AddEventHandlerDelegate(sSystemEventHandlerDelegate);
    ReturnErrorOnFailure(mTimerList.Init());

    VerifyOrReturnError(mLayerState.Init(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLwIP::Shutdown()
{
    VerifyOrReturnError(mLayerState.Shutdown(), CHIP_ERROR_INCORRECT_STATE);
    mLayerState.Reset(); // Return to uninitialized state to permit re-initialization.
    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplLwIP::StartTimer(uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delayMilliseconds = 0);

    CancelTimer(onComplete, appState);

    Timer * timer = Timer::New(*this, delayMilliseconds, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // this is the new earliest timer and so the timer needs (re-)starting provided that
        // the system is not currently processing expired timers, in which case it is left to
        // HandleExpiredTimers() to re-start the timer.
        if (!mHandlingTimerComplete)
        {
            StartPlatformTimer(delayMilliseconds);
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

    Timer * timer = Timer::New(*this, 0, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    return PostEvent(*timer, chip::System::kEvent_ScheduleWork, 0);
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

/**
 * This is the dispatch handler for system layer events.
 *
 *  @param[in,out]  aTarget     A pointer to the CHIP System Layer object making the post request.
 *  @param[in]      aEventType  The type of event to post.
 *  @param[in,out]  aArgument   The argument associated with the event to post.
 */
CHIP_ERROR LayerImplLwIP::HandleSystemLayerEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    // Dispatch logic specific to the event type
    switch (aEventType)
    {
    case kEvent_ReleaseObj:
        aTarget.Release();
        return CHIP_NO_ERROR;

    case kEvent_ScheduleWork:
        static_cast<Timer &>(aTarget).HandleComplete();
        return CHIP_NO_ERROR;

    default:
        return CHIP_ERROR_UNEXPECTED_EVENT;
    }
}

CHIP_ERROR LayerImplLwIP::AddEventHandlerDelegate(EventHandlerDelegate & aDelegate)
{
    LwIPEventHandlerDelegate lDelegate = static_cast<LwIPEventHandlerDelegate &>(aDelegate);
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
 * This is a syntactic wrapper around a platform-specific hook that effects an event loop, waiting on a queue that services this
 * instance, pulling events off of that queue, and then dispatching them for handling.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
CHIP_ERROR LayerImplLwIP::DispatchEvents()
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return PlatformEventing::DispatchEvents(*this);
}

/**
 * This dispatches the specified event for handling by this instance.
 *
 * The unmarshalling of the type and arguments from the event is handled by a platform-specific hook which should then call
 * back to Layer::HandleEvent for the actual dispatch.
 *
 *  @param[in]  aEvent  The platform-specific event object to dispatch for handling.
 *
 * @return CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
CHIP_ERROR LayerImplLwIP::DispatchEvent(Event aEvent)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return PlatformEventing::DispatchEvent(*this, aEvent);
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
 *  @param[in]  aDelayMilliseconds  The timer duration in milliseconds.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR LayerImplLwIP::StartPlatformTimer(uint32_t aDelayMilliseconds)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR status = PlatformEventing::StartTimer(*this, aDelayMilliseconds);
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
    // regardless how long the processing of the currently expired timers took
    Clock::MonotonicMilliseconds currentTime = Clock::GetMonotonicMilliseconds();

    // limit the number of timers handled before the control is returned to the event queue.  The bound is similar to
    // (though not exactly same) as that on the sockets-based systems.

    // The platform timer API has MSEC resolution so expire any timer with less than 1 msec remaining.
    size_t timersHandled = 0;
    Timer * timer        = nullptr;
    while ((timersHandled < CHIP_SYSTEM_CONFIG_NUM_TIMERS) && ((timer = mTimerList.PopIfEarlier(currentTime + 1)) != nullptr))
    {
        mHandlingTimerComplete = true;
        timer->HandleComplete();
        mHandlingTimerComplete = false;

        timersHandled++;
    }

    if (!mTimerList.Empty())
    {
        // timers still exist so restart the platform timer.
        uint64_t delayMilliseconds = 0ULL;

        currentTime = Clock::GetMonotonicMilliseconds();

        // the next timer expires in the future, so set the delayMilliseconds to a non-zero value
        if (currentTime < mTimerList.Earliest()->mAwakenTime)
        {
            delayMilliseconds = mTimerList.Earliest()->mAwakenTime - currentTime;
        }
        /*
         * StartPlatformTimer() accepts a 32bit value in milliseconds. Timestamps are 64bit numbers. The only way in which this
         * could overflow is if time went backwards (e.g. as a result of a time adjustment from time synchronization).  Verify
         * that the timer can still be executed (even if it is very late) and exit if that is the case.  Note: if the time sync
         * ever ends up adjusting the clock, we should implement a method that deals with all the timers in the system.
         */
        VerifyOrDie(delayMilliseconds <= UINT32_MAX);

        StartPlatformTimer(static_cast<uint32_t>(delayMilliseconds));
    }

    return CHIP_NO_ERROR;
}

} // namespace System
} // namespace chip
