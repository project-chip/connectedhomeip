/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>

#include <app/util/af.h>
#include <app/util/config.h>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class EvseTargetsStore
{
public:
    EvseTargetsStore() {}
    virtual ~EvseTargetsStore() { Finish(); }

    // Non-copyable
    EvseTargetsStore(EvseTargetsStore const &) = delete;
    void operator=(EvseTargetsStore const &)   = delete;

    /**
     * @brief Initialize the EVSE Targets store to map to a given storage delegate.
     *
     * @param storage Pointer to persistent storage delegate to use. Must outlive this instance.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if already initialized
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Finalize the EVSE Targets store, so that subsequent operations fail
     */
    void Finish()
    {
        VerifyOrReturn(mStorage != nullptr);
        mStorage = nullptr;
    }

    CHIP_ERROR LoadTargetsFromStore();

protected:
    static constexpr uint8_t kEnergyEvseTargetsTLVMaxSize = 240; // TODO
    PersistentStorageDelegate * mStorage                  = nullptr;
};

struct EnergyEvseTargetEntry
{
    uint16_t targetTimeMinutesPastMidnight;
    chip::Percent targetSoC;
    int64_t addedEnergy;
};

struct EnergyEvseTargetDayEntry
{
    chip::BitMask<TargetDayOfWeekBitmap> dayOfWeekMap;
    DataModel::List<EnergyEvseTargetEntry> dailyChargingTargets;
};

class EvseTargets
{

public:
    /**
     * @brief   Copies a ChargingTargetSchedule into our mTargets
     *
     * @param [in] an entry from the SetTargets list containing:
     *             dayOfWeekForSequence and chargingTargets (list)
     *
     * This routine scans the existing mTargets to see if we have a day of week
     * set that matches the new target dayOfWeek bits. If there is an existing
     * matching day then it replaces the days existing targets with the new entry
     */
    CHIP_ERROR CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType &);

    /**
     *  @brief   This deletes all targets and resets the list to empty
     */
    CHIP_ERROR ClearTargets();

private:
    static constexpr uint8_t kMaxNumberOfDays                  = 7;
    static constexpr uint8_t kMaxNumberOfChargingTargetsPerDay = 10;

    // EnergyEvseTargetEntry mTargets[kMaxNumberOfDays][kMaxNumberOfChargingTargetsPerDay];
    DataModel::List<EnergyEvseTargetDayEntry> mTargets;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
