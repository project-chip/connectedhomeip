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

CHIP_ERROR ICDConfigurationData::SetModeDurations(uint32_t activeModeDuration_ms, uint32_t idleModeInterval_s)
{
    VerifyOrReturnError(activeModeDuration_ms <= (idleModeInterval_s * 1000), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(idleModeInterval_s <= kMaxIdleModeDuration_s, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(idleModeInterval_s >= kMinIdleModeDuration_s, CHIP_ERROR_INVALID_ARGUMENT);

    mIdleModeDuration_s    = idleModeInterval_s;
    mActiveModeDuration_ms = activeModeDuration_ms;

    return CHIP_NO_ERROR;
}

} // namespace chip
