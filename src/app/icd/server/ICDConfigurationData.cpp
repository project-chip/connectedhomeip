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
#include <lib/support/CodeUtils.h>

namespace chip {

ICDConfigurationData ICDConfigurationData::instance;

System::Clock::Milliseconds32 ICDConfigurationData::GetSlowPollingInterval()
{
    // When LIT capable device operates in SIT mode, it shall transition to use the mSITPollingInterval
    // if this one is shorter than the configured mLITPollingInterval.
    // Either way, the slow poll interval used SHALL NOT be greater than the SIT mode polling threshold, per spec.
    // This is important for ICD device configured for LIT operation but currently operating as a SIT
    // due to a lack of client registration
    if (mFeatureMap.Has(app::Clusters::IcdManagement::Feature::kLongIdleTimeSupport) && mICDMode == ICDMode::SIT)
    {
        // mSITPollingInterval cannot be configured to a value greater than kSITPollingThreshold.
        // The SIT slow polling interval compliance is therefore always respected by using the smallest
        // value from mLITPollingInterval or mSITPollingInterval;
        return std::min(mLITPollingInterval, mSITPollingInterval);
    }

    return mLITPollingInterval;
}

CHIP_ERROR ICDConfigurationData::SetSlowPollingInterval(System::Clock::Milliseconds32 slowPollInterval)
{
    bool isLITSupported = mFeatureMap.Has(app::Clusters::IcdManagement::Feature::kLongIdleTimeSupport);
    // If LIT is not supported, the slow polling interval cannot be set higher than kSITPollingThreshold.
    VerifyOrReturnError((isLITSupported || slowPollInterval <= kSITPollingThreshold), CHIP_ERROR_INVALID_ARGUMENT);

    mLITPollingInterval = slowPollInterval;
    return CHIP_NO_ERROR;
};

CHIP_ERROR ICDConfigurationData::SetSITPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
    VerifyOrReturnError(pollingInterval <= kSITPollingThreshold, CHIP_ERROR_INVALID_ARGUMENT);
    mSITPollingInterval = pollingInterval;
    return CHIP_NO_ERROR;
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
