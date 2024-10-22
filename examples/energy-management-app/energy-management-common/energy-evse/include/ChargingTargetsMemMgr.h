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
#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

/*
 * The full Target data structure defined as:
 *
 * DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> chargingTargetSchedules;
 *
 * contains a list of ChargingTargetScheduleStructs which in turn contains a list of ChargingTargetStructs.
 * This means that somewhere the following memory needs to be allocated in the case where
 * the Target is at its maximum size:
 *
 * ChargingTargetStruct::Type mDailyChargingTargets[kEvseTargetsMaxNumberOfDays][kEvseTargetsMaxTargetsPerDay]
 *
 * This is 1680B.
 *
 * However it is likely the number of chargingTargets configured will be considerably less. To avoid
 * allocating the maximum possible Target size, each List<ChargingTargetScheduleStructs> is allocated
 * separately. This class handles that allocation.
 *
 * When iterating through the chargingTargetSchedules, an index in this list is kept and the
 * ChargingTargetsMemMgr::PrepareDaySchedule must be called so this object knows which day schedule it is tracking.
 * This will free any previous memory allocated for the day schedule in this object.
 *
 * There are then three usage cases:
 *
 * 1. When loading the Target from persistent storage. In this scenario, it is not known upfront
 *    how many chargingTargets are associated with this day schedule so ChargingTargetsMemMgr::AddChargingTarget()
 *    needs to be called as each individual chargingTarget is loaded from persistent data.
 *
 *    Once the chargingTargets for the day schedule have been loaded, ChargingTargetsMemMgr::AllocAndCopy() is
 *    called to allocate the memory to store the chargingTargets and the chargingTargets are copied.
 *
 * 2. When updating a Target and a day schedule is unaffected, the chargingTargets associated with
 *    day schedule need copying. The following should be called:
 *
 *    ChargingTargetsMemMgr::AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets)
 *
 * 3. When in SetTargets, a new list of chargingTargets needs to be added to a day schedule, the following
 *    should be called:
 *
 *    ChargingTargetsMemMgr::AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> &
 * chargingTargets)
 *
 * Having allocated and copied the chargingTargets accordingly, they can be added to a
 * DataModel::List<const Structs::ChargingTargetStruct::Type as follows:
 *
 * chargingTargetsList = DataModel::List<Structs::ChargingTargetStruct::Type>(ChargingTargetsMemMgr::GetChargingTargets(),
 * ChargingTargetsMemMgr::GetNumDailyChargingTargets());
 *
 * All memory allocated by this object is released When the ChargingTargetsMemMgr destructor is called.
 *
 */

class ChargingTargetsMemMgr
{
public:
    ChargingTargetsMemMgr();
    ~ChargingTargetsMemMgr();

    /**
     * @brief This method prepares a new day schedule. Subsequent calls to GetChargingTargets
     *        and the AllocAndCopy methods below will reference this day schedule.
     *
     * @param chargingTargetSchedulesIdx  - The new day schedule index
     */
    void PrepareDaySchedule(uint16_t chargingTargetSchedulesIdx);

    /**
     * @brief Called as each individual chargingTarget is loaded from persistent data.
     *        When loading the Target from persistent storage, it is not known upfront
     *        how many chargingTargets are associated with this day schedule so
     *        ChargingTargetsMemMgr::AddChargingTarget() needs to be called as each individual
     *        chargingTarget is loaded from persistent data.
     *
     * @param chargingTarget  - The chargingTarget that will be added into the current day schedule
     */
    void AddChargingTarget(const EnergyEvse::Structs::ChargingTargetStruct::Type & chargingTarget);

    /**
     * @brief Called to allocate and copy the chargingTargets added via AddChargingTarget into the
     *        current day schedule as set by PrepareDaySchedule().
     */
    CHIP_ERROR AllocAndCopy();

    /**
     * @brief Called to allocate and copy the chargingTargets into the current day schedule as set
     *        set by PrepareDaySchedule().
     *        If an attempt is made to add more than kEvseTargetsMaxTargetsPerDay chargingTargets
     *        for the current day schedule, then the chargingTarget is not added and an error message
     *        is printed.
     *
     * @param chargingTargets  - The chargingTargets to add into the current day schedule
     */
    CHIP_ERROR AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets);

    /**
     * @brief Called to allocate and copy the chargingTargets into the current day schedule as set
     *        set by PrepareDaySchedule().
     *
     * @param chargingTargets  - The chargingTargets to add into the current day schedule
     */
    CHIP_ERROR AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> & chargingTargets);

    /**
     * @brief Returns the list of chargingTargets associated with the current day schedule.
     *
     * @return The charging targets associated with the current day schedule.
     */
    EnergyEvse::Structs::ChargingTargetStruct::Type * GetChargingTargets() const;

    /**
     * @brief Returns the number of chargingTargets associated with current day schedule.
     *
     * @return Returns the number of chargingTargets associated with current day schedule.
     */
    uint16_t GetNumDailyChargingTargets() const;

private:
    EnergyEvse::Structs::ChargingTargetStruct::Type * mpListOfDays[kEvseTargetsMaxNumberOfDays];
    EnergyEvse::Structs::ChargingTargetStruct::Type mDailyChargingTargets[kEvseTargetsMaxTargetsPerDay];
    uint16_t mChargingTargetSchedulesIdx;
    uint16_t mNumDailyChargingTargets;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
