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

#include <app/clusters/energy-evse-server/energy-evse-server.h>
#include <credentials/FabricTable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>

#include <app-common/zap-generated/cluster-objects.h>

#include <ChargingTargetsMemMgr.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class EvseTargetsDelegate : public chip::FabricTable::Delegate
{
public:
    EvseTargetsDelegate();
    ~EvseTargetsDelegate();

    CHIP_ERROR Init(PersistentStorageDelegate * targetStore);

    /**
     * @brief Delegate should implement a handler for LoadTargets
     *
     * This needs to load any stored targets into memory
     */
    CHIP_ERROR LoadTargets();

    /**
     *  @brief   This returns a reference to the existing targets
     */
    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & GetTargets();

    /**
     * @brief   Copies a ChargingTargetSchedule into our store
     *
     * @param [in] an entry from the SetTargets list containing:
     *             dayOfWeekForSequence and chargingTargets (list)
     *
     * This routine scans the existing targets to see if we have a day of week
     * set that matches the new target dayOfWeek bits. If there is an existing
     * matching day then it replaces the days existing targets with the new entry
     */
    CHIP_ERROR SetTargets(
        const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedulesChanges);

    /**
     *  @brief   This deletes all targets and resets the list to empty
     */
    CHIP_ERROR ClearTargets();

    /**
     * Part of the FabricTable::Delegate interface. Gets called when a fabric is deleted, such as on FabricTable::Delete().
     **/
    virtual void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

private:
    // This is the upper bound in bytes of the TLV storage required to store the chargingTargetSchedulesList
    static uint16_t GetTlvSizeUpperBound();

    CHIP_ERROR SaveTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedulesList);

    // For debug purposes
    void PrintTargets(const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules);

protected:
    enum class TargetEntryTag : uint8_t
    {
        kTargetEntry           = 1,
        kDayOfWeek             = 2,
        kChargingTargetsList   = 3,
        kChargingTargetsStruct = 4,
        kTargetTime            = 5,
        kTargetSoC             = 6,
        kAddedEnergy           = 7,
    };

private:
    // Object to handle the allocation of memory for the chargingTargets
    ChargingTargetsMemMgr mChargingTargets;

    // Need memory to store the ChargingTargetScheduleStruct as this is pointed to from a
    // List<ChargingTargetScheduleStruct::Type>
    Structs::ChargingTargetScheduleStruct::Type mChargingTargetSchedulesArray[kEvseTargetsMaxNumberOfDays];

    // The current Target definition
    DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> mChargingTargetSchedulesList;

    // Pointer to the PeristentStorage
    PersistentStorageDelegate * mpTargetStore = nullptr;

    // Need a key to store the Charging Preference Targets which is a TLV of list of lists
    static constexpr const char * spEvseTargetsKeyName = "g/ev/targ";
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
