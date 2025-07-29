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

    /* This class is a placeholder - most of the functionality is in the
     * cluster server. Consider removing this.
     */
private:
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
