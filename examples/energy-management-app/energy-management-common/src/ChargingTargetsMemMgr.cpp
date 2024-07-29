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

#include <app-common/zap-generated/cluster-objects.h>

#include "ChargingTargetsMemMgr.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

ChargingTargetsMemMgr::ChargingTargetsMemMgr() : mChargingTargetSchedulesIdx(0), mNumDailyChargingTargets(0)
{
    memset(mpListOfDays, 0, sizeof(mpListOfDays));
}

ChargingTargetsMemMgr::~ChargingTargetsMemMgr()
{
    // Free all memory allocated for the charging targets
    for (uint16_t idx = 0; idx < kEvseTargetsMaxNumberOfDays; idx++)
    {
        if (mpListOfDays[idx] != nullptr)
        {
            chip::Platform::Delete(mpListOfDays[idx]);
        }
    }
}

void ChargingTargetsMemMgr::PrepareDaySchedule(uint16_t chargingTargetSchedulesIdx)
{
    // MUST be called for each entry in DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> chargingTargetSchedules
    mNumDailyChargingTargets = 0;

    // Should not occur but just to be safe
    if (chargingTargetSchedulesIdx >= kEvseTargetsMaxNumberOfDays)
    {
        ChipLogError(AppServer, "PrepareDaySchedule bad chargingTargetSchedulesIdx %u", chargingTargetSchedulesIdx);
        return;
    }

    mChargingTargetSchedulesIdx = chargingTargetSchedulesIdx;

    // Free up any memory associated with this targetSchedule
    if (mpListOfDays[mChargingTargetSchedulesIdx] != nullptr)
    {
        chip::Platform::MemoryFree(mpListOfDays[mChargingTargetSchedulesIdx]);
        mpListOfDays[mChargingTargetSchedulesIdx] = nullptr;
    }
}

void ChargingTargetsMemMgr::AddChargingTarget(const EnergyEvse::Structs::ChargingTargetStruct::Type & chargingTarget)
{
    if (mNumDailyChargingTargets < kEvseTargetsMaxTargetsPerDay)
    {
        mDailyChargingTargets[mNumDailyChargingTargets++] = chargingTarget;
    }
    else
    {
        ChipLogError(AppServer, "AddChargingTarget: trying to add too many chargingTargets");
    }
}

EnergyEvse::Structs::ChargingTargetStruct::Type * ChargingTargetsMemMgr::GetChargingTargets() const
{
    return mpListOfDays[mChargingTargetSchedulesIdx];
}

uint16_t ChargingTargetsMemMgr::GetNumDailyChargingTargets() const
{
    return mNumDailyChargingTargets;
}

CHIP_ERROR ChargingTargetsMemMgr::AllocAndCopy()
{
    // NOTE: ChargingTargetsMemMgr::PrepareDaySchedule() must be called as specified in the class comments in
    // ChargingTargetsMemMgr.h before this method can be called.

    VerifyOrDie(mpListOfDays[mChargingTargetSchedulesIdx] == nullptr);

    if (mNumDailyChargingTargets > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfDays[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * mNumDailyChargingTargets));

        VerifyOrReturnError(mpListOfDays[mChargingTargetSchedulesIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        for (uint16_t idx = 0; idx < mNumDailyChargingTargets; idx++)
        {
            // This will cause the ChargingTargetStruct constructor to be called and this element in the array
            new (mpListOfDays[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

            // Now copy the chargingTarget
            mpListOfDays[mChargingTargetSchedulesIdx][idx] = mDailyChargingTargets[idx];
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
ChargingTargetsMemMgr::AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets)
{
    // NOTE: ChargingTargetsMemMgr::PrepareDaySchedule() must be called as specified in the class comments in
    // ChargingTargetsMemMgr.h before this method can be called.

    VerifyOrDie(mpListOfDays[mChargingTargetSchedulesIdx] == nullptr);

    mNumDailyChargingTargets = static_cast<uint16_t>(chargingTargets.size());

    if (mNumDailyChargingTargets > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfDays[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * chargingTargets.size()));

        VerifyOrReturnError(mpListOfDays[mChargingTargetSchedulesIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        uint16_t idx = 0;
        for (auto & chargingTarget : chargingTargets)
        {
            // This will cause the ChargingTargetStruct constructor to be called and this element in the array
            new (mpListOfDays[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

            // Now copy the chargingTarget
            mpListOfDays[mChargingTargetSchedulesIdx][idx] = chargingTarget;

            idx++;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
ChargingTargetsMemMgr::AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> & chargingTargets)
{
    // NOTE: ChargingTargetsMemMgr::PrepareDaySchedule() must be called as specified in the class comments in
    // ChargingTargetsMemMgr.h before this method can be called.

    VerifyOrDie(mpListOfDays[mChargingTargetSchedulesIdx] == nullptr);

    size_t numDailyChargingTargets = 0;
    ReturnErrorOnFailure(chargingTargets.ComputeSize(&numDailyChargingTargets));

    mNumDailyChargingTargets = static_cast<uint16_t>(numDailyChargingTargets);

    if (mNumDailyChargingTargets > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfDays[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * mNumDailyChargingTargets));

        VerifyOrReturnError(mpListOfDays[mChargingTargetSchedulesIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        uint16_t idx = 0;
        auto it      = chargingTargets.begin();
        while (it.Next())
        {
            // Check that the idx is still valid
            VerifyOrReturnError(idx < mNumDailyChargingTargets, CHIP_ERROR_INCORRECT_STATE);

            auto & chargingTarget = it.GetValue();

            // This will cause the ChargingTargetStruct constructor to be called and this element in the array
            new (mpListOfDays[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

            // Now copy the chargingTarget
            mpListOfDays[mChargingTargetSchedulesIdx][idx] = chargingTarget;

            idx++;
        }
    }

    return CHIP_NO_ERROR;
}
