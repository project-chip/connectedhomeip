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

ChargingTargetsMemMgr::ChargingTargetsMemMgr() : mChargingTargetSchedulesIdx(0), mNumChargingTargets(0)
{
    memset(mpChargingTargets, 0, sizeof(mpChargingTargets));
}

ChargingTargetsMemMgr::~ChargingTargetsMemMgr()
{
    // Free all memory allocated for the charging targets
    for (uint16_t idx = 0; idx < kEvseTargetsMaxNumberOfDays; idx++)
    {
        if (mpChargingTargets[idx] != nullptr)
        {
            chip::Platform::Delete(mpChargingTargets[idx]);
        }
    }
}

void ChargingTargetsMemMgr::Reset(uint16_t chargingTargetSchedulesIdx)
{
    // MUST be called for each entry in DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> chargingTargetSchedules
    mNumChargingTargets         = 0;
    mChargingTargetSchedulesIdx = chargingTargetSchedulesIdx;

    // Free up any memory associated with this targetSchedule
    if (mpChargingTargets[mChargingTargetSchedulesIdx] != nullptr)
    {
        chip::Platform::MemoryFree(mpChargingTargets[mChargingTargetSchedulesIdx]);
        mpChargingTargets[mChargingTargetSchedulesIdx] = nullptr;
    }
}

void ChargingTargetsMemMgr::AddChargingTarget(EnergyEvse::Structs::ChargingTargetStruct::Type & chargingTarget)
{
    if (mNumChargingTargets < kEvseTargetsMaxTargetsPerDay)
    {
        mChargingTargets[mNumChargingTargets++] = chargingTarget;
    }
}

EnergyEvse::Structs::ChargingTargetStruct::Type * ChargingTargetsMemMgr::GetChargingTargets() const
{
    return mpChargingTargets[mChargingTargetSchedulesIdx];
}

uint16_t ChargingTargetsMemMgr::GetNumChargingTargets() const
{
    return mNumChargingTargets;
}

void ChargingTargetsMemMgr::AllocAndCopy()
{
    if (mNumChargingTargets > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpChargingTargets[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * mNumChargingTargets));

        for (uint16_t idx = 0; idx < mNumChargingTargets; idx++)
        {
            // This will cause the ChargingTargetStruct constructor to be called and this element in the array
            new (mpChargingTargets[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

            // Now copy the chargingTarget
            mpChargingTargets[mChargingTargetSchedulesIdx][idx] = mChargingTargets[idx];
        }
    }
}

void ChargingTargetsMemMgr::AllocAndCopy(const DataModel::List<const Structs::ChargingTargetStruct::Type> & chargingTargets)
{
    mNumChargingTargets = static_cast<uint16_t>(chargingTargets.size());

    if (mNumChargingTargets > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpChargingTargets[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * chargingTargets.size()));

        uint16_t idx = 0;
        for (auto & chargingTarget : chargingTargets)
        {
            // This will cause the ChargingTargetStruct constructor to be called and this element in the array
            new (mpChargingTargets[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

            // Now copy the chargingTarget
            mpChargingTargets[mChargingTargetSchedulesIdx][idx] = chargingTarget;

            idx++;
        }
    }
}

CHIP_ERROR
ChargingTargetsMemMgr::AllocAndCopy(const DataModel::DecodableList<Structs::ChargingTargetStruct::DecodableType> & chargingTargets)
{
    size_t numChargingTargets = 0;
    CHIP_ERROR err            = chargingTargets.ComputeSize(&numChargingTargets);
    if (err == CHIP_NO_ERROR)
    {
        mNumChargingTargets = static_cast<uint16_t>(numChargingTargets);

        if (mNumChargingTargets > 0)
        {
            // Allocate the memory first and then use placement new to initialise the memory of each element in the array
            mpChargingTargets[mChargingTargetSchedulesIdx] = static_cast<EnergyEvse::Structs::ChargingTargetStruct::Type *>(
                chip::Platform::MemoryAlloc(sizeof(EnergyEvse::Structs::ChargingTargetStruct::Type) * mNumChargingTargets));

            uint16_t idx = 0;
            auto it      = chargingTargets.begin();
            while (it.Next())
            {
                auto & chargingTarget = it.GetValue();

                // This will cause the ChargingTargetStruct constructor to be called and this element in the array
                new (mpChargingTargets[mChargingTargetSchedulesIdx] + idx) EnergyEvse::Structs::ChargingTargetStruct::Type();

                // Now copy the chargingTarget
                mpChargingTargets[mChargingTargetSchedulesIdx][idx] = chargingTarget;

                idx++;
            }
        }
    }

    return err;
}
