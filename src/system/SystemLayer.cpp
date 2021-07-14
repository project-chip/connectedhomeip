/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file contains definitions of the chip::System::Layer
 *      class methods and related data and functions.
 */

// Include module header
#include <system/SystemLayer.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

// Include additional CHIP headers
#include <platform/LockTracker.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>

// Include system and language headers
#include <stddef.h>
#include <string.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
#include <lwip/err.h>
#include <lwip/sys.h>
#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

namespace chip {
namespace System {

namespace {

Clock::MonotonicMilliseconds GetTimestamp(const Callback::Cancelable * timer)
{
    Clock::MonotonicMilliseconds timestamp;
    static_assert(sizeof(timestamp) <= sizeof(timer->mInfo), "mInfo is too small for timestamp");
    memcpy(&timestamp, &timer->mInfo, sizeof(timestamp));
    return timestamp;
}

void SetTimestamp(Callback::Cancelable * timer, Clock::MonotonicMilliseconds timestamp)
{
    static_assert(sizeof(timestamp) <= sizeof(timer->mInfo), "mInfo is too small for timestamp");
    memcpy(&timer->mInfo, &timestamp, sizeof(timestamp));
}

bool TimerReady(const Clock::MonotonicMilliseconds timestamp, const Callback::Cancelable * timer)
{
    return !Timer::IsEarlier(timestamp, GetTimestamp(timer));
}

int TimerCompare(void * p, const Callback::Cancelable * a, const Callback::Cancelable * b)
{
    (void) p;

    Clock::MonotonicMilliseconds timeA = GetTimestamp(a);
    Clock::MonotonicMilliseconds timeB = GetTimestamp(b);

    return (timeA > timeB) ? 1 : (timeA < timeB) ? -1 : 0;
}

} // namespace

using namespace ::chip::Callback;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
bool LwIPEventHandlerDelegate::IsInitialized() const
{
    return this->mFunction != NULL;
}

void LwIPEventHandlerDelegate::Init(LwIPEventHandlerFunction aFunction)
{
    this->mFunction     = aFunction;
    this->mNextDelegate = NULL;
}

void LwIPEventHandlerDelegate::Prepend(const LwIPEventHandlerDelegate *& aDelegateList)
{
    this->mNextDelegate = aDelegateList;
    aDelegateList       = this;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

Layer::Layer() : mLayerState(kLayerState_NotInitialized), mContext(nullptr), mPlatformData(nullptr)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (!sSystemEventHandlerDelegate.IsInitialized())
        sSystemEventHandlerDelegate.Init(HandleSystemLayerEvent);

    this->mEventDelegateList = NULL;
    this->mTimerList         = NULL;
    this->mTimerComplete     = false;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
}

CHIP_ERROR Layer::Init(void * aContext)
{
    CHIP_ERROR lReturn;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    RegisterPOSIXErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    RegisterLwIPErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    if (this->mLayerState != kLayerState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    lReturn = Platform::Layer::WillInit(*this, aContext);
    SuccessOrExit(lReturn);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    mWatchableEvents.Init(*this);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    this->AddEventHandlerDelegate(sSystemEventHandlerDelegate);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    // Create an event to allow an arbitrary thread to wake the thread in the select loop.
    lReturn = this->mWakeEvent.Open(mWatchableEvents);
    SuccessOrExit(lReturn);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    this->mLayerState = kLayerState_Initialized;
    this->mContext    = aContext;

exit:
    Platform::Layer::DidInit(*this, aContext, lReturn);
    return lReturn;
}

CHIP_ERROR Layer::Shutdown()
{
    CHIP_ERROR lReturn;
    void * lContext;

    if (this->mLayerState == kLayerState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    lContext = this->mContext;
    lReturn  = Platform::Layer::WillShutdown(*this, lContext);
    SuccessOrExit(lReturn);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    lReturn = mWakeEvent.Close();
    SuccessOrExit(lReturn);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    for (size_t i = 0; i < Timer::sPool.Size(); ++i)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr)
        {
            lTimer->Cancel();
        }
    }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    mWatchableEvents.Shutdown();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    this->mContext    = nullptr;
    this->mLayerState = kLayerState_NotInitialized;

exit:
    Platform::Layer::DidShutdown(*this, lContext, lReturn);
    return lReturn;
}

/**
 * This returns any client-specific platform data assigned to the instance, if it has been previously set.
 *
 * @return Client-specific platform data, if is has been previously set; otherwise, NULL.
 */
void * Layer::GetPlatformData() const
{
    return this->mPlatformData;
}

/**
 * This sets the specified client-specific platform data to the
 * instance for later retrieval by the client platform.
 *
 * @param[in]  aPlatformData  The client-specific platform data to set.
 *
 */
void Layer::SetPlatformData(void * aPlatformData)
{
    this->mPlatformData = aPlatformData;
}

CHIP_ERROR Layer::NewTimer(Timer *& aTimerPtr)
{
    Timer * lTimer = nullptr;

    if (this->State() != kLayerState_Initialized)
        return CHIP_ERROR_INCORRECT_STATE;

    lTimer    = Timer::sPool.TryCreate(*this);
    aTimerPtr = lTimer;

    if (lTimer == nullptr)
    {
        ChipLogError(chipSystemLayer, "Timer pool EMPTY");
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

/**
 * @brief
 *   This method starts a one-shot timer.
 *
 *   @note
 *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
 *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
 *       the currently-running timer will first be cancelled.
 *
 *   @param[in]  aMilliseconds Expiration time in milliseconds.
 *   @param[in]  aCallback     A pointer to the Callback that fires when the timer expires
 *
 *   @return CHIP_NO_ERROR On success.
 *   @return Other Value indicating timer failed to start.
 *
 */
void Layer::StartTimer(uint32_t aMilliseconds, chip::Callback::Callback<> * aCallback)
{
#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (mDispatchQueue != nullptr)
    {
        ChipLogError(chipSystemLayer, "%s is not supported with libdispatch", __PRETTY_FUNCTION__);
        chipDie();
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    assertChipStackLockedByCurrentThread();

    Cancelable * ca = aCallback->Cancel();

    SetTimestamp(ca, Clock::GetMonotonicMilliseconds() + aMilliseconds);

    mTimerCallbacks.InsertBy(ca, TimerCompare, nullptr);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (mTimerCallbacks.First() == ca)
    {
        // this is the new earliest timer and so the timer needs (re-)starting provided that
        // the system is not currently processing expired timers, in which case it is left to
        // HandleExpiredTimers() to re-start the timer.
        if (!mTimerComplete)
        {
            StartPlatformTimer(aMilliseconds);
        }
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * @brief
 *   This method starts a one-shot timer.
 *
 *   @note
 *       Only a single timer is allowed to be started with the same @a aComplete and @a aAppState
 *       arguments. If called with @a aComplete and @a aAppState identical to an existing timer,
 *       the currently-running timer will first be cancelled.
 *
 *   @param[in]  aMilliseconds Expiration time in milliseconds.
 *   @param[in]  aComplete     A pointer to the function called when timer expires.
 *   @param[in]  aAppState     A pointer to the application state object used when timer expires.
 *
 *   @return CHIP_NO_ERROR On success.
 *   @return CHIP_ERROR_NO_MEMORY If a timer cannot be allocated.
 *   @return Other Value indicating timer failed to start.
 *
 */
CHIP_ERROR Layer::StartTimer(uint32_t aMilliseconds, TimerCompleteFunct aComplete, void * aAppState)
{
    CHIP_ERROR lReturn;
    Timer * lTimer;

    this->CancelTimer(aComplete, aAppState);
    lReturn = this->NewTimer(lTimer);
    SuccessOrExit(lReturn);

    lReturn = lTimer->Start(aMilliseconds, aComplete, aAppState);
    if (lReturn != CHIP_NO_ERROR)
    {
        lTimer->Release();
    }

exit:
    return lReturn;
}

/**
 * @brief
 *   This method cancels a one-shot timer, started earlier through @p StartTimer().
 *
 *   @note
 *       The cancellation could fail silently in two different ways. If the timer specified by the combination of the callback
 *       function and application state object couldn't be found, cancellation could fail. If the timer has fired, but not yet
 *       removed from memory, cancellation could also fail.
 *
 *   @param[in]  aOnComplete   A pointer to the callback function used in calling @p StartTimer().
 *   @param[in]  aAppState     A pointer to the application state object used in calling @p StartTimer().
 *
 */
void Layer::CancelTimer(Layer::TimerCompleteFunct aOnComplete, void * aAppState)
{
    if (this->State() != kLayerState_Initialized)
        return;

    for (size_t i = 0; i < Timer::sPool.Size(); ++i)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr && lTimer->OnComplete == aOnComplete && lTimer->AppState == aAppState)
        {
            lTimer->Cancel();
            break;
        }
    }
}

/**
 * @brief
 *   Schedules a function with a signature identical to
 *   `TimerCompleteFunct` to be run as soon as possible on the CHIP
 *   thread.
 *
 * @note
 *   This function could, in principle, be implemented as
 *   `StartTimer`.  The specification for
 *   `SystemTimer` however permits certain optimizations that might
 *   make that implementation impossible. Specifically, `SystemTimer`
 *   API may only be called from the thread owning the particular
 *   `SystemLayer`, whereas the `ScheduleWork` may be called from
 *   any thread.  Additionally, whereas the `SystemTimer` API permits
 *   the invocation of the already expired handler in line,
 *   `ScheduleWork` guarantees that the handler function will be
 *   called only after the current CHIP event completes.
 *
 * @param[in] aComplete A pointer to a callback function to be called
 *                      when this timer fires.
 *
 * @param[in] aAppState A pointer to an application state object to be
 *                      passed to the callback function as argument.
 *
 * @retval CHIP_ERROR_INCORRECT_STATE If the SystemLayer has
 *                      not been initialized.
 *
 * @retval CHIP_ERROR_NO_MEMORY If the SystemLayer cannot
 *                      allocate a new timer.
 *
 * @retval CHIP_NO_ERROR On success.
 */
CHIP_ERROR Layer::ScheduleWork(TimerCompleteFunct aComplete, void * aAppState)
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR lReturn;
    Timer * lTimer;

    lReturn = this->NewTimer(lTimer);
    SuccessOrExit(lReturn);

    lReturn = lTimer->ScheduleWork(aComplete, aAppState);
    if (lReturn != CHIP_NO_ERROR)
    {
        lTimer->Release();
    }

exit:
    return lReturn;
}

/**
 * @brief
 *   Run any timers that are due based on input current time
 */
void Layer::DispatchTimerCallbacks(const Clock::MonotonicMilliseconds aCurrentTime)
{
    // dispatch TimerCallbacks
    Cancelable ready;

    mTimerCallbacks.DequeueBy(TimerReady, static_cast<uint64_t>(aCurrentTime), ready);

    while (ready.mNext != &ready)
    {
        // one-shot
        chip::Callback::Callback<> * cb = chip::Callback::Callback<>::FromCancelable(ready.mNext);
        cb->Cancel();
        cb->mCall(cb->mContext);
    }
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

bool Layer::GetTimeout(struct timeval & aSleepTime)
{
    if (this->State() != kLayerState_Initialized)
        return false;

    const Clock::MonotonicMilliseconds kCurrentTime = Clock::GetMonotonicMilliseconds();
    Clock::MonotonicMilliseconds lAwakenTime = kCurrentTime + static_cast<Clock::MonotonicMilliseconds>(aSleepTime.tv_sec) * 1000 +
        static_cast<uint32_t>(aSleepTime.tv_usec) / 1000;

    bool anyTimer = false;
    for (size_t i = 0; i < Timer::sPool.Size(); i++)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr)
        {
            anyTimer = true;

            if (!Timer::IsEarlier(kCurrentTime, lTimer->mAwakenTime))
            {
                lAwakenTime = kCurrentTime;
                break;
            }

            if (Timer::IsEarlier(lTimer->mAwakenTime, lAwakenTime))
                lAwakenTime = lTimer->mAwakenTime;
        }
    }

    // check for an earlier callback timer, too
    if (lAwakenTime != kCurrentTime)
    {
        Cancelable * ca = mTimerCallbacks.First();
        if (ca != nullptr && !Timer::IsEarlier(kCurrentTime, GetTimestamp(ca)))
        {
            anyTimer    = true;
            lAwakenTime = GetTimestamp(ca);
        }
    }

    const Clock::MonotonicMilliseconds kSleepTime = lAwakenTime - kCurrentTime;
    aSleepTime.tv_sec                             = static_cast<time_t>(kSleepTime / 1000);
    aSleepTime.tv_usec                            = static_cast<suseconds_t>((kSleepTime % 1000) * 1000);

    return anyTimer;
}

void Layer::HandleTimeout()
{
    assertChipStackLockedByCurrentThread();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mHandleSelectThread = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    const Clock::MonotonicMilliseconds kCurrentTime = Clock::GetMonotonicMilliseconds();

    for (size_t i = 0; i < Timer::sPool.Size(); i++)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr && !Timer::IsEarlier(kCurrentTime, lTimer->mAwakenTime))
        {
            lTimer->HandleComplete();
        }
    }

    DispatchTimerCallbacks(kCurrentTime);

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_IO_THREAD

/**
 * Wake up the I/O thread by writing a single byte to the wake pipe.
 *
 *  @note
 *      If @p WakeIOThread() is being called from within an I/O event callback, then writing to the wake pipe can be skipped,
 * since the I/O thread is already awake.
 *
 *      Furthermore, we don't care if this write fails as the only reasonably likely failure is that the pipe is full, in which
 *      case the select calling thread is going to wake up anyway.
 */
void Layer::WakeIOThread()
{
    CHIP_ERROR lReturn;

    if (this->State() != kLayerState_Initialized)
        return;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    if (pthread_equal(this->mHandleSelectThread, pthread_self()))
    {
        return;
    }
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    // Send notification to wake up the select call.
    lReturn = this->mWakeEvent.Notify();
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "System wake event notify failed: %s", ErrorStr(lReturn));
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_IO_THREAD

#if CHIP_SYSTEM_CONFIG_USE_LWIP
LwIPEventHandlerDelegate Layer::sSystemEventHandlerDelegate;

/**
 * This is the dispatch handler for system layer events.
 *
 *  @param[in,out]  aTarget     A pointer to the CHIP System Layer object making the post request.
 *  @param[in]      aEventType  The type of event to post.
 *  @param[in,out]  aArgument   The argument associated with the event to post.
 */
CHIP_ERROR Layer::HandleSystemLayerEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    ;

    // Dispatch logic specific to the event type
    switch (aEventType)
    {
    case kEvent_ReleaseObj:
        aTarget.Release();
        break;

    case kEvent_ScheduleWork:
        static_cast<Timer &>(aTarget).HandleComplete();
        break;

    default:
        lReturn = CHIP_ERROR_UNEXPECTED_EVENT;
        break;
    }

    return lReturn;
}

/**
 * This adds an event handler delegate to the system layer to extend its ability to handle LwIP events.
 *
 *  @param[in]  aDelegate   An uninitialied LwIP event handler delegate structure
 *
 *  @retval     CHIP_NO_ERROR                 On success.
 *  @retval     CHIP_ERROR_INVALID_ARGUMENT   If the function pointer contained in aDelegate is NULL
 */
CHIP_ERROR Layer::AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate)
{
    CHIP_ERROR lReturn;

    VerifyOrExit(aDelegate.mFunction != NULL, lReturn = CHIP_ERROR_INVALID_ARGUMENT);
    aDelegate.Prepend(this->mEventDelegateList);
    lReturn = CHIP_NO_ERROR;

exit:
    return lReturn;
}

/**
 * This posts an event / message of the specified type with the provided argument to this instance's platform-specific event
 * queue.
 *
 *  @param[in,out]  aTarget     A pointer to the CHIP System Layer object making the post request.
 *  @param[in]      aEventType  The type of event to post.
 *  @param[in,out]  aArgument   The argument associated with the event to post.
 *
 *  @retval    CHIP_NO_ERROR                  On success.
 *  @retval    CHIP_ERROR_INCORRECT_STATE     If the state of the Layer object is incorrect.
 *  @retval    CHIP_ERROR_NO_MEMORY           If the event queue is already full.
 *  @retval    other Platform-specific errors generated indicating the reason for failure.
 */
CHIP_ERROR Layer::PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    // Sanity check that this instance and the target layer haven't been "crossed".
    VerifyOrDieWithMsg(aTarget.IsRetained(*this), chipSystemLayer, "wrong poster! [target %p != this %p]", &(aTarget.SystemLayer()),
                       this);

    lReturn = Platform::Layer::PostEvent(*this, this->mContext, aTarget, aEventType, aArgument);
    if (lReturn != CHIP_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "Failed to queue CHIP System Layer event (type %d): %s", aEventType, ErrorStr(lReturn));
    }
    SuccessOrExit(lReturn);

exit:
    return lReturn;
}

/**
 * This is a syntactic wrapper around a platform-specific hook that effects an event loop, waiting on a queue that services this
 * instance, pulling events off of that queue, and then dispatching them for handling.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
CHIP_ERROR Layer::DispatchEvents()
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    lReturn = Platform::Layer::DispatchEvents(*this, this->mContext);
    SuccessOrExit(lReturn);

exit:
    return lReturn;
}

/**
 * This dispatches the specified event for handling by this instance.
 *
 *  The unmarshalling of the type and arguments from the event is handled by a platform-specific hook which should then call
 * back to Layer::HandleEvent for the actual dispatch.
 *
 *  @param[in]  aEvent  The platform-specific event object to dispatch for handling.
 *
 * @return CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
CHIP_ERROR Layer::DispatchEvent(Event aEvent)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    lReturn = Platform::Layer::DispatchEvent(*this, this->mContext, aEvent);
    SuccessOrExit(lReturn);

exit:
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
CHIP_ERROR Layer::HandleEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    const LwIPEventHandlerDelegate * lEventDelegate;
    CHIP_ERROR lReturn;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    // Sanity check that this instance and the target layer haven't been "crossed".
    VerifyOrDieWithMsg(aTarget.IsRetained(*this), chipSystemLayer, "wrong handler! [target %p != this %p]",
                       &(aTarget.SystemLayer()), this);

    lReturn        = CHIP_ERROR_UNEXPECTED_EVENT;
    lEventDelegate = this->mEventDelegateList;

    // Prevent the target object from being freed while dispatching the event.
    aTarget.Retain();

    while (lReturn == CHIP_ERROR_UNEXPECTED_EVENT && lEventDelegate != NULL)
    {
        lReturn        = lEventDelegate->mFunction(aTarget, aEventType, aArgument);
        lEventDelegate = lEventDelegate->mNextDelegate;
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

exit:
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
CHIP_ERROR Layer::StartPlatformTimer(uint32_t aDelayMilliseconds)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    lReturn = Platform::Layer::StartTimer(*this, this->mContext, aDelayMilliseconds);
    SuccessOrExit(lReturn);

exit:
    return lReturn;
}

/**
 * Handle the platform timer expiration event.
 *
 *  @brief
 *      Calls chip::System::Timer::HandleExpiredTimers to handle any expired timers.  It is assumed that this API is called
 *      only while on the thread which owns the CHIP System Layer object.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR Layer::HandlePlatformTimer()
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_ERROR_INCORRECT_STATE);

    lReturn = Timer::HandleExpiredTimers(*this);

    DispatchTimerCallbacks(Clock::GetMonotonicMilliseconds());

    SuccessOrExit(lReturn);

exit:
    return lReturn;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS

// MARK: CHIP System Layer platform- and system-specific functions for LwIP-native eventing.
struct LwIPEvent
{
    EventType Type;
    Object * Target;
    uintptr_t Argument;
};

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace Platform {
namespace Layer {

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS

/**
 * This is a platform-specific CHIP System Layer pre-initialization hook. This may be overridden by assserting the preprocessor
 * definition, #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS.
 *
 *  @param[in,out] aLayer    A reference to the CHIP System Layer instance being initialized.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Init.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 *      Returning non-successful status will abort initialization.
 */
DLL_EXPORT CHIP_ERROR WillInit(System::Layer & aLayer, void * aContext)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);

    return CHIP_NO_ERROR;
}

/**
 * This is a platform-specific CHIP System Layer pre-shutdown hook. This may be overridden by assserting the preprocessor
 * definition, #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS.
 *
 *  @param[in,out] aLayer    A pointer to the CHIP System Layer instance being shutdown.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Shutdown.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for shutdown failure. Returning
 *      non-successful status will abort shutdown.
 */
DLL_EXPORT CHIP_ERROR WillShutdown(System::Layer & aLayer, void * aContext)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);

    return CHIP_NO_ERROR;
}

/**
 * This is a platform-specific CHIP System Layer post-initialization hook. This may be overridden by assserting the preprocessor
 * definition, #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS.
 *
 *  @param[in,out] aLayer    A reference to the CHIP System Layer instance being initialized.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Init.
 *
 *  @param[in]     aStatus   The overall status being returned via the CHIP System Layer \::Init method.
 */
DLL_EXPORT void DidInit(System::Layer & aLayer, void * aContext, CHIP_ERROR aStatus)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);
    static_cast<void>(aStatus);
}

/**
 * This is a platform-specific CHIP System Layer pre-shutdown hook. This may be overridden by assserting the preprocessor
 * definition, #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS.
 *
 *  @param[in,out] aLayer    A reference to the CHIP System Layer instance being shutdown.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Shutdown.
 *
 *  @param[in]     aStatus   The overall status being returned via the CHIP System Layer \::Shutdown method.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for shutdown failure. Returning
 *      non-successful status will abort shutdown.
 */
DLL_EXPORT void DidShutdown(System::Layer & aLayer, void * aContext, CHIP_ERROR aStatus)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);
    static_cast<void>(aStatus);
}

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS

using chip::System::LwIPEvent;

/**
 *  This is a platform-specific event / message post hook. This may be overridden by assserting the preprocessor definition,
 *  #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS.
 *
 *  This posts an event / message of the specified type with the provided argument to this instance's platform-specific event /
 *  message queue.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[in,out] aLayer    A pointer to the layer instance to which the event / message is being posted.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Init.
 *
 *  @param[in,out] aTarget   A pointer to the CHIP System Layer object making the post request.
 *
 *  @param[in]     aType     The type of event to post.
 *
 *  @param[in,out] aArgument The argument associated with the event to post.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
DLL_EXPORT CHIP_ERROR PostEvent(Layer & aLayer, void * aContext, Object & aTarget, EventType aType, uintptr_t aArgument)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    sys_mbox_t lSysMbox;
    LwIPEvent * ev;
    err_t lLwIPError;

    VerifyOrExit(aContext != NULL, lReturn = CHIP_ERROR_INVALID_ARGUMENT);
    lSysMbox = reinterpret_cast<sys_mbox_t>(aContext);

    ev = chip::Platform::New<LwIPEvent>();
    VerifyOrExit(ev != nullptr, lReturn = CHIP_ERROR_NO_MEMORY);

    ev->Type     = aType;
    ev->Target   = &aTarget;
    ev->Argument = aArgument;

    lLwIPError = sys_mbox_trypost(&lSysMbox, ev);
    VerifyOrExit(lLwIPError == ERR_OK, chip::Platform::Delete(ev); lReturn = chip::System::MapErrorLwIP(lLwIPError));

exit:
    return lReturn;
}

/**
 *  This is a platform-specific event / message dispatch hook. This may be overridden by assserting the preprocessor definition,
 *  #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS.
 *
 *  This effects an event loop, waiting on a queue that services this instance, pulling events off of that queue, and then
 *  dispatching them for handling.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[in,out] aLayer    A reference to the layer instance for which events / messages are being dispatched.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Init.
 *
 *  @retval   #CHIP_ERROR_INVALID_ARGUMENT      If aLayer or aContext is NULL.
 *  @retval   #CHIP_ERROR_INCORRECT_STATE       If the state of the CHIP System Layer object is unexpected.
 *  @retval   #CHIP_ERROR_UNEXPECTED_EVENT      If an event type is unrecognized.
 *  @retval   #CHIP_NO_ERROR                    On success.
 */
DLL_EXPORT CHIP_ERROR DispatchEvents(Layer & aLayer, void * aContext)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;
    err_t lLwIPError;
    sys_mbox_t lSysMbox;
    void * lVoidPointer;
    const LwIPEvent * lEvent;

    // Sanity check the context / queue.
    VerifyOrExit(aContext != NULL, lReturn = CHIP_ERROR_INVALID_ARGUMENT);
    lSysMbox = reinterpret_cast<sys_mbox_t>(aContext);

    while (true)
    {
        lLwIPError = sys_arch_mbox_tryfetch(&lSysMbox, &lVoidPointer);
        VerifyOrExit(lLwIPError == ERR_OK, lReturn = chip::System::MapErrorLwIP(lLwIPError));

        lEvent = static_cast<const LwIPEvent *>(lVoidPointer);
        VerifyOrExit(lEvent != NULL && lEvent->Target != NULL, lReturn = CHIP_ERROR_UNEXPECTED_EVENT);

        lReturn = aLayer.HandleEvent(*lEvent->Target, lEvent->Type, lEvent->Argument);
        chip::Platform::Delete(lEvent);

        SuccessOrExit(lReturn);
    }

exit:
    return lReturn;
}

/**
 *  This is a platform-specific event / message dispatch hook. This may be overridden by assserting the preprocessor definition,
 *  #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS.
 *
 *  This dispatches the specified event for handling, unmarshalling the type and arguments from the event for hand off to CHIP
 *  System Layer::HandleEvent for the actual dispatch.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[in,out] aLayer    A reference to the layer instance for which events / messages are being dispatched.
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Init.
 *  @param[in]     aEvent    The platform-specific event object to dispatch for handling.
 *
 *  @retval   #CHIP_ERROR_INVALID_ARGUMENT      If aLayer or the event target is NULL.
 *  @retval   #CHIP_ERROR_UNEXPECTED_EVENT      If the event type is unrecognized.
 *  @retval   #CHIP_ERROR_INCORRECT_STATE       If the state of the CHIP System Layer object is unexpected.
 *  @retval   #CHIP_NO_ERROR                    On success.
 */
DLL_EXPORT CHIP_ERROR DispatchEvent(Layer & aLayer, void * aContext, Event aEvent)
{
    const EventType type = aEvent->Type;
    Object * target      = aEvent->Target;
    const uint32_t data  = aEvent->Argument;
    CHIP_ERROR lReturn   = CHIP_NO_ERROR;

    // Sanity check the target object.
    VerifyOrExit(target != NULL, lReturn = CHIP_ERROR_INVALID_ARGUMENT);

    // Handle the event.
    lReturn = aLayer.HandleEvent(*target, type, data);
    SuccessOrExit(lReturn);

exit:
    return lReturn;
}

/**
 *  This is a platform-specific event / message dispatch hook. This may be overridden by assserting the preprocessor definition,
 *  #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS.
 *
 *  @note
 *    This is an implementation for LwIP.
 *
 *  @param[in,out] aLayer               A reference to the layer instance for which events / messages are being dispatched.
 *  @param[in,out] aContext             Platform-specific context data passed to the layer initialization method, \::Init.
 *  @param[in]     aMilliseconds        The number of milliseconds to set for the timer.
 *
 *  @retval   #CHIP_NO_ERROR    Always succeeds unless overridden.
 */
DLL_EXPORT CHIP_ERROR StartTimer(Layer & aLayer, void * aContext, uint32_t aMilliseconds)
{
    CHIP_ERROR lReturn = CHIP_NO_ERROR;

    // At the moment there is no need to do anything for standalone CHIP + LWIP.
    // the Task will periodically call HandleTimer which will process any expired
    // timers.
    static_cast<void>(aLayer);
    static_cast<void>(aContext);
    static_cast<void>(aMilliseconds);

    return lReturn;
}

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
