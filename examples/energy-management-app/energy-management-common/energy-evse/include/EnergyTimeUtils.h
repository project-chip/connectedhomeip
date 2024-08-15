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

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/util/config.h>
#include <cstring>
#include <lib/core/CHIPError.h>
#include <lib/support/BitMask.h>
#include <protocols/Protocols.h>
#include <time.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

/**
 * @brief   Helper function to get current timestamp in Epoch format
 *
 * @param   chipEpoch reference to hold return timestamp
 */
CHIP_ERROR GetEpochTS(uint32_t & chipEpoch);

/**
 * @brief   Helper function to get current timestamp and work out the day of week
 *
 * NOTE that the time_t is converted using localtime to provide the timestamp
 * in local time. If this is not supported on some platforms an alternative
 * implementation may be required.
 *
 * @param   unixEpoch (as time_t)
 *
 * @return  bitmap value for day of week as defined by EnergyEvse::TargetDayOfWeekBitmap. Note
 *          only one bit will be set for the day of the week.
 */
BitMask<EnergyEvse::TargetDayOfWeekBitmap> GetLocalDayOfWeekFromUnixEpoch(time_t unixEpoch);

/**
 * @brief   Helper function to get current timestamp and work out the day of week based on localtime
 *
 * @param   reference to hold the day of week as a bitmap as defined by EnergyEvse::TargetDayOfWeekBitmap.
 *          Note only one bit will be set for the current day.
 */
CHIP_ERROR GetLocalDayOfWeekNow(BitMask<EnergyEvse::TargetDayOfWeekBitmap> & dayOfWeekMap);

/**
 * @brief   Helper function to get current timestamp and work out the current number of minutes
 *          past midnight based on localtime
 *
 * @param   reference to hold the number of minutes past midnight
 */
CHIP_ERROR GetMinutesPastMidnight(uint16_t & minutesPastMidnight);

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
