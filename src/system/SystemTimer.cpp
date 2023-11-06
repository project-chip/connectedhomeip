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

// Include local headers
#include <string.h>

#include <system/SystemError.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace System {

TimerList::Node * TimerList::Add(TimerList::Node * add)
{
    VerifyOrDie(add != mEarliestTimer);
    if (mEarliestTimer == nullptr || (add->AwakenTime() < mEarliestTimer->AwakenTime()))
    {
        add->mNextTimer = mEarliestTimer;
        mEarliestTimer  = add;
    }
    else
    {
        TimerList::Node * lTimer = mEarliestTimer;
        while (lTimer->mNextTimer)
        {
            VerifyOrDie(lTimer->mNextTimer != add);
            if (add->AwakenTime() < lTimer->mNextTimer->AwakenTime())
            {
                // found the insert location.
                break;
            }
            lTimer = lTimer->mNextTimer;
        }
        add->mNextTimer    = lTimer->mNextTimer;
        lTimer->mNextTimer = add;
    }
    return mEarliestTimer;
}

TimerList::Node * TimerList::Remove(TimerList::Node * remove)
{
    if (mEarliestTimer != nullptr && remove != nullptr)
    {
        if (remove == mEarliestTimer)
        {
            mEarliestTimer = remove->mNextTimer;
        }
        else
        {
            TimerList::Node * lTimer = mEarliestTimer;

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
    }
    return mEarliestTimer;
}

TimerList::Node * TimerList::Remove(TimerCompleteCallback aOnComplete, void * aAppState)
{
    TimerList::Node * previous = nullptr;
    for (TimerList::Node * timer = mEarliestTimer; timer != nullptr; timer = timer->mNextTimer)
    {
        if (timer->GetCallback().GetOnComplete() == aOnComplete && timer->GetCallback().GetAppState() == aAppState)
        {
            if (previous == nullptr)
            {
                mEarliestTimer = timer->mNextTimer;
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

TimerList::Node * TimerList::PopEarliest()
{
    if (mEarliestTimer == nullptr)
    {
        return nullptr;
    }
    TimerList::Node * earliest = mEarliestTimer;
    mEarliestTimer             = mEarliestTimer->mNextTimer;
    earliest->mNextTimer       = nullptr;
    return earliest;
}

TimerList::Node * TimerList::PopIfEarlier(Clock::Timestamp t)
{
    if ((mEarliestTimer == nullptr) || !(mEarliestTimer->AwakenTime() < t))
    {
        return nullptr;
    }
    TimerList::Node * earliest = mEarliestTimer;
    mEarliestTimer             = mEarliestTimer->mNextTimer;
    earliest->mNextTimer       = nullptr;
    return earliest;
}

TimerList TimerList::ExtractEarlier(Clock::Timestamp t)
{
    TimerList out;

    if ((mEarliestTimer != nullptr) && (mEarliestTimer->AwakenTime() < t))
    {
        out.mEarliestTimer    = mEarliestTimer;
        TimerList::Node * end = mEarliestTimer;
        while ((end->mNextTimer != nullptr) && (end->mNextTimer->AwakenTime() < t))
        {
            end = end->mNextTimer;
        }
        mEarliestTimer  = end->mNextTimer;
        end->mNextTimer = nullptr;
    }

    return out;
}

Clock::Timeout TimerList::GetRemainingTime(TimerCompleteCallback aOnComplete, void * aAppState)
{
    for (TimerList::Node * timer = mEarliestTimer; timer != nullptr; timer = timer->mNextTimer)
    {
        if (timer->GetCallback().GetOnComplete() == aOnComplete && timer->GetCallback().GetAppState() == aAppState)
        {
            Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();

            if (currentTime < timer->AwakenTime())
            {
                return Clock::Timeout(timer->AwakenTime() - currentTime);
            }
            return Clock::kZero;
        }
    }
    return Clock::kZero;
}

} // namespace System
} // namespace chip
