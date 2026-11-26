/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <system/SystemClock.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  The delays of an exponential back-off between attempts at something
 *  that keeps failing.
 *
 *  The first failure is followed by the initial delay. Each failure after
 *  it doubles the delay, up to the maximum. A failure that comes more
 *  than the quiet period after the previous one starts over at the
 *  initial delay, as does one after Reset().
 *
 *  The caller supplies the time of each failure, so the back-off itself
 *  keeps no clock and schedules nothing.
 */
class ExponentialBackOff
{
public:
    /**
     *  @param[in] initialDelay  The delay after a first failure. Must be
     *                           greater than zero.
     *  @param[in] maximumDelay  The longest delay. Must be at least
     *                           initialDelay.
     *  @param[in] quietPeriod   How long without a failure starts the
     *                           back-off over.
     */
    ExponentialBackOff(System::Clock::Seconds32 initialDelay, System::Clock::Seconds32 maximumDelay,
                       System::Clock::Timeout quietPeriod);

    /**
     *  Records a failure, and returns how long to wait before the next
     *  attempt.
     *
     *  @param[in] now  The time of the failure, on a monotonic clock.
     */
    System::Clock::Seconds32 NextDelay(System::Clock::Timestamp now);

    /**
     *  Forgets every earlier failure, so that the next failure is followed
     *  by the initial delay.
     */
    void Reset() { mDelay = System::Clock::kZero; }

private:
    const System::Clock::Seconds32 mInitialDelay;
    const System::Clock::Seconds32 mMaximumDelay;
    const System::Clock::Timeout mQuietPeriod;
    System::Clock::Seconds32 mDelay       = System::Clock::kZero;
    System::Clock::Timestamp mLastFailure = System::Clock::kZero;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
