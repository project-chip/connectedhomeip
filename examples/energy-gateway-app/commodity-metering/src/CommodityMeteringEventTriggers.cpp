/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <CommodityMeteringMain.h>
#include <app/clusters/commodity-metering-server/CommodityMeteringTestEventTriggerHandler.h>

#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityMetering;

namespace MeteredQuantitySamples {
// Define component arrays as constexpr
namespace Sample1 {
static constexpr uint32_t TariffComponents1[] = { 1011, 1012 };
static constexpr uint32_t TariffComponents2[] = { 1021, 1022, 1023 };

// Non-constexpr storage for the actual data
static const Structs::MeteredQuantityStruct::Type Data[] = {
    { .tariffComponentIDs = DataModel::List(TariffComponents1, MATTER_ARRAY_SIZE(TariffComponents1)),
      .quantity           = MATTER_ARRAY_SIZE(TariffComponents1) },
    { .tariffComponentIDs = DataModel::List(TariffComponents2, MATTER_ARRAY_SIZE(TariffComponents2)),
      .quantity           = MATTER_ARRAY_SIZE(TariffComponents2) }
};
} // namespace Sample1

namespace Sample2 {
static constexpr uint32_t TariffComponents1[] = { 2011 };
static constexpr uint32_t TariffComponents2[] = { 2021, 2023 };
static constexpr uint32_t TariffComponents3[] = { 2031, 2032, 2033 };

static const Structs::MeteredQuantityStruct::Type Data[] = {
    { .tariffComponentIDs = DataModel::List(TariffComponents1, MATTER_ARRAY_SIZE(TariffComponents1)),
      .quantity           = MATTER_ARRAY_SIZE(TariffComponents1) },
    { .tariffComponentIDs = DataModel::List(TariffComponents2, MATTER_ARRAY_SIZE(TariffComponents2)),
      .quantity           = MATTER_ARRAY_SIZE(TariffComponents2) },
    { .tariffComponentIDs = DataModel::List(TariffComponents3, MATTER_ARRAY_SIZE(TariffComponents2)),
      .quantity           = MATTER_ARRAY_SIZE(TariffComponents3) }
};
} // namespace Sample2
} // namespace MeteredQuantitySamples

namespace {

class TestDataManager
{
    static constexpr size_t MAX_MQ_SAMPLES = 4;

private:
    Instance * mInstance = nullptr;

    DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> mMeteredQuantity;
    DataModel::Nullable<uint32_t> mMeteredQuantityTimestamp;
    DataModel::Nullable<Globals::TariffUnitEnum> mTariffUnit;
    DataModel::Nullable<uint16_t> mMaximumMeteredQuantities;

    std::pair<std::array<const Structs::MeteredQuantityStruct::Type, MAX_MQ_SAMPLES>, size_t>
    GetMeteredQuantityDataSample(uint8_t presetIdx)
    {
        switch (presetIdx)
        {
        case 0:
            return { { MeteredQuantitySamples::Sample1::Data[0], MeteredQuantitySamples::Sample1::Data[1] },
                     MATTER_ARRAY_SIZE(MeteredQuantitySamples::Sample1::Data) };
        case 1:
            return { { MeteredQuantitySamples::Sample2::Data[0], MeteredQuantitySamples::Sample2::Data[1],
                       MeteredQuantitySamples::Sample2::Data[2] },
                     MATTER_ARRAY_SIZE(MeteredQuantitySamples::Sample2::Data) };
        default:
            return {}; // Return empty array
        }
    }

    void SaveMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & newValue)
    {
        // Clear existing data if any
        ClearMeteredQuantity();

        if (!newValue.IsNull())
        {
            const auto & sourceList = newValue.Value();
            const size_t count      = sourceList.size();

            if (count > 0)
            {
                // Allocate memory for the new list
                auto * newList = static_cast<Structs::MeteredQuantityStruct::Type *>(
                    Platform::MemoryCalloc(count, sizeof(Structs::MeteredQuantityStruct::Type)));
                if (newList == nullptr)
                {
                    ChipLogError(Zcl, "Failed to allocate memory for MeteredQuantity");
                    return;
                }

                // Copy each element
                for (size_t i = 0; i < count; i++)
                {
                    const auto & src = sourceList[i];

                    auto * components =
                        static_cast<uint32_t *>(Platform::MemoryCalloc(src.tariffComponentIDs.size(), sizeof(uint32_t)));
                    if (components == nullptr)
                    {
                        // Clean up previously allocated memory
                        Platform::MemoryFree(newList);
                        ChipLogError(Zcl, "Failed to allocate memory for tariffComponentIDs");
                        return;
                    }

                    // Copy component IDs
                    std::copy(src.tariffComponentIDs.begin(), src.tariffComponentIDs.end(), components);

                    // Create new entry
                    newList[i] = Structs::MeteredQuantityStruct::Type{
                        DataModel::List<const uint32_t>(components, src.tariffComponentIDs.size()), src.quantity
                    };
                }

                // Create the new list
                mMeteredQuantity = DataModel::MakeNullable(DataModel::List<Structs::MeteredQuantityStruct::Type>(newList, count));
            }
            else
            {
                // Empty list case
                mMeteredQuantity = DataModel::MakeNullable(DataModel::List<Structs::MeteredQuantityStruct::Type>());
            }
        }
    }

    void ClearMeteredQuantity()
    {
        if (!mMeteredQuantity.IsNull())
        {
            for (auto & item : mMeteredQuantity.Value())
            {
                if (!item.tariffComponentIDs.empty())
                {
                    Platform::MemoryFree(const_cast<uint32_t *>(item.tariffComponentIDs.data()));
                }
            }
            Platform::MemoryFree(const_cast<Structs::MeteredQuantityStruct::Type *>(mMeteredQuantity.Value().data()));
            mMeteredQuantity.SetNull();
        }
    }

    void SaveAttributes()
    {
        mInstance = GetCommodityMeteringInstance();
        VerifyOrDieWithMsg(mInstance, AppServer, "CommodityMetering instance is null");
        mMaximumMeteredQuantities = mInstance->GetMaximumMeteredQuantities();
        SaveMeteredQuantity(mInstance->GetMeteredQuantity());
        mMeteredQuantityTimestamp = mInstance->GetMeteredQuantityTimestamp();
        mTariffUnit               = mInstance->GetTariffUnit();
    }

    void ClearAttributes()
    {
        ClearMeteredQuantity();
        mMeteredQuantityTimestamp.SetNull();
        mTariffUnit.SetNull();
        mMaximumMeteredQuantities.SetNull();
    }

    void RestoreAttributes() const
    {
        if (mInstance)
        {
            TEMPORARY_RETURN_IGNORED mInstance->SetMaximumMeteredQuantities(mMaximumMeteredQuantities);
            TEMPORARY_RETURN_IGNORED mInstance->SetMeteredQuantity(mMeteredQuantity);
            TEMPORARY_RETURN_IGNORED mInstance->SetMeteredQuantityTimestamp(mMeteredQuantityTimestamp);
            TEMPORARY_RETURN_IGNORED mInstance->SetTariffUnit(mTariffUnit);
        }
    }

    void UpdAttrs()
    {
        uint32_t matterEpoch = 0;

        CHIP_ERROR err = System::Clock::GetClock_MatterEpochS(matterEpoch);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogDetail(Support, "UpdAttrs() got time: %" PRIu32, matterEpoch);
            mMeteredQuantityTimestamp.SetNonNull(matterEpoch);
        }
        else
        {
            ChipLogError(Support, "UpdAttrs() could not get time, setting Null");
            mMeteredQuantityTimestamp.SetNull();
        }

        if (mTariffUnit.IsNull() || (mTariffUnit.Value() == Globals::TariffUnitEnum::kKWh))
        {
            mTariffUnit.SetNonNull(Globals::TariffUnitEnum::kKVAh);
        }
        else
        {
            mTariffUnit.SetNonNull(Globals::TariffUnitEnum::kKWh);
        }

        auto MQSamplePair =
            GetMeteredQuantityDataSample(static_cast<uint8_t>(mTariffUnit.Value() == Globals::TariffUnitEnum::kKWh));

        std::array<Structs::MeteredQuantityStruct::Type, MAX_MQ_SAMPLES> mqBuffer;

        std::copy(MQSamplePair.first.begin(), MQSamplePair.first.begin() + static_cast<std::ptrdiff_t>(MQSamplePair.second),
                  mqBuffer.data());

        DataModel::List<Structs::MeteredQuantityStruct::Type> tmpList(mqBuffer.data(), MQSamplePair.second);
        DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> nullableList;
        nullableList.SetNonNull(std::move(tmpList));

        mMaximumMeteredQuantities.SetNonNull(MQSamplePair.second);

        TEMPORARY_RETURN_IGNORED mInstance->SetMaximumMeteredQuantities(mMaximumMeteredQuantities);
        TEMPORARY_RETURN_IGNORED mInstance->SetMeteredQuantity(nullableList);
        TEMPORARY_RETURN_IGNORED mInstance->SetMeteredQuantityTimestamp(mMeteredQuantityTimestamp);
        TEMPORARY_RETURN_IGNORED mInstance->SetTariffUnit(mTariffUnit);
    }

public:
    TestDataManager(){};
    ~TestDataManager() = default;

    void Update()
    {
        if (!mInstance)
        {
            SaveAttributes();
        }

        UpdAttrs();
    }

    void Clear()
    {
        ClearAttributes();
        RestoreAttributes();
    }
};

TestDataManager mTestDataManager;
} // namespace

bool HandleCommodityMeteringTestEventTrigger(uint64_t eventTrigger)
{
    CommodityMeteringTrigger trigger = static_cast<CommodityMeteringTrigger>(eventTrigger);

    switch (trigger)
    {
    case CommodityMeteringTrigger::kAttributesValueUpdate:
        ChipLogProgress(Support, "[CommodityMetering-Test-Event] => Attributes value update");
        mTestDataManager.Update();
        break;
    case CommodityMeteringTrigger::kAttributesValueUpdateClear:
        ChipLogProgress(Support, "[CommodityMetering-Test-Event] => Attributes value clear");
        mTestDataManager.Clear();
        break;
    default:
        return false;
    }

    return true;
}
