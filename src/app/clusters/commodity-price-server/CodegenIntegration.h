/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/commodity-price-server/CommodityPriceCluster.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/CommodityPrice/Enums.h>
#include <clusters/CommodityPrice/Structs.h>
#include <clusters/shared/Enums.h>
#include <clusters/shared/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

// From ISO 4217 (non exhaustive selection)
inline constexpr uint16_t kCurrencyCHF  = 756;
inline constexpr uint16_t kCurrencyEURO = 978;
inline constexpr uint16_t kCurrencyGBP  = 826;
inline constexpr uint16_t kCurrencyNOK  = 578;
inline constexpr uint16_t kCurrencySEK  = 752;
inline constexpr uint16_t kCurrencyUSD  = 840;

/// Place holder that applications derive from. CommodityPriceCluster delegates nothing, so this
/// carries no delegated methods and exists only for backwards compatibility.
class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

protected:
    EndpointId mEndpointId = 0;
};

/// Owns a CommodityPriceCluster and registers it with the codegen data model provider.
///
/// This preserves the API that applications used before the cluster became code driven. New
/// applications should instantiate CommodityPriceCluster directly, which additionally lets them
/// choose which of the two optionally conformant commands to support.
class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature);

    ~Instance() { Shutdown(); }

    Instance(const Instance &)             = delete;
    Instance & operator=(const Instance &) = delete;

    CHIP_ERROR Init();

    /// Unregisters the cluster. Doing this more than once is a no-op.
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    // Set attribute methods
    CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum);
    CHIP_ERROR SetCurrency(Globals::Structs::CurrencyStruct::Type);
    CHIP_ERROR SetCurrentPrice(const DataModel::Nullable<Structs::CommodityPriceStruct::Type>);
    CHIP_ERROR SetForecast(const DataModel::List<const Structs::CommodityPriceStruct::Type> &);

    // Send Price Change events
    Protocols::InteractionModel::Status GeneratePriceChangeEvent();

private:
    RegisteredServerCluster<CommodityPriceCluster> mCluster;
    bool mRegistered = false;
};

} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
