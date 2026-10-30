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

#include "ExponentialBackOff.h"

#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

ExponentialBackOff::ExponentialBackOff(System::Clock::Seconds32 initialDelay, System::Clock::Seconds32 maximumDelay,
                                       System::Clock::Timeout quietPeriod) :
    mInitialDelay(initialDelay),
    mMaximumDelay(maximumDelay), mQuietPeriod(quietPeriod)
{
    // A zero initial delay would never grow, retrying at once forever.
    VerifyOrDie(initialDelay > System::Clock::kZero);
    VerifyOrDie(maximumDelay >= initialDelay);
}

System::Clock::Seconds32 ExponentialBackOff::NextDelay(System::Clock::Timestamp now)
{
    if ((mDelay == System::Clock::kZero) || ((now - mLastFailure) > mQuietPeriod))
    {
        mDelay = mInitialDelay;
    }
    else if (mDelay >= (mMaximumDelay - mDelay))
    {
        // Doubling would reach the maximum, or overflow.
        mDelay = mMaximumDelay;
    }
    else
    {
        mDelay += mDelay;
    }

    mLastFailure = now;

    return mDelay;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
