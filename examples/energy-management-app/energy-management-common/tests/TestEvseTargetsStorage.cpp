/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inttypes.h>

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <gtest/gtest.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include "EnergyEvseTargetsStore.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

namespace {

constexpr uint16_t ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK       = 7;
constexpr uint16_t ENERGY_EVSE_SET_TARGETS_MAX_CHARGING_TARGETS = 10;

class TestEvseTargetsStorage : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    bool CompTargets(const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & targets1,
                     const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & targets2);

    void PopulateTargets(uint16_t numDays, uint16_t numChargingTargetsPerDay);

    void SetTargets();
    void CheckTargets();

private:
    uint8_t mStore[4096];

    EnergyEvse::Structs::ChargingTargetScheduleStruct::Type mChargingTargetSchedules[ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK];
    EnergyEvse::Structs::ChargingTargetStruct::Type mChargingTargets[ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK]
                                                                    [ENERGY_EVSE_SET_TARGETS_MAX_CHARGING_TARGETS];
    chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>
        mChargingTargetsList[ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK];

    chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> mRefChargingTargetSchedulesList;
    DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> mDecodableChargingTargetSchedulesList;

    TestPersistentStorageDelegate mStorageDelegate;

    EvseTargetsDelegate mEtd;
    bool mEtdInitialised = false;
};

TEST_F(TestEvseTargetsStorage, TestEmpty)
{
    PopulateTargets(0, 0);
    SetTargets();
    CheckTargets();
}

TEST_F(TestEvseTargetsStorage, TestFull)
{
    PopulateTargets(ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK, ENERGY_EVSE_SET_TARGETS_MAX_CHARGING_TARGETS);
    SetTargets();
    CheckTargets();
}

TEST_F(TestEvseTargetsStorage, TestPartial1)
{
    PopulateTargets(ENERGY_EVSE_SET_TARGETS_DAYS_IN_A_WEEK, 1);
    SetTargets();
    CheckTargets();
}

TEST_F(TestEvseTargetsStorage, TestPartial2)
{
    PopulateTargets(1, ENERGY_EVSE_SET_TARGETS_MAX_CHARGING_TARGETS);
    SetTargets();
    CheckTargets();
}

bool TestEvseTargetsStorage::CompTargets(const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & targets1,
                                         const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & targets2)
{
    if (targets1.size() != targets2.size())
    {
        ChipLogError(AppServer, "CompTargets: Different number of ChargingTargetScheduleStruct in lists");
        return false;
    }

    uint16_t dayIdx = 0;
    for (const auto & entry1 : targets1)
    {
        const auto & entry2 = targets2[dayIdx++];

        if (entry1.dayOfWeekForSequence != entry2.dayOfWeekForSequence)
        {
            ChipLogError(AppServer, "CompTargets: Different dayOfWeekForSequence");
            return false;
        }

        if (entry1.chargingTargets.size() != entry2.chargingTargets.size())
        {
            ChipLogError(AppServer, "CompTargets: Different number of chargingTargets in day list");
            return false;
        }

        uint16_t chargingTargetsIdx = 0;
        for (const auto & targetStruct1 : entry1.chargingTargets)
        {
            const auto & targetStruct2 = entry2.chargingTargets[chargingTargetsIdx++];

            if (targetStruct1.targetTimeMinutesPastMidnight != targetStruct2.targetTimeMinutesPastMidnight)
            {
                ChipLogError(AppServer, "CompTargets: Different targetTimeMinutesPastMidnight");
                return false;
            }

            if (targetStruct1.targetSoC != targetStruct2.targetSoC)
            {
                ChipLogError(AppServer, "CompTargets: Different targetSoC");
                return false;
            }

            if (targetStruct1.addedEnergy != targetStruct2.addedEnergy)
            {
                ChipLogError(AppServer, "CompTargets: Different addedEnergy");
                return false;
            }
        }
    }

    return true;
}

void TestEvseTargetsStorage::PopulateTargets(uint16_t numDays, uint16_t numChargingTargetsPerDay)
{
    for (uint16_t dayIdx = 0; dayIdx < numDays; dayIdx++)
    {
        for (uint16_t chargingTargetIdx = 0; chargingTargetIdx < numChargingTargetsPerDay; chargingTargetIdx++)
        {
            mChargingTargets[dayIdx][chargingTargetIdx].targetTimeMinutesPastMidnight =
                static_cast<uint16_t>(dayIdx * 60 + chargingTargetIdx);
            mChargingTargets[dayIdx][chargingTargetIdx].targetSoC.SetValue(65);
            mChargingTargets[dayIdx][chargingTargetIdx].addedEnergy.SetValue(400);
        }

        mChargingTargetsList[dayIdx] = chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
            mChargingTargets[dayIdx], numChargingTargetsPerDay);

        mChargingTargetSchedules[dayIdx].dayOfWeekForSequence.Set(static_cast<EnergyEvse::TargetDayOfWeekBitmap>(1 << dayIdx));
        mChargingTargetSchedules[dayIdx].chargingTargets = mChargingTargetsList[dayIdx];
    }

    chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> chargingTargetSchedulesList(
        mChargingTargetSchedules, numDays);

    mRefChargingTargetSchedulesList = chargingTargetSchedulesList;

    TLV::TLVReader mReader;
    TLV::TLVWriter mWriter;

    mWriter.Init(mStore, sizeof(mStore));

    CHIP_ERROR err = DataModel::Encode(mWriter, TLV::AnonymousTag(), mRefChargingTargetSchedulesList);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(mWriter.Finalize(), CHIP_NO_ERROR);

    mReader.Init(mStore);
    EXPECT_EQ(mReader.Next(), CHIP_NO_ERROR);

    err = DataModel::Decode(mReader, mDecodableChargingTargetSchedulesList);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestEvseTargetsStorage::SetTargets()
{
    if (!mEtdInitialised)
    {
        mEtd.Init(&mStorageDelegate);

        mEtdInitialised = true;
    }

    CHIP_ERROR err = mEtd.SetTargets(mDecodableChargingTargetSchedulesList);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestEvseTargetsStorage::CheckTargets()
{
    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> targets = mEtd.GetTargets();

    EXPECT_TRUE(CompTargets(mRefChargingTargetSchedulesList, targets));
}

} // namespace
