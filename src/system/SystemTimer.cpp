/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines the member functions and private data for
 *      the nl::Weave::System::Timer class, which is used for
 *      representing an in-progress one-shot timer.
 */

// Include module header
#include <SystemLayer/SystemTimer.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <string.h>

#include <SystemLayer/SystemError.h>
#include <SystemLayer/SystemLayer.h>
#include <SystemLayer/SystemFaultInjection.h>

#include <Weave/Support/CodeUtils.h>

/*******************************************************************************
 * Timer state
 *
 * There are two fundamental state-change variables: Object::mSystemLayer and
 * Timer::OnComplete. These must be checked and changed atomically. The state
 * of the timer is governed by the following state machine:
 *
 *  INITIAL STATE: mSystemLayer == NULL, OnComplete == NULL
 *      |
 *      V
 *  UNALLOCATED<-----------------------------+
 *      |                                    |
 *  (set mSystemLayer != NULL)               |
 *      |                                    |
 *      V                                    |
 *  ALLOCATED-------(set mSystemLayer NULL)--+
 *      |    \-----------------------------+
 *      |                                  |
 *  (set OnComplete != NULL)               |
 *      |                                  |
 *      V                                  |
 *    ARMED ---------( clear OnComplete )--+
 *
 * When in the ARMED state:
 *
 *     * None of the member variables may mutate.
 *     * OnComplete must only be cleared by Cancel() or HandleComplete()
 *     * Cancel() and HandleComplete() will test that they are the one to
 *       successfully set OnComplete NULL. And if so, that will be the
 *       thread that must call Object::Release().
 *
 *******************************************************************************
 */

namespace nl {
namespace Weave {
namespace System {

ObjectPool<Timer, WEAVE_SYSTEM_CONFIG_NUM_TIMERS> Timer::sPool;

/**
 *  This method returns the current epoch, corrected by system sleep with the system timescale, in milliseconds.
 *
 *  DEPRECATED -- Please use System::Layer::GetClock_MonotonicMS() instead.
 *
 *  @return A timestamp in milliseconds.
 */
Timer::Epoch Timer::GetCurrentEpoch(void)
{
    return Platform::Layer::GetClock_MonotonicMS();
}

/**
 *  Compares two Timer::Epoch values and returns true if the first value is earlier than the second value.
 *
 *  @brief
 *      A static API that gets called to compare 2 time values.  This API attempts to account for timer wrap by assuming that the
 *      difference between the 2 input values will only be more than half the Epoch scalar range if a timer wrap has occurred
 *      between the 2 samples.
 *
 *  @note
 *      This implementation assumes that Timer::Epoch is an unsigned scalar type.
 *
 *  @return true if the first param is earlier than the second, false otherwise.
 */
bool Timer::IsEarlierEpoch(const Timer::Epoch &inFirst, const Timer::Epoch &inSecond)
{
    static const Epoch kMaxTime_2 = static_cast<Epoch>((static_cast<Epoch>(0) - static_cast<Epoch>(1)) / 2);

    // account for timer wrap with the assumption that no two input times will "naturally"
    // be more than half the timer range apart.
    return (((inFirst < inSecond) && (inSecond - inFirst < kMaxTime_2)) ||
            ((inFirst > inSecond) && (inFirst - inSecond > kMaxTime_2)));
}

/**
 *  This method registers an one-shot timer with the underlying timer mechanism provided by the platform.
 *
 *  @param[in]  aDelayMilliseconds  The number of milliseconds before this timer fires
 *  @param[in]  aOnComplete          A pointer to the callback function when this timer fires
 *  @param[in]  aAppState            An arbitrary pointer to be passed into onComplete when this timer fires
 *
 *  @retval #WEAVE_SYSTEM_NO_ERROR Unconditionally.
 *
 */
Error Timer::Start(uint32_t aDelayMilliseconds, OnCompleteFunct aOnComplete, void* aAppState)
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    Layer& lLayer = this->SystemLayer();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

    WEAVE_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, aDelayMilliseconds = 0);

    this->AppState = aAppState;
    this->mAwakenEpoch = Timer::GetCurrentEpoch() + static_cast<Epoch>(aDelayMilliseconds);
    if (!__sync_bool_compare_and_swap(&this->OnComplete, NULL, aOnComplete))
    {
        WeaveDie();
    }

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    // add to the sorted list of timers. Earliest timer appears first.
    if (lLayer.mTimerList == NULL ||
        this->IsEarlierEpoch(this->mAwakenEpoch, lLayer.mTimerList->mAwakenEpoch))
    {
        this->mNextTimer = lLayer.mTimerList;
        lLayer.mTimerList = this;

        // this is the new eariest timer and so the timer needs (re-)starting provided that
        // the system is not currently processing expired timers, in which case it is left to
        // HandleExpiredTimers() to re-start the timer.
        if (!lLayer.mTimerComplete)
        {
            lLayer.StartPlatformTimer(aDelayMilliseconds);
        }
    }
    else
    {
        Timer* lTimer = lLayer.mTimerList;

        while (lTimer->mNextTimer)
        {
            if (this->IsEarlierEpoch(this->mAwakenEpoch, lTimer->mNextTimer->mAwakenEpoch))
            {
                // found the insert location.
                break;
            }

            lTimer = lTimer->mNextTimer;
        }

        this->mNextTimer = lTimer->mNextTimer;
        lTimer->mNextTimer = this;
    }
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

    return WEAVE_SYSTEM_NO_ERROR;
}

Error Timer::ScheduleWork(OnCompleteFunct aOnComplete, void* aAppState)
{
    Error err = WEAVE_SYSTEM_NO_ERROR;
    Layer& lLayer = this->SystemLayer();

    this->AppState = aAppState;
    this->mAwakenEpoch = Timer::GetCurrentEpoch();
    if (!__sync_bool_compare_and_swap(&this->OnComplete, NULL, aOnComplete))
    {
        WeaveDie();
    }

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    err = lLayer.PostEvent(*this, Weave::System::kEvent_ScheduleWork, 0);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    lLayer.WakeSelect();
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    return err;
}

/**
 *  This method de-initializes the timer object, and prevents this timer from firing if it hasn't done so.
 *
 *  @retval #WEAVE_SYSTEM_NO_ERROR Unconditionally.
 */
Error Timer::Cancel()
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    Layer& lLayer = this->SystemLayer();
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP
    OnCompleteFunct lOnComplete = this->OnComplete;

    // Check if the timer is armed
    VerifyOrExit(lOnComplete != NULL, );
    // Atomically disarm if the value has not changed
    VerifyOrExit(__sync_bool_compare_and_swap(&this->OnComplete, lOnComplete, NULL), );

    // Since this thread changed the state of OnComplete, release the timer.
    this->AppState = NULL;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    if (lLayer.mTimerList)
    {
        if (this == lLayer.mTimerList)
        {
            lLayer.mTimerList = this->mNextTimer;
        }
        else
        {
            Timer* lTimer = lLayer.mTimerList;

            while (lTimer->mNextTimer)
            {
                if (this == lTimer->mNextTimer)
                {
                    lTimer->mNextTimer = this->mNextTimer;
                    break;
                }

                lTimer = lTimer->mNextTimer;
            }
        }

        this->mNextTimer = NULL;
    }
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

    this->Release();
exit:
    return WEAVE_SYSTEM_NO_ERROR;
}

/**
 *  This method is called by the underlying timer mechanism provided by the platform when the timer fires.
 */
void Timer::HandleComplete()
{
    // Save information needed to perform the callback.
    Layer& lLayer = this->SystemLayer();
    const OnCompleteFunct lOnComplete = this->OnComplete;
    void* lAppState = this->AppState;

    // Check if timer is armed
    VerifyOrExit(lOnComplete != NULL, );
    // Atomically disarm if the value has not changed.
    VerifyOrExit(__sync_bool_compare_and_swap(&this->OnComplete, lOnComplete, NULL), );

    // Since this thread changed the state of OnComplete, release the timer.
    AppState = NULL;
    this->Release();

    // Invoke the app's callback, if it's still valid.
    if (lOnComplete != NULL)
        lOnComplete(&lLayer, lAppState, WEAVE_SYSTEM_NO_ERROR);

exit:
    return;
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
/**
 * Completes any timers that have expired.
 *
 *  @brief
 *      A static API that gets called when the platform timer expires. Any expired timers are completed and removed from the list
 *      of active timers in the layer object. If unexpired timers remain on completion, StartPlatformTimer will be called to
 *      restart the platform timer.
 *
 *  @note
 *      It's harmless if this API gets called and there are no expired timers.
 *
 *  @return WEAVE_SYSTEM_NO_ERROR on success, error code otherwise.
 *
 */
Error Timer::HandleExpiredTimers(Layer& aLayer)
{
    size_t timersHandled = 0;

    // Expire each timer in turn until an unexpired timer is reached or the timerlist is emptied.  We set the current expiration
    // time outside the loop; that way timers set after the current tick will not be executed within this expiration window
    // regardless how long the processing of the currently expired timers took
    Epoch currentEpoch = Timer::GetCurrentEpoch();

    while (aLayer.mTimerList)
    {
        // limit the number of timers handled before the control is returned to the event queue.  The bound is similar to
        // (though not exactly same) as that on the sockets-based systems.

        // The platform timer API has MSEC resolution so expire any timer with less than 1 msec remaining.
        if ((timersHandled < Timer::sPool.Size()) && Timer::IsEarlierEpoch(aLayer.mTimerList->mAwakenEpoch, currentEpoch + 1))
        {
            Timer& lTimer = *aLayer.mTimerList;
            aLayer.mTimerList = lTimer.mNextTimer;
            lTimer.mNextTimer = NULL;

            aLayer.mTimerComplete = true;
            lTimer.HandleComplete();
            aLayer.mTimerComplete = false;

            timersHandled++;
        }
        else
        {
            // timers still exist so restart the platform timer.
            uint64_t delayMilliseconds = 0ULL;

            currentEpoch = Timer::GetCurrentEpoch();

            // the next timer expires in the future, so set the delayMilliseconds to a non-zero value
            if (currentEpoch < aLayer.mTimerList->mAwakenEpoch)
            {
                delayMilliseconds = aLayer.mTimerList->mAwakenEpoch - currentEpoch;
            }
            /*
             * StartPlatformTimer() accepts a 32bit value in milliseconds.  Epochs are 64bit numbers.  The only way in which this could
             * overflow is if time went backwards (e.g. as a result of a time adjustment from time synchronization).  Verify that the
             * timer can still be executed (even if it is very late) and exit if that is the case.  Note: if the time sync ever ends up
             * adjusting the clock, we should implement a method that deals with all the timers in the system.
             */
            VerifyOrDie(delayMilliseconds <= UINT32_MAX);

            aLayer.StartPlatformTimer(static_cast<uint32_t>(delayMilliseconds));
            break; // all remaining timers are still ticking.
        }
    }

    return WEAVE_SYSTEM_NO_ERROR;
}
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace Weave
} // namespace nl
