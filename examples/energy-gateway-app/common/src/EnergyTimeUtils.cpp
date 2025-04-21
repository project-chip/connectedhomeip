/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <EnergyTimeUtils.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

/**
 * @brief   Helper function to get current timestamp in Epoch format
 *
 * @param[out]   chipEpoch reference to hold return timestamp. Set to 0 if an error occurs.
 */
CHIP_ERROR GetEpochTS(uint32_t & chipEpoch)
{
    chipEpoch = 0;

    System::Clock::Milliseconds64 cTMs;
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(cTMs);

    /* If the GetClock_RealTimeMS returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, then
     * This platform cannot ever report real time !
     */
    VerifyOrDie(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Unable to get current time - err:%" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    auto unixEpoch = std::chrono::duration_cast<System::Clock::Seconds32>(cTMs).count();
    if (!UnixEpochToChipEpochTime(unixEpoch, chipEpoch))
    {
        ChipLogError(Zcl, "Unable to convert Unix Epoch time to Matter Epoch Time");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}
