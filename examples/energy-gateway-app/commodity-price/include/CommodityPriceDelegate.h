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
#pragma once

#include "CommodityPriceMemMgr.h"
#include <app/clusters/commodity-price-server/commodity-price-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>
namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

using chip::Protocols::InteractionModel::Status;

class CommodityPriceDelegate : public CommodityPrice::Delegate
{
public:
    CommodityPriceDelegate();
    ~CommodityPriceDelegate() = default;

    // Attribute Accessors

    /* These functions are called by the ReadAttribute handler to iterate through lists
     * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
     * lock on the data.
     * The delegate is expected to not change these values once Start<Type>Read has been called
     * until the End<Type>Read() has been called (e.g. releasing a lock on the data)
     */
    // CHIP_ERROR StartPriceForecastRead(CommodityPriceDetailBitmap bitmap) override;
    // CHIP_ERROR GetPriceForecastByIndex(uint8_t, Structs::CommodityPriceStruct::Type &) override;
    // CHIP_ERROR EndPriceForecastRead() override;

    // TODO work out how to set forecast data and retrieve it

    const PriceForecastMemMgr & GetPriceForecastMemMgr() { return mPriceForecastMemMgr; }

private:
    // This storage is only needed if the FORE (forecast) feature is supported
    // ------------
    // Object to handle the allocation of memory for the price components list
    PriceForecastMemMgr mPriceForecastMemMgr;

    // actual storage of price entries as an array
    Structs::CommodityPriceStruct::Type mPriceEntryArray[kMaxForecastEntries];
};

class CommodityPriceInstance : public Instance
{
public:
    CommodityPriceInstance(EndpointId aEndpointId, CommodityPriceDelegate & aDelegate, Feature aFeature) :
        CommodityPrice::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    CommodityPriceInstance(const CommodityPriceInstance &)             = delete;
    CommodityPriceInstance(const CommodityPriceInstance &&)            = delete;
    CommodityPriceInstance & operator=(const CommodityPriceInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();
    CHIP_ERROR AppInit();

    CommodityPriceDelegate * GetDelegate() { return mDelegate; };

private:
    CommodityPriceDelegate * mDelegate;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
