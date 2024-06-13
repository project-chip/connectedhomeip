/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <DEMUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;

using chip::app::LogEvent;
using chip::Protocols::InteractionModel::Status;

/**
 * @brief   Helper function to get current timestamp in Epoch format
 *
 * @param   chipEpoch reference to hold return timestamp
 */
CHIP_ERROR UtilsGetEpochTS(uint32_t & chipEpoch)
{
    chipEpoch = 0;

    System::Clock::Milliseconds64 cTMs;
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(cTMs);

    /* If the GetClock_RealTimeMS returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE, then
     * This platform cannot ever report real time !
     * This should not be certifiable since getting time is a Mandatory
     * feature of EVSE Cluster
     */
    VerifyOrDie(err != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "EVSE: Unable to get current time - err:%" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    auto unixEpoch = std::chrono::duration_cast<System::Clock::Seconds32>(cTMs).count();
    if (!UnixEpochToChipEpochTime(unixEpoch, chipEpoch))
    {
        ChipLogError(Zcl, "EVSE: unable to convert Unix Epoch time to Matter Epoch Time");
        return err;
    }

    return CHIP_NO_ERROR;
}

/**
 * @brief   Helper function to get current timestamp and work out the day of week
 *
 * NOTE that the time_t is converted using localtime to provide the timestamp
 * in local time. If this is not supported on some platforms an alternative
 * implementation may be required.
 *
 * @param   unixEpoch (as time_t)
 *
 * @return  bitmap value for day of week
 * Sunday = 0x01, Monday = 0x01 ... Saturday = 0x40 (1<<6)
 */
uint8_t UtilsGetDayOfWeekUnixEpoch(time_t unixEpoch)
{
    // Define a timezone structure and initialize it to the local timezone
    // This will capture any daylight saving time changes
    struct tm local_time;
    localtime_r(&unixEpoch, &local_time);

    // Get the day of the week (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
    uint8_t dayOfWeek = static_cast<uint8_t>(local_time.tm_wday);

    // Calculate the bitmap value based on the day of the week
    uint8_t bitmap = static_cast<uint8_t>(1 << dayOfWeek);

    return bitmap;
}
/**
 * @brief   Helper function to get current timestamp and work out the day of week based on localtime
 *
 * @param   reference to hold the day of week as a bitmap
 *
 * Sunday = 0x01, Monday = 0x01 ... Saturday = 0x40 (1<<6)
 */
CHIP_ERROR UtilsGetDayOfWeekNow(uint8_t & dayOfWeekMap)
{
    chip::System::Clock::Milliseconds64 cTMs;
    CHIP_ERROR err = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "EVSE: unable to get current time to check user schedules error=%" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    time_t unixEpoch = std::chrono::duration_cast<chip::System::Clock::Seconds32>(cTMs).count();
    dayOfWeekMap     = UtilsGetDayOfWeekUnixEpoch(unixEpoch);

    return CHIP_NO_ERROR;
}
