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

#include <CommodityMeteringInstance.h>
#include <app/clusters/commodity-metering-server/CommodityMeteringTestEventTriggerHandler.h>
#include <array>
#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityMetering;

namespace MeteredQuantitySamples {
// Define component arrays as constexpr
namespace Sample1 {
static constexpr uint32_t TariffComponents1[] = { 0x5001, 0x5003 };
static constexpr uint32_t TariffComponents2[] = { 0x5002, 0x5003, 0x5004 };

// Non-constexpr storage for the actual data
static const Structs::MeteredQuantityStruct::Type Data[] = {
    { DataModel::List<const uint32_t>(TariffComponents1, sizeof(TariffComponents1) / sizeof(TariffComponents1[0])), 3500 },
    { DataModel::List<const uint32_t>(TariffComponents2, sizeof(TariffComponents2) / sizeof(TariffComponents2[0])), -2000 }
};
} // namespace Sample1

namespace Sample2 {
static constexpr uint32_t TariffComponents1[] = { 0x6001 };
static constexpr uint32_t TariffComponents2[] = { 0x6002, 0x6003 };

static const Structs::MeteredQuantityStruct::Type Data[] = {
    { DataModel::List<const uint32_t>(TariffComponents1, sizeof(TariffComponents1) / sizeof(TariffComponents1[0])), 4200 },
    { DataModel::List<const uint32_t>(TariffComponents2, sizeof(TariffComponents2) / sizeof(TariffComponents2[0])), -1500 }
};
} // namespace Sample2
} // namespace MeteredQuantitySamples

namespace {

class OldCommodityMeteringAttributes
{
private:
    Instance * mInstance = nullptr;

    DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> mMeteredQuantity;
    DataModel::Nullable<uint32_t> mMeteredQuantityTimestamp;
    Globals::TariffUnitEnum mTariffUnit;

    DataModel::List<Structs::MeteredQuantityStruct::Type> GetMeteredQuantityDataSample(uint8_t presetIdx)
    {
        switch (presetIdx)
        {
        case 0:
            return DataModel::List<Structs::MeteredQuantityStruct::Type>(
                const_cast<Structs::MeteredQuantityStruct::Type *>(MeteredQuantitySamples::Sample1::Data),
                sizeof(MeteredQuantitySamples::Sample1::Data) / sizeof(MeteredQuantitySamples::Sample1::Data[0]));
        case 1:
            return DataModel::List<Structs::MeteredQuantityStruct::Type>(
                const_cast<Structs::MeteredQuantityStruct::Type *>(MeteredQuantitySamples::Sample2::Data),
                sizeof(MeteredQuantitySamples::Sample2::Data) / sizeof(MeteredQuantitySamples::Sample2::Data[0]));
        default:
            return DataModel::List<Structs::MeteredQuantityStruct::Type>();
        }
    }

    void SaveMeteredQuantity(const DataModel::Nullable<DataModel::List<Structs::MeteredQuantityStruct::Type>> & newValue)
    {
        // Clear existing data if any
        mMeteredQuantity.SetNull();

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

                    // Allocate memory for tariffComponentIDs
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
        mInstance = GetInstance();
        VerifyOrDieWithMsg(mInstance, AppServer, "CommodityMetering instance is null");
        SaveMeteredQuantity(mInstance->GetMeteredQuantity());
        mMeteredQuantityTimestamp = mInstance->GetMeteredQuantityTimestamp();
        mTariffUnit               = mInstance->GetTariffUnit();
    }

    void ClearAttributes()
    {
        ClearMeteredQuantity();
        mMeteredQuantityTimestamp.SetNull();
    }

    void RestoreAttributes() const
    {
        if (mInstance)
        {
            mInstance->SetMeteredQuantity(mMeteredQuantity);
            mInstance->SetMeteredQuantityTimestamp(mMeteredQuantityTimestamp);
            mInstance->SetTariffUnit(mTariffUnit);
        }
    }

    void UpdAttrs()
    {
        uint32_t matterEpoch = 0;

        CHIP_ERROR err = System::Clock::GetClock_MatterEpochS(matterEpoch);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "UpdAttrs() could not get time");
        }

        mMeteredQuantityTimestamp.SetNonNull(matterEpoch);

        if (mTariffUnit == Globals::TariffUnitEnum::kKWh)
        {
            mTariffUnit = Globals::TariffUnitEnum::kKVAh;
        }
        else
        {
            mTariffUnit = Globals::TariffUnitEnum::kKWh;
        }

        auto MQListSample = GetMeteredQuantityDataSample(static_cast<uint8_t>(mTariffUnit == Globals::TariffUnitEnum::kKWh));

        mInstance->SetMeteredQuantity(MakeNullable(MQListSample));
        mInstance->SetMeteredQuantityTimestamp(mMeteredQuantityTimestamp);
        mInstance->SetTariffUnit(mTariffUnit);
    }

public:
    OldCommodityMeteringAttributes(){};
    ~OldCommodityMeteringAttributes() = default;

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

OldCommodityMeteringAttributes mOldCommodityMeteringAttributes;
} // namespace

bool HandleCommodityMeteringTestEventTrigger(uint64_t eventTrigger)
{
    CommodityMeteringTrigger trigger = static_cast<CommodityMeteringTrigger>(eventTrigger);

    switch (trigger)
    {
    case CommodityMeteringTrigger::kAttributesValueUpdate:
        ChipLogProgress(Support, "[CommodityMetering-Test-Event] => Attributes value update");
        mOldCommodityMeteringAttributes.Update();
        break;
    case CommodityMeteringTrigger::kAttributesValueUpdateClear:
        ChipLogProgress(Support, "[CommodityMetering-Test-Event] => Attributes value clear");
        mOldCommodityMeteringAttributes.Clear();
        break;
    default:
        return false;
    }

    return true;
}
