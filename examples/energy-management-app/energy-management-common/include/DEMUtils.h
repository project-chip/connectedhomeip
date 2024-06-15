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

#include <protocols/interaction_model/StatusCode.h>
#include <protocols/Protocols.h>
#include <app/util/config.h>
#include <cstring>

using chip::Protocols::InteractionModel::Status;

/**
 * @brief   Helper function to get current timestamp in Epoch format
 *
 * @param   chipEpoch reference to hold return timestamp
 */
CHIP_ERROR UtilsGetEpochTS(uint32_t & chipEpoch);

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
uint8_t UtilsGetLocalDayOfWeekFromUnixEpoch(time_t unixEpoch);

/**
 * @brief   Helper function to get current timestamp and work out the day of week based on localtime
 *
 * @param   reference to hold the day of week as a bitmap
 *
 * Sunday = 0x01, Monday = 0x01 ... Saturday = 0x40 (1<<6)
 */
CHIP_ERROR UtilsGetLocalDayOfWeekNow(uint8_t & dayOfWeekMap);
