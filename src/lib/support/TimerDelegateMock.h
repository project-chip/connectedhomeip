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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemClock.h>

namespace chip {

class TimerDelegateMock : public TimerDelegate
{
public:
    CriticalFailure StartTimer(TimerContext * context, chip::System::Clock::Timeout aTimeout) override
    {
        mTimerContext = context;
        mTimerTimeout = mMockSystemTimestamp + aTimeout;
        return CHIP_NO_ERROR;
    }

    void CancelTimer(TimerContext * context) override
    {
        mTimerContext = nullptr;
        mTimerTimeout = chip::System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    }

    bool IsTimerActive(TimerContext * context) override { return mTimerContext != nullptr && mTimerTimeout > mMockSystemTimestamp; }

    chip::System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mMockSystemTimestamp; }

    void AdvanceClock(chip::System::Clock::Timeout aTimeout)
    {
        mMockSystemTimestamp += aTimeout;
        if (mTimerContext && mMockSystemTimestamp >= mTimerTimeout)
        {
            mTimerContext->TimerFired();
        }
    }

private:
    TimerContext * mTimerContext                        = nullptr;
    chip::System::Clock::Timestamp mTimerTimeout        = chip::System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    chip::System::Clock::Timestamp mMockSystemTimestamp = chip::System::Clock::Milliseconds64(0);
};

} // namespace chip
