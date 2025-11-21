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
    std::optional<System::Clock::Seconds32> tmpIdleModeDuration = std::nullopt;
    if (idleModeDuration.HasValue())
        tmpIdleModeDuration = std::chrono::duration_cast<System::Clock::Seconds32>(idleModeDuration.Value());

    return SetModeDurations(activeModeDuration.std_optional(), tmpIdleModeDuration, std::nullopt);
}

CHIP_ERROR ICDConfigurationData::SetModeDurations(std::optional<System::Clock::Milliseconds32> activeModeDuration,
                                                  std::optional<System::Clock::Seconds32> idleModeDuration,
                                                  std::optional<System::Clock::Seconds32> shortIdleModeDuration)
{
    VerifyOrReturnError(activeModeDuration.has_value() || idleModeDuration.has_value() || shortIdleModeDuration.has_value(),
                        CHIP_ERROR_INVALID_ARGUMENT);

    System::Clock::Milliseconds32 tmpActiveModeDuration = activeModeDuration.value_or(mActiveModeDuration);
    System::Clock::Seconds32 tmpIdleModeDuration        = idleModeDuration.value_or(mIdleModeDuration);

    VerifyOrReturnError(tmpActiveModeDuration <= tmpIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration <= kMaxIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration >= kMinIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);

    System::Clock::Seconds32 tmpShortIdleModeDuration;
    if (shortIdleModeDuration.has_value())
    {
        // shortIdleModeDuration was provided, it shall be lesser than or equal to idleModeDuration.
        tmpShortIdleModeDuration = shortIdleModeDuration.value();
        VerifyOrReturnError(tmpShortIdleModeDuration <= tmpIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        // shortIdleModeDuration was not provided. To ensure correct mode transitions and device compliance,
        // shortIdleModeDuration must not exceed idleModeDuration, so we use the smaller of the current shortIdleModeDuration
        // and the resultant idleModeDuration.
        // This approach overwrite a previous valid shortIdleModeDuration rather than erroing the call but maitains the previous api
        // behavior.
        tmpShortIdleModeDuration = std::min(mShortIdleModeDuration, tmpIdleModeDuration);
    }

    VerifyOrReturnError(tmpShortIdleModeDuration <= tmpIdleModeDuration, CHIP_ERROR_INVALID_ARGUMENT);

    mIdleModeDuration      = tmpIdleModeDuration;
    mActiveModeDuration    = tmpActiveModeDuration;
    mShortIdleModeDuration = tmpShortIdleModeDuration;

    return CHIP_NO_ERROR;
}

bool ICDConfigurationData::ShouldUseShortIdle()
{
    VerifyOrReturnValue(mShortIdleModeDuration < mIdleModeDuration, false);
    return (mFeatureMap.Has(app::Clusters::IcdManagement::Feature::kLongIdleTimeSupport) && mICDMode == ICDMode::SIT);
}

System::Clock::Seconds32 ICDConfigurationData::GetModeBasedIdleModeDuration()
{
    return ShouldUseShortIdle() ? mShortIdleModeDuration : mIdleModeDuration;
}

} // namespace chip
