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

#include <app-common/zap-generated/cluster-objects.h>

#include "CommodityPriceMemMgr.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityPrice;

PriceForecastMemMgr::PriceForecastMemMgr() : mPriceEntryIdx(0), mNumPriceComponents(0)
{
    memset(mpListOfPriceEntries, 0, sizeof(mpListOfPriceEntries));
}

PriceForecastMemMgr::~PriceForecastMemMgr()
{
    // Free all memory allocated for the price components
    for (uint16_t idx = 0; idx < kMaxCommodityPriceEntries; idx++)
    {
        if (mpListOfPriceEntries[idx] != nullptr)
        {
            chip::Platform::Delete(mpListOfPriceEntries[idx]);
        }
    }
}

void PriceForecastMemMgr::PreparePriceEntry(uint16_t priceEntryIdx)
{
    // MUST be called for each entry in DataModel::List<const Structs::CommodityPriceStruct::Type> price
    mNumPriceComponents = 0;

    // Should not occur but just to be safe
    if (priceEntryIdx >= kMaxCommodityPriceEntries)
    {
        ChipLogError(AppServer, "PreparePriceEntry bad priceEntryIdx %u", priceEntryIdx);
        return;
    }

    mPriceEntryIdx = priceEntryIdx;

    // Free up any memory associated with this targetSchedule
    if (mpListOfPriceEntries[mPriceEntryIdx] != nullptr)
    {
        chip::Platform::MemoryFree(mpListOfPriceEntries[mPriceEntryIdx]);
        mpListOfPriceEntries[mPriceEntryIdx] = nullptr;
    }
}

void PriceForecastMemMgr::AddPriceComponent(const CommodityPrice::Structs::CommodityPriceComponentStruct::Type & priceComponent)
{
    if (mNumPriceComponents < kMaxComponentsPerPriceEntry)
    {
        mPriceComponents[mNumPriceComponents++] = priceComponent;
    }
    else
    {
        ChipLogError(AppServer, "AddPriceComponent: trying to add too many price components");
    }
}

CommodityPrice::Structs::CommodityPriceComponentStruct::Type * PriceForecastMemMgr::GetPriceComponents() const
{
    return mpListOfPriceEntries[mPriceEntryIdx];
}

uint16_t PriceForecastMemMgr::GetNumPriceComponents() const
{
    return mNumPriceComponents;
}

CHIP_ERROR PriceForecastMemMgr::AllocAndCopy()
{
    // NOTE: PriceForecastMemMgr::PreparePriceEntry() must be called as
    // specified in the class comments in CommodityPriceMemMgr.h before this
    // method can be called.

    VerifyOrDie(mpListOfPriceEntries[mPriceEntryIdx] == nullptr);

    if (mNumPriceComponents > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfPriceEntries[mPriceEntryIdx] = static_cast<CommodityPrice::Structs::CommodityPriceComponentStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(CommodityPrice::Structs::CommodityPriceComponentStruct::Type) * mNumPriceComponents));

        VerifyOrReturnError(mpListOfPriceEntries[mPriceEntryIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        for (uint16_t idx = 0; idx < mNumPriceComponents; idx++)
        {
            // This will cause the CommodityPriceComponent constructor to be called and this element in the array
            new (mpListOfPriceEntries[mPriceEntryIdx] + idx) CommodityPrice::Structs::CommodityPriceComponentStruct::Type();

            // Now copy the price component
            mpListOfPriceEntries[mPriceEntryIdx][idx] = mPriceComponents[idx];
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PriceForecastMemMgr::AllocAndCopy(const DataModel::List<const Structs::CommodityPriceComponentStruct::Type> & priceComponents)
{
    // NOTE: PriceForecastMemMgr::PreparePriceEntry() must be called as
    // specified in the class comments in CommodityPriceMemMgr.h before this
    // method can be called.

    VerifyOrDie(mpListOfPriceEntries[mPriceEntryIdx] == nullptr);

    mNumPriceComponents = static_cast<uint16_t>(priceComponents.size());

    if (mNumPriceComponents > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfPriceEntries[mPriceEntryIdx] = static_cast<CommodityPrice::Structs::CommodityPriceComponentStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(CommodityPrice::Structs::CommodityPriceComponentStruct::Type) * priceComponents.size()));

        VerifyOrReturnError(mpListOfPriceEntries[mPriceEntryIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        uint16_t idx = 0;
        for (auto & component : priceComponents)
        {
            // This will cause the CommodityPriceComponentStruct constructor to be called and this element in the array
            new (mpListOfPriceEntries[mPriceEntryIdx] + idx) CommodityPrice::Structs::CommodityPriceComponentStruct::Type();

            // Now copy the component
            mpListOfPriceEntries[mPriceEntryIdx][idx] = component;

            idx++;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PriceForecastMemMgr::AllocAndCopy(const DataModel::DecodableList<Structs::CommodityPriceComponentStruct::DecodableType> & priceComponents)
{
    // NOTE: PriceForecastMemMgr::PreparePriceEntry() must be called as
    // specified in the class comments in CommodityPriceMemMgr.h before this
    // method can be called.

    VerifyOrDie(mpListOfPriceEntries[mPriceEntryIdx] == nullptr);

    size_t numPriceComponents = 0;
    ReturnErrorOnFailure(priceComponents.ComputeSize(&numPriceComponents));

    mNumPriceComponents = static_cast<uint16_t>(numPriceComponents);

    if (mNumPriceComponents > 0)
    {
        // Allocate the memory first and then use placement new to initialise the memory of each element in the array
        mpListOfPriceEntries[mPriceEntryIdx] = static_cast<CommodityPrice::Structs::CommodityPriceComponentStruct::Type *>(
            chip::Platform::MemoryAlloc(sizeof(CommodityPrice::Structs::CommodityPriceComponentStruct::Type) * mNumPriceComponents));

        VerifyOrReturnError(mpListOfPriceEntries[mPriceEntryIdx] != nullptr, CHIP_ERROR_NO_MEMORY);

        uint16_t idx = 0;
        auto it      = priceComponents.begin();
        while (it.Next())
        {
            // Check that the idx is still valid
            VerifyOrReturnError(idx < mNumPriceComponents, CHIP_ERROR_INCORRECT_STATE);

            auto & component = it.GetValue();

            // This will cause the CommodityPriceComponentStruct constructor to be called and this element in the array
            new (mpListOfPriceEntries[mPriceEntryIdx] + idx) CommodityPrice::Structs::CommodityPriceComponentStruct::Type();

            // Now copy the component
            mpListOfPriceEntries[mPriceEntryIdx][idx] = component;

            idx++;
        }
    }

    return CHIP_NO_ERROR;
}
