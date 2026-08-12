/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

#include <cstddef>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Test TimerDelegate giving a test control over every cluster timer.
 *
 * The shared chip::TimerDelegateMock cannot serve these tests: it holds a single
 * TimerContext, while the session manager arms one timer per pending request, and it
 * always reports success, while the rollback paths only run on a StartTimer failure.
 *
 * Time is virtual: AdvanceClock() fires everything due synchronously, so a test for a
 * 300-second expiry costs no wall-clock time. Modelled on TestReportScheduler.cpp's
 * TestTimerDelegate (fixed-capacity, no heap).
 */
class CommissioningProxyMockTimer : public TimerDelegate
{
public:
    static constexpr size_t kMaxTimers = 8;

    CriticalFailure StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) override
    {
        mStartCount++;
        if (mNextStartResult != CHIP_NO_ERROR)
        {
            // One-shot: the caller under test sees this failure, later timers succeed.
            CHIP_ERROR err   = mNextStartResult;
            mNextStartResult = CHIP_NO_ERROR;
            return err;
        }

        CancelTimer(context); // re-arming replaces any existing timer for this context
        VerifyOrReturnError(mCount < kMaxTimers, CHIP_ERROR_NO_MEMORY);
        mTimers[mCount].context = context;
        mTimers[mCount].firesAt = mNow + aTimeout;
        mCount++;
        return CHIP_NO_ERROR;
    }

    void CancelTimer(TimerContext * context) override
    {
        for (size_t i = 0; i < mCount; i++)
        {
            if (mTimers[i].context == context)
            {
                mTimers[i] = mTimers[mCount - 1];
                mCount--;
                return;
            }
        }
    }

    bool IsTimerActive(TimerContext * context) override
    {
        for (size_t i = 0; i < mCount; i++)
        {
            if (mTimers[i].context == context)
            {
                return true;
            }
        }
        return false;
    }

    System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mNow; }

    // --- Test control ----------------------------------------------------------

    /// Make the next StartTimer() call fail with @p err (one-shot).
    void FailNextStart(CHIP_ERROR err = CHIP_ERROR_NO_MEMORY) { mNextStartResult = err; }

    /// Move virtual time forward, firing every timer that becomes due. Firing a timer
    /// removes it first, so a callback may re-arm its own context safely.
    void AdvanceClock(System::Clock::Timeout aTimeout)
    {
        mNow += aTimeout;
        bool fired = true;
        while (fired)
        {
            fired = false;
            for (size_t i = 0; i < mCount; i++)
            {
                if (mTimers[i].firesAt <= mNow)
                {
                    TimerContext * context = mTimers[i].context;
                    mTimers[i]             = mTimers[mCount - 1];
                    mCount--;
                    context->TimerFired();
                    fired = true;
                    break;
                }
            }
        }
    }

    size_t ActiveCount() const { return mCount; }
    unsigned StartCount() const { return mStartCount; }

private:
    struct Entry
    {
        TimerContext * context = nullptr;
        System::Clock::Timestamp firesAt;
    };

    Entry mTimers[kMaxTimers];
    size_t mCount                 = 0;
    unsigned mStartCount          = 0;
    CHIP_ERROR mNextStartResult   = CHIP_NO_ERROR;
    System::Clock::Timestamp mNow = System::Clock::Milliseconds64(0);
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
