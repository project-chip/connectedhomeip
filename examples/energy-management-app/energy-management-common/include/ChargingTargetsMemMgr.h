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
 * ChargingTargetsMemMgr::Reset must be called so this object knows which day schedule it is tracking.
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
     * @brief This method sets the index (mChargingTargetSchedulesIdx) to use into mpListOfDays.
     *        This index is used in subsequent calls to GetChargingTargets and the AllocAndCopy
     *        methods below.
     *
     *        NOTE: This method MUST be called each time a new day is added to
     *        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> chargingTargetSchedules
     *        before building up the ChargingTargetStruct associated with the mpListOfDays entry.
     *
     * @param chargingTargetSchedulesIdx  - The new index to use when accessing mpListOfDays
     */
    void Reset(uint16_t chargingTargetSchedulesIdx);

    /**
     * @brief Called as each individual chargingTarget is loaded from persistent data.
     *        When loading the Target from persistent storage, it is not known upfront
     *        how many chargingTargets are associated with this day schedule so
     *        ChargingTargetsMemMgr::AddChargingTarget() needs to be called as each individual
     *        chargingTarget is loaded from persistent data.
     *
     * @param chargingTarget  - The chargingTarget to add into mpListOfDays[mChargingTargetSchedulesIdx]
     */
    void AddChargingTarget(const EnergyEvse::Structs::ChargingTargetStruct::Type & chargingTarget);

    /**
     * @brief Called to allocate and copy the chargingTargets in mDailyChargingTargets to
     *        mpListOfDays[mChargingTargetSchedulesIdx].
     *        This method is used once a days's worth of chargingTargets have been loaded from persistent
     *        storage.
     */
    CHIP_ERROR AllocAndCopy();

    /**
     * @brief Called to allocate and copy the chargingTargets in the parameter chargingTargets to
     *        mpListOfDays[mChargingTargetSchedulesIdx].
     *
     * @param chargingTargets  - The chargingTargets to add into mpListOfDays[mChargingTargetSchedulesIdx]
     */
    CHIP_ERROR AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets);

    /**
     * @brief Called to allocate and copy the chargingTargets in the parameter chargingTargets to
     *        mpListOfDays[mChargingTargetSchedulesIdx].
     *
     * @param chargingTargets  - The chargingTargets to add into mpListOfDays[mChargingTargetSchedulesIdx]
     */
    CHIP_ERROR AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> & chargingTargets);

    /**
     * @brief Returns the list of chargingTargets into mpListOfDays dependant on mChargingTargetSchedulesIdx.
     *
     * @return mpListOfDays[mChargingTargetSchedulesIdx]
     */
    EnergyEvse::Structs::ChargingTargetStruct::Type * GetChargingTargets() const;

    /**
     * @brief Returns the number of chargingTargets associated with current day (mChargingTargetSchedulesIdx).
     *
     * @return Returns mNumDailyChargingTargets.
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
