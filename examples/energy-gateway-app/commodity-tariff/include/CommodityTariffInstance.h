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

#include <json/json.h>

#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

using chip::Protocols::InteractionModel::Status;

class CommodityTariffDelegate : public CommodityTariff::Delegate
{
public:
    CommodityTariffDelegate();
    ~CommodityTariffDelegate() = default;

    Status GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId,
                                                        Structs::DayEntryStruct::Type & aDayEntry) override;

    Status GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                                   DataModel::Nullable<chip::CharSpan> & label,
                                                                   DataModel::List<const uint32_t> & dayEntryIDs,
                                                                   Structs::TariffComponentStruct::Type & aTariffComponent) override;
    CHIP_ERROR LoadTariffData(const Json::Value & value);

    bool TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx) override;
};

class CommodityTariffInstance : public Instance
{
public:
    CommodityTariffInstance(EndpointId aEndpointId, CommodityTariffDelegate & aDelegate, Feature aFeature) :
        CommodityTariff::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    CommodityTariffInstance(const CommodityTariffInstance &)             = delete;
    CommodityTariffInstance(const CommodityTariffInstance &&)            = delete;
    CommodityTariffInstance & operator=(const CommodityTariffInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();
    CHIP_ERROR AppInit();

    CommodityTariffDelegate * GetDelegate() { return mDelegate; };

private:
    CommodityTariffDelegate * mDelegate;
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
 