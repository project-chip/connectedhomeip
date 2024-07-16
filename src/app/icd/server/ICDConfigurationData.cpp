/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ICDConfigurationData.h"
#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/CodeUtils.h>

namespace chip {

ICDConfigurationData ICDConfigurationData::instance;

System::Clock::Milliseconds32 ICDConfigurationData::GetSlowPollingInterval()
{
#if ICD_ENFORCE_SIT_SLOW_POLL_LIMIT
    // When in SIT mode, the slow poll interval SHOULDN'T be greater than the SIT mode polling threshold, per spec.
    // This is important for ICD device configured for LIT operation but currently operating as a SIT
    // due to a lack of client registration
    if (mICDMode == ICDMode::SIT && mSlowPollingInterval > kSITPollingThreshold)
    {
        return kSITPollingThreshold;
    }
#endif
    return mSlowPollingInterval;
}

CHIP_ERROR ICDConfigurationData::SetModeDurations(Optional<System::Clock::Milliseconds32> activeModeDuration,
                                                  Optional<System::Clock::Milliseconds32> idleModeDuration)
{
    VerifyOrReturnError(activeModeDuration.HasValue() || idleModeDuration.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

    // Convert idleModeDuration to seconds for the correct precision
    System::Clock::Seconds32 tmpIdleModeDuration = idleModeDuration.HasValue()
        ? std::chrono::duration_cast<System::Clock::Seconds32>(idleModeDuration.Value())
        : mIdleModeDuration;

    System::Clock::Milliseconds32 tmpActiveModeDuration = activeModeDuration.ValueOr(mActiveModeDuration);

    VerifyOrReturnError(tmpActiveModeDuration <= tmpIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration <= kMaxIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration >= kMinIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);

    mIdleModeDuration   = tmpIdleModeDuration;
    mActiveModeDuration = tmpActiveModeDuration;

    return CHIP_NO_ERROR;
}

} // namespace chip
