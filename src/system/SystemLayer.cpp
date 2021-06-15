/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

Error Layer::Init(void * aContext)
{
    Error lReturn;

    RegisterLayerErrorFormatter();
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    RegisterPOSIXErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    RegisterLwIPErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    if (this->mLayerState != kLayerState_NotInitialized)
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;

    lReturn = Platform::Layer::WillInit(*this, aContext);
    SuccessOrExit(lReturn);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    this->AddEventHandlerDelegate(sSystemEventHandlerDelegate);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    // Create an event to allow an arbitrary thread to wake the thread in the select loop.
    lReturn = this->mWakeEvent.Open();
    SuccessOrExit(lReturn);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    this->mLayerState = kLayerState_Initialized;
    this->mContext    = aContext;

exit:
    Platform::Layer::DidInit(*this, aContext, lReturn);
    return lReturn;
}

Error Layer::Shutdown()
{
    Error lReturn;
    void * lContext;

    if (this->mLayerState == kLayerState_NotInitialized)
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;

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

Error Layer::NewTimer(Timer *& aTimerPtr)
{
    Timer * lTimer = nullptr;

    if (this->State() != kLayerState_Initialized)
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;

    lTimer    = Timer::sPool.TryCreate(*this);
    aTimerPtr = lTimer;

    if (lTimer == nullptr)
    {
        ChipLogError(chipSystemLayer, "Timer pool EMPTY");
        return CHIP_SYSTEM_ERROR_NO_MEMORY;
    }

    return CHIP_SYSTEM_NO_ERROR;
}

static bool TimerReady(const Timer::Epoch epoch, const Cancelable * timer)
{
    return !Timer::IsEarlierEpoch(epoch, timer->mInfoScalar);
}

static int TimerCompare(void * p, const Cancelable * a, const Cancelable * b)
{
    (void) p;
    return (a->mInfoScalar > b->mInfoScalar) ? 1 : (a->mInfoScalar < b->mInfoScalar) ? -1 : 0;
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
 *   @return CHIP_SYSTEM_NO_ERROR On success.
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

    ca->mInfoScalar = Timer::GetCurrentEpoch() + aMilliseconds;

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
 *   @return CHIP_SYSTEM_NO_ERROR On success.
 *   @return CHIP_SYSTEM_ERROR_NO_MEMORY If a timer cannot be allocated.
 *   @return Other Value indicating timer failed to start.
 *
 */
Error Layer::StartTimer(uint32_t aMilliseconds, TimerCompleteFunct aComplete, void * aAppState)
{
    Error lReturn;
    Timer * lTimer;

    this->CancelTimer(aComplete, aAppState);
    lReturn = this->NewTimer(lTimer);
    SuccessOrExit(lReturn);

    lReturn = lTimer->Start(aMilliseconds, aComplete, aAppState);
    if (lReturn != CHIP_SYSTEM_NO_ERROR)
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
 *   `StartTimer(0, aComplete, aAppState)`.  The specification for
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
 * @retval CHIP_SYSTEM_ERROR_UNEXPECTED_STATE If the SystemLayer has
 *                      not been initialized.
 *
 * @retval CHIP_SYSTEM_ERROR_NO_MEMORY If the SystemLayer cannot
 *                      allocate a new timer.
 *
 * @retval CHIP_SYSTEM_NO_ERROR On success.
 */
Error Layer::ScheduleWork(TimerCompleteFunct aComplete, void * aAppState)
{
    assertChipStackLockedByCurrentThread();

    Error lReturn;
    Timer * lTimer;

    lReturn = this->NewTimer(lTimer);
    SuccessOrExit(lReturn);

    lReturn = lTimer->ScheduleWork(aComplete, aAppState);
    if (lReturn != CHIP_SYSTEM_NO_ERROR)
    {
        lTimer->Release();
    }

exit:
    return lReturn;
}

/**
 * @brief
 *   Returns a monotonic system time in units of microseconds.
 *
 * This function returns an elapsed time in microseconds since an arbitrary, platform-defined
 * epoch.  The value returned is guaranteed to be ever-increasing (i.e. never wrapping) between
 * reboots of the system.  Additionally, the underlying time source is guaranteed to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * Although some platforms may choose to return a value that measures the time since boot for the
 * system, applications must *not* rely on this.  Additionally, the epoch for GetClock_Monotonic()
 * is *not* required to be the same as that for any of the other GetClock... functions.  Therefore
 * relative time calculations can only be performed on values returned by the same function.
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
uint64_t Layer::GetClock_Monotonic()
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::GetClock_Monotonic();
}

/**
 * @brief
 *   Returns a monotonic system time in units of milliseconds.
 *
 * This function returns an elapsed time in milliseconds since an arbitrary, platform-defined
 * epoch.  The value returned is guaranteed to be ever-increasing (i.e. never wrapping) between
 * reboots of the system.  Additionally, the underlying time source is guaranteed to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * Although some platforms may choose to return a value that measures the time since boot for the
 * system, applications must *not* rely on this.  Additionally, the epoch for GetClock_Monotonic()
 * is *not* required to be the same as that for any of the other GetClock... functions.  Therefore
 * relative time calculations can only be performed on values returned by the same function.
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @returns             Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
 */
uint64_t Layer::GetClock_MonotonicMS()
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::GetClock_MonotonicMS();
}

/**
 * @brief
 *   Returns a (potentially) high-resolution monotonic system time in units of microseconds.
 *
 * This function returns an elapsed time in microseconds since an arbitrary, platform-defined
 * epoch.  The value returned is guaranteed to be ever-increasing (i.e. never wrapping) between
 * reboots of the system.  However, the underlying timer is *not* required to tick continuously
 * during system deep-sleep states.
 *
 * Some platforms may implement GetClock_MonotonicHiRes() using a high-resolution timer capable
 * of greater precision than GetClock_Monotonic(), and that is not subject to gradual clock
 * adjustments (slewing).  Systems without such a timer may simply return the same value as
 * GetClock_Monotonic().
 *
 * The epoch for time returned by GetClock_MonotonicHiRes() is not required to be the same that
 * for any of the other GetClock... functions, including GetClock_Monotonic().
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
uint64_t Layer::GetClock_MonotonicHiRes()
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::GetClock_MonotonicHiRes();
}

/**
 * @brief
 *   Returns the current real (civil) time in microsecond Unix time format.
 *
 * This method returns the local platform's notion of current real time, expressed as a Unix time
 * value scaled to microseconds.  The underlying clock is guaranteed to tick at a rate of least at
 * whole seconds (values of 1,000,000), but on some platforms may tick faster.
 *
 * If the underlying platform is capable of tracking real time, but the system is currently
 * unsynchronized, GetClock_RealTime() will return the error CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED.
 *
 * On platforms that are incapable of tracking real time, the GetClock_RealTime() method may be absent,
 * resulting a link error for any application that references it.  Alternatively, such platforms may
 * supply an implementation of GetClock_RealTime() that always returns the error CHIP_SYSTEM_ERROR_NOT_SUPPORTED.
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @param[out] curTime                  The current time, expressed as Unix time scaled to microseconds.
 *
 * @retval #CHIP_SYSTEM_NO_ERROR       If the method succeeded.
 * @retval #CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED
 *                                      If the platform is capable of tracking real time, but is
 *                                      is currently unsynchronized.
 * @retval #CHIP_SYSTEM_ERROR_NOT_SUPPORTED
 *                                      If the platform is incapable of tracking real time.
 */
Error Layer::GetClock_RealTime(uint64_t & curTime)
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::GetClock_RealTime(curTime);
}

/**
 * @brief
 *   Returns the current real (civil) time in millisecond Unix time format.
 *
 * This method returns the local platform's notion of current real time, expressed as a Unix time
 * value scaled to milliseconds.  The underlying clock is guaranteed to tick at a rate of least at
 * whole seconds (values of 1,000,000), but on some platforms may tick faster.
 *
 * If the underlying platform is capable of tracking real time, but the system is currently
 * unsynchronized, GetClock_RealTimeMS() will return the error CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED.
 *
 * On platforms that are incapable of tracking real time, the GetClock_RealTimeMS() method may be absent,
 * resulting a link error for any application that references it.  Alternatively, such platforms may
 * supply an implementation of GetClock_RealTimeMS() that always returns the error CHIP_SYSTEM_ERROR_NOT_SUPPORTED.
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @param[out] curTimeMS               The current time, expressed as Unix time scaled to milliseconds.
 *
 * @retval #CHIP_SYSTEM_NO_ERROR       If the method succeeded.
 * @retval #CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED
 *                                      If the platform is capable of tracking real time, but is
 *                                      is currently unsynchronized.
 * @retval #CHIP_SYSTEM_ERROR_NOT_SUPPORTED
 *                                      If the platform is incapable of tracking real time.
 */
Error Layer::GetClock_RealTimeMS(uint64_t & curTimeMS)
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::GetClock_RealTimeMS(curTimeMS);
}

/**
 * @brief
 *   Sets the platform's notion of current real (civil) time.
 *
 * Applications can call this function to set the local platform's notion of current real time.  The
 * new current time is expressed as a Unix time value scaled to microseconds.
 *
 * Once set, underlying platform clock is guaranteed to track real time with a granularity of at least
 * whole seconds.
 *
 * Some platforms may restrict which applications or processes can set real time.  If the caller is
 * not permitted to change real time, the SetClock_RealTime() function will return the error
 * CHIP_SYSTEM_ERROR_ACCESS_DENIED.
 *
 * On platforms that are incapable of tracking real time, or do not offer the ability to set real time,
 * the SetClock_RealTime() function may be absent, resulting a link error for any application that
 * references it.  Alternatively, such platforms may supply an implementation of SetClock_RealTime()
 * that always returns the error CHIP_SYSTEM_ERROR_NOT_SUPPORTED.
 *
 * This function is guaranteed to be thread-safe on any platform that employs threading.
 *
 * @param[in] newCurTime                The new current time, expressed as Unix time scaled to microseconds.
 *
 * @retval #CHIP_SYSTEM_NO_ERROR       If the method succeeded.
 * @retval #CHIP_SYSTEM_ERROR_NOT_SUPPORTED
 *                                      If the platform is incapable of tracking real time.
 * @retval #CHIP_SYSTEM_ERROR_ACCESS_DENIED
 *                                      If the calling application does not have the privilege to set the
 *                                      current time.
 */
Error Layer::SetClock_RealTime(uint64_t newCurTime)
{
    // Current implementation is a simple pass-through to the platform.
    return Platform::Layer::SetClock_RealTime(newCurTime);
}

/**
 * @brief
 *   Run any timers that are due based on input current time
 */
void Layer::DispatchTimerCallbacks(const uint64_t kCurrentEpoch)
{
    // dispatch TimerCallbacks
    Cancelable ready;

    mTimerCallbacks.DequeueBy(TimerReady, kCurrentEpoch, ready);

    while (ready.mNext != &ready)
    {
        // one-shot
        chip::Callback::Callback<> * cb = chip::Callback::Callback<>::FromCancelable(ready.mNext);
        cb->Cancel();
        cb->mCall(cb->mContext);
    }
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

/**
 *  Prepare the sets of file descriptors for @p select() to work with.
 *
 *  @param[out] aSetSize        The range of file descriptors in the file descriptor set.
 *  @param[in]  aReadSet        A pointer to the set of readable file descriptors.
 *  @param[in]  aWriteSet       A pointer to the set of writable file descriptors.
 *  @param[in]  aExceptionSet   A pointer to the set of file descriptors with errors.
 *  @param[in]  aSleepTime      A reference to the maximum sleep time.
 */
void Layer::PrepareSelect(int & aSetSize, fd_set * aReadSet, fd_set * aWriteSet, fd_set * aExceptionSet,
                          struct timeval & aSleepTime)
{
    if (this->State() != kLayerState_Initialized)
        return;

    const int wakeEventFd = this->mWakeEvent.GetNotifFD();
    FD_SET(wakeEventFd, aReadSet);

    if (wakeEventFd + 1 > aSetSize)
        aSetSize = wakeEventFd + 1;

    const Timer::Epoch kCurrentEpoch = Timer::GetCurrentEpoch();
    Timer::Epoch lAwakenEpoch =
        kCurrentEpoch + static_cast<Timer::Epoch>(aSleepTime.tv_sec) * 1000 + static_cast<uint32_t>(aSleepTime.tv_usec) / 1000;

    for (size_t i = 0; i < Timer::sPool.Size(); i++)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr)
        {
            if (!Timer::IsEarlierEpoch(kCurrentEpoch, lTimer->mAwakenEpoch))
            {
                lAwakenEpoch = kCurrentEpoch;
                break;
            }

            if (Timer::IsEarlierEpoch(lTimer->mAwakenEpoch, lAwakenEpoch))
                lAwakenEpoch = lTimer->mAwakenEpoch;
        }
    }

    // check for an earlier callback timer, too
    if (lAwakenEpoch != kCurrentEpoch)
    {
        Cancelable * ca = mTimerCallbacks.First();
        if (ca != nullptr && !Timer::IsEarlierEpoch(kCurrentEpoch, ca->mInfoScalar))
        {
            lAwakenEpoch = ca->mInfoScalar;
        }
    }

    const Timer::Epoch kSleepTime = lAwakenEpoch - kCurrentEpoch;
    aSleepTime.tv_sec             = static_cast<time_t>(kSleepTime / 1000);
    aSleepTime.tv_usec            = static_cast<suseconds_t>((kSleepTime % 1000) * 1000);
}

/**
 * Handle I/O from a select call. This method registers the pending I/O event in each active endpoint and then invokes the
 * respective I/O handling functions for those endpoints.
 *
 * @note
 *  It is important to set the pending I/O fields for all endpoints *before* making any callbacks. This avoids the case where an
 *  endpoint is closed and then re-opened within the callback for another endpoint. When this happens the new endpoint is likely
 * to be assigned the same file descriptor as the old endpoint. However, any pending I/O for that file descriptor number
 * represents I/O related to the old incarnation of the endpoint, not the current one. Saving the pending I/O state in each
 * endpoint before acting on it allows the endpoint code to clear the I/O flags in the event of a close, thus avoiding any
 * confusion.
 *
 *  @param[in]    aSetSize          The return value of the select call.
 *  @param[in]    aReadSet          A pointer to the set of read file descriptors.
 *  @param[in]    aWriteSet         A pointer to the set of write file descriptors.
 *  @param[in]    aExceptionSet     A pointer to the set of file descriptors with errors.
 *
 */
void Layer::HandleSelectResult(int aSetSize, fd_set * aReadSet, fd_set * aWriteSet, fd_set * aExceptionSet)
{
    assertChipStackLockedByCurrentThread();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    pthread_t lThreadSelf;
#endif
    Error lReturn;

    if (this->State() != kLayerState_Initialized)
        return;

    if (aSetSize < 0)
        return;

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    lThreadSelf = pthread_self();
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    if (aSetSize > 0)
    {
        // If we woke because of someone writing to the wake event, clear the event before returning.
        if (FD_ISSET(this->mWakeEvent.GetNotifFD(), aReadSet))
        {
            lReturn = this->mWakeEvent.Confirm();
            if (lReturn != CHIP_SYSTEM_NO_ERROR)
            {
                ChipLogError(chipSystemLayer, "System wake event confirm failed: %s", ErrorStr(lReturn));
            }
        }
    }

    const Timer::Epoch kCurrentEpoch = Timer::GetCurrentEpoch();

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mHandleSelectThread = lThreadSelf;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

    for (size_t i = 0; i < Timer::sPool.Size(); i++)
    {
        Timer * lTimer = Timer::sPool.Get(*this, i);

        if (lTimer != nullptr && !Timer::IsEarlierEpoch(kCurrentEpoch, lTimer->mAwakenEpoch))
        {
            lTimer->HandleComplete();
        }
    }

    DispatchTimerCallbacks(kCurrentEpoch);

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    this->mHandleSelectThread = PTHREAD_NULL;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING
}

/**
 * Wake up the I/O thread that monitors the file descriptors using select() by writing a single byte to the wake pipe.
 *
 *  @note
 *      If @p WakeSelect() is being called from within @p HandleSelectResult(), then writing to the wake pipe can be skipped,
 * since the I/O thread is already awake.
 *
 *      Furthermore, we don't care if this write fails as the only reasonably likely failure is that the pipe is full, in which
 *      case the select calling thread is going to wake up anyway.
 */
void Layer::WakeSelect()
{
    Error lReturn;

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
    if (lReturn != CHIP_SYSTEM_NO_ERROR)
    {
        ChipLogError(chipSystemLayer, "System wake event notify failed: %s", ErrorStr(lReturn));
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_LWIP
LwIPEventHandlerDelegate Layer::sSystemEventHandlerDelegate;

/**
 * This is the dispatch handler for system layer events.
 *
 *  @param[in,out]  aTarget     A pointer to the CHIP System Layer object making the post request.
 *  @param[in]      aEventType  The type of event to post.
 *  @param[in,out]  aArgument   The argument associated with the event to post.
 */
Error Layer::HandleSystemLayerEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
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
        lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT;
        break;
    }

    return lReturn;
}

/**
 * This adds an event handler delegate to the system layer to extend its ability to handle LwIP events.
 *
 *  @param[in]  aDelegate   An uninitialied LwIP event handler delegate structure
 *
 *  @retval     CHIP_SYSTEM_NO_ERROR          On success.
 *  @retval     CHIP_SYSTEM_ERROR_BAD_ARGS    If the function pointer contained in aDelegate is NULL
 */
Error Layer::AddEventHandlerDelegate(LwIPEventHandlerDelegate & aDelegate)
{
    Error lReturn;

    VerifyOrExit(aDelegate.mFunction != NULL, lReturn = CHIP_SYSTEM_ERROR_BAD_ARGS);
    aDelegate.Prepend(this->mEventDelegateList);
    lReturn = CHIP_SYSTEM_NO_ERROR;

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
 *  @retval    CHIP_SYSTEM_NO_ERROR                   On success.
 *  @retval    CHIP_SYSTEM_ERROR_UNEXPECTED_STATE     If the state of the Layer object is incorrect.
 *  @retval    CHIP_SYSTEM_ERROR_NO_MEMORY            If the event queue is already full.
 *  @retval    other Platform-specific errors generated indicating the reason for failure.
 */
Error Layer::PostEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

    // Sanity check that this instance and the target layer haven't been "crossed".
    VerifyOrDieWithMsg(aTarget.IsRetained(*this), chipSystemLayer, "wrong poster! [target %p != this %p]", &(aTarget.SystemLayer()),
                       this);

    lReturn = Platform::Layer::PostEvent(*this, this->mContext, aTarget, aEventType, aArgument);
    if (lReturn != CHIP_SYSTEM_NO_ERROR)
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
 *  @return #CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
Error Layer::DispatchEvents()
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

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
 * @return CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
Error Layer::DispatchEvent(Event aEvent)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

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
 *  @retval   CHIP_SYSTEM_NO_ERROR                On success.
 *  @retval   CHIP_SYSTEM_ERROR_UNEXPECTED_STATE  If the state of the InetLayer object is incorrect.
 *  @retval   CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT  If the event type is unrecognized.
 */
Error Layer::HandleEvent(Object & aTarget, EventType aEventType, uintptr_t aArgument)
{
    const LwIPEventHandlerDelegate * lEventDelegate;
    Error lReturn;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

    // Sanity check that this instance and the target layer haven't been "crossed".
    VerifyOrDieWithMsg(aTarget.IsRetained(*this), chipSystemLayer, "wrong handler! [target %p != this %p]",
                       &(aTarget.SystemLayer()), this);

    lReturn        = CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT;
    lEventDelegate = this->mEventDelegateList;

    // Prevent the target object from being freed while dispatching the event.
    aTarget.Retain();

    while (lReturn == CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT && lEventDelegate != NULL)
    {
        lReturn        = lEventDelegate->mFunction(aTarget, aEventType, aArgument);
        lEventDelegate = lEventDelegate->mNextDelegate;
    }

    if (lReturn == CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT)
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
 *  @return CHIP_SYSTEM_NO_ERROR on success, error code otherwise.
 *
 */
Error Layer::StartPlatformTimer(uint32_t aDelayMilliseconds)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

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
 *  @return CHIP_SYSTEM_NO_ERROR on success, error code otherwise.
 *
 */
Error Layer::HandlePlatformTimer()
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    VerifyOrExit(this->State() == kLayerState_Initialized, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

    lReturn = Timer::HandleExpiredTimers(*this);

    DispatchTimerCallbacks(Timer::GetCurrentEpoch());

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
 *  @return #CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 *      Returning non-successful status will abort initialization.
 */
DLL_EXPORT Error WillInit(Layer & aLayer, void * aContext)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);

    return CHIP_SYSTEM_NO_ERROR;
}

/**
 * This is a platform-specific CHIP System Layer pre-shutdown hook. This may be overridden by assserting the preprocessor
 * definition, #CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_XTOR_FUNCTIONS.
 *
 *  @param[in,out] aLayer    A pointer to the CHIP System Layer instance being shutdown.
 *
 *  @param[in,out] aContext  Platform-specific context data passed to the layer initialization method, \::Shutdown.
 *
 *  @return #CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for shutdown failure. Returning
 *      non-successful status will abort shutdown.
 */
DLL_EXPORT Error WillShutdown(Layer & aLayer, void * aContext)
{
    static_cast<void>(aLayer);
    static_cast<void>(aContext);

    return CHIP_SYSTEM_NO_ERROR;
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
DLL_EXPORT void DidInit(Layer & aLayer, void * aContext, Error aStatus)
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
 *  @return #CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for shutdown failure. Returning
 *      non-successful status will abort shutdown.
 */
DLL_EXPORT void DidShutdown(Layer & aLayer, void * aContext, Error aStatus)
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
 *  @return #CHIP_SYSTEM_NO_ERROR on success; otherwise, a specific error indicating the reason for initialization failure.
 */
DLL_EXPORT Error PostEvent(Layer & aLayer, void * aContext, Object & aTarget, EventType aType, uintptr_t aArgument)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    sys_mbox_t lSysMbox;
    LwIPEvent * ev;
    err_t lLwIPError;

    VerifyOrExit(aContext != NULL, lReturn = CHIP_SYSTEM_ERROR_BAD_ARGS);
    lSysMbox = reinterpret_cast<sys_mbox_t>(aContext);

    ev = chip::Platform::New<LwIPEvent>();
    VerifyOrExit(ev != nullptr, lReturn = CHIP_SYSTEM_ERROR_NO_MEMORY);

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
 *  @retval   #CHIP_SYSTEM_ERROR_BAD_ARGS          If aLayer or aContext is NULL.
 *  @retval   #CHIP_SYSTEM_ERROR_UNEXPECTED_STATE  If the state of the CHIP System Layer object is unexpected.
 *  @retval   #CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT  If an event type is unrecognized.
 *  @retval   #CHIP_SYSTEM_NO_ERROR                On success.
 */
DLL_EXPORT Error DispatchEvents(Layer & aLayer, void * aContext)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;
    err_t lLwIPError;
    sys_mbox_t lSysMbox;
    void * lVoidPointer;
    const LwIPEvent * lEvent;

    // Sanity check the context / queue.
    VerifyOrExit(aContext != NULL, lReturn = CHIP_SYSTEM_ERROR_BAD_ARGS);
    lSysMbox = reinterpret_cast<sys_mbox_t>(aContext);

    while (true)
    {
        lLwIPError = sys_arch_mbox_tryfetch(&lSysMbox, &lVoidPointer);
        VerifyOrExit(lLwIPError == ERR_OK, lReturn = chip::System::MapErrorLwIP(lLwIPError));

        lEvent = static_cast<const LwIPEvent *>(lVoidPointer);
        VerifyOrExit(lEvent != NULL && lEvent->Target != NULL, lReturn = CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT);

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
 *  @retval   #CHIP_SYSTEM_ERROR_BAD_ARGS          If aLayer or the event target is NULL.
 *  @retval   #CHIP_SYSTEM_ERROR_UNEXPECTED_EVENT  If the event type is unrecognized.
 *  @retval   #CHIP_SYSTEM_ERROR_UNEXPECTED_STATE  If the state of the CHIP System Layer object is unexpected.
 *  @retval   #CHIP_SYSTEM_NO_ERROR                On success.
 */
DLL_EXPORT Error DispatchEvent(Layer & aLayer, void * aContext, Event aEvent)
{
    const EventType type = aEvent->Type;
    Object * target      = aEvent->Target;
    const uint32_t data  = aEvent->Argument;
    Error lReturn        = CHIP_SYSTEM_NO_ERROR;

    // Sanity check the target object.
    VerifyOrExit(target != NULL, lReturn = CHIP_SYSTEM_ERROR_BAD_ARGS);

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
 *  @retval   #CHIP_SYSTEM_NO_ERROR    Always succeeds unless overridden.
 */
DLL_EXPORT Error StartTimer(Layer & aLayer, void * aContext, uint32_t aMilliseconds)
{
    Error lReturn = CHIP_SYSTEM_NO_ERROR;

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
