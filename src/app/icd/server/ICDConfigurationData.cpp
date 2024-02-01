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

CHIP_ERROR ICDConfigurationData::SetModeDurations(Optional<uint32_t> activeModeDuration_ms, Optional<uint32_t> idleModeDuration_ms)
{
    VerifyOrReturnError(activeModeDuration_ms.HasValue() || idleModeDuration_ms.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

    uint32_t tmpIdleModeDuration_s =
        idleModeDuration_ms.HasValue() ? (idleModeDuration_ms.Value() / kMillisecondsPerSecond) : mIdleModeDuration_s;
    uint32_t tmpActiveModeDuration_ms = activeModeDuration_ms.HasValue() ? activeModeDuration_ms.Value() : mActiveModeDuration_ms;

    VerifyOrReturnError(tmpActiveModeDuration_ms <= (tmpIdleModeDuration_s * kMillisecondsPerSecond), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration_s <= kMaxIdleModeDuration_s, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(tmpIdleModeDuration_s >= kMinIdleModeDuration_s, CHIP_ERROR_INVALID_ARGUMENT);

    mIdleModeDuration_s    = tmpIdleModeDuration_s;
    mActiveModeDuration_ms = tmpActiveModeDuration_ms;

    return CHIP_NO_ERROR;
}

} // namespace chip
