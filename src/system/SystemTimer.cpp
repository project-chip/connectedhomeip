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
 *      This file defines the member functions and private data for
 *      the chip::System::Timer class, which is used for
 *      representing an in-progress one-shot timer.
 */

// Include module header
#include <system/SystemTimer.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <string.h>

#include <system/SystemError.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace System {

#if CHIP_SYSTEM_CONFIG_NUM_TIMERS

/*******************************************************************************
 * Timer state
 *
 * There are two fundamental state-change variables: Object::mSystemLayer and
 * Timer::mOnComplete. These must be checked and changed atomically. The state
 * of the timer is governed by the following state machine:
 *
 *  INITIAL STATE: mSystemLayer == NULL, mOnComplete == NULL
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
 *  (set mOnComplete != NULL)               |
 *      |                                  |
 *      V                                  |
 *    ARMED ---------( clear mOnComplete )--+
 *
 * When in the ARMED state:
 *
 *     * None of the member variables may mutate.
 *     * mOnComplete must only be cleared by Cancel() or HandleComplete()
 *     * Cancel() and HandleComplete() will test that they are the one to
 *       successfully set mOnComplete NULL. And if so, that will be the
 *       thread that must call Object::Release().
 *
 *******************************************************************************
 */

ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> Timer::sPool;

Timer * Timer::New(System::Layer & systemLayer, uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState)
{
    Timer * timer = Timer::sPool.TryCreate();
    if (timer == nullptr)
    {
        ChipLogError(chipSystemLayer, "Timer pool EMPTY");
    }
    else
    {
        timer->AppState     = appState;
        timer->mSystemLayer = &systemLayer;
        timer->mAwakenTime  = Clock::GetMonotonicMilliseconds() + static_cast<Clock::MonotonicMilliseconds>(delayMilliseconds);
        if (!__sync_bool_compare_and_swap(&timer->mOnComplete, nullptr, onComplete))
        {
            chipDie();
        }
    }
    return timer;
}

void Timer::Clear()
{
    TimerCompleteCallback lOnComplete = this->mOnComplete;

    // Check if the timer is armed
    VerifyOrReturn(lOnComplete != nullptr);

    // Atomically disarm if the value has not changed
    VerifyOrReturn(__sync_bool_compare_and_swap(&mOnComplete, lOnComplete, nullptr));

    // Since this thread changed the state of mOnComplete, release the timer.
    AppState     = nullptr;
    mSystemLayer = nullptr;
}

void Timer::HandleComplete()
{
    // Save information needed to perform the callback.
    Layer * lLayer                          = this->mSystemLayer;
    const TimerCompleteCallback lOnComplete = this->mOnComplete;
    void * lAppState                        = this->AppState;

    // Check if timer is armed
    VerifyOrReturn(lOnComplete != nullptr, );
    // Atomically disarm if the value has not changed.
    VerifyOrReturn(__sync_bool_compare_and_swap(&this->mOnComplete, lOnComplete, nullptr), );

    // Since this thread changed the state of mOnComplete, release the timer.
    AppState     = nullptr;
    mSystemLayer = nullptr;
    this->Release();

    // Invoke the app's callback, if it's still valid.
    if (lOnComplete != nullptr)
        lOnComplete(lLayer, lAppState);
}

Timer * Timer::List::Add(Timer * add)
{
    VerifyOrDie(add != mHead);
    if (mHead == NULL || Clock::IsEarlier(add->mAwakenTime, mHead->mAwakenTime))
    {
        add->mNextTimer = mHead;
        mHead           = add;
    }
    else
    {
        Timer * lTimer = mHead;
        while (lTimer->mNextTimer)
        {
            VerifyOrDie(lTimer->mNextTimer != add);
            if (Clock::IsEarlier(add->mAwakenTime, lTimer->mNextTimer->mAwakenTime))
            {
                // found the insert location.
                break;
            }
            lTimer = lTimer->mNextTimer;
        }
        add->mNextTimer    = lTimer->mNextTimer;
        lTimer->mNextTimer = add;
    }
    return mHead;
}

Timer * Timer::List::Remove(Timer * remove)
{
    VerifyOrDie(mHead != nullptr);

    if (remove == mHead)
    {
        mHead = remove->mNextTimer;
    }
    else
    {
        Timer * lTimer = mHead;

        while (lTimer->mNextTimer)
        {
            if (remove == lTimer->mNextTimer)
            {
                lTimer->mNextTimer = remove->mNextTimer;
                break;
            }

            lTimer = lTimer->mNextTimer;
        }
    }

    remove->mNextTimer = nullptr;
    return mHead;
}

Timer * Timer::List::Remove(TimerCompleteCallback aOnComplete, void * aAppState)
{
    Timer * previous = nullptr;
    for (Timer * timer = mHead; timer != nullptr; timer = timer->mNextTimer)
    {
        if (timer->mOnComplete == aOnComplete && timer->AppState == aAppState)
        {
            if (previous == nullptr)
            {
                mHead = timer->mNextTimer;
            }
            else
            {
                previous->mNextTimer = timer->mNextTimer;
            }
            timer->mNextTimer = nullptr;
            return timer;
        }
        previous = timer;
    }
    return nullptr;
}

Timer * Timer::List::PopEarliest()
{
    if (mHead == nullptr)
    {
        return nullptr;
    }
    Timer * earliest     = mHead;
    mHead                = mHead->mNextTimer;
    earliest->mNextTimer = nullptr;
    return earliest;
}

Timer * Timer::List::PopIfEarlier(Clock::MonotonicMilliseconds t)
{
    if ((mHead == nullptr) || !Clock::IsEarlier(mHead->mAwakenTime, t))
    {
        return nullptr;
    }
    Timer * earliest     = mHead;
    mHead                = mHead->mNextTimer;
    earliest->mNextTimer = nullptr;
    return earliest;
}

Timer * Timer::List::ExtractEarlier(Clock::MonotonicMilliseconds t)
{
    if ((mHead == nullptr) || !Clock::IsEarlier(mHead->mAwakenTime, t))
    {
        return nullptr;
    }
    Timer * begin = mHead;
    Timer * end   = mHead;
    while ((end->mNextTimer != nullptr) && Clock::IsEarlier(end->mNextTimer->mAwakenTime, t))
    {
        end = end->mNextTimer;
    }
    mHead           = end->mNextTimer;
    end->mNextTimer = nullptr;
    return begin;
}

CHIP_ERROR Timer::MutexedList::Init()
{
    mHead = nullptr;
#if CHIP_SYSTEM_CONFIG_NO_LOCKING
    return CHIP_NO_ERROR;
#else  // CHIP_SYSTEM_CONFIG_NO_LOCKING
    return Mutex::Init(mMutex);
#endif // CHIP_SYSTEM_CONFIG_NO_LOCKING
}

#endif // CHIP_SYSTEM_CONFIG_NUM_TIMERS

} // namespace System
} // namespace chip
