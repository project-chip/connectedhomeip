/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/electrical-energy-tariff/impl/SimulatedElectricalEnergyTariff.h>

#include <lib/support/CodeUtils.h>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR SimulatedElectricalEnergyTariff::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mCommodityPriceCluster.Create(endpoint);

    CommodityPrice::CommodityPriceCluster & cluster = mCommodityPriceCluster.Cluster();
    ReturnErrorOnFailure(cluster.SetCurrency(DataModel::MakeNullable(Globals::Structs::CurrencyStruct::Type{
        .currency      = kCurrencyEur,
        .decimalPoints = kCurrencyDecimalPoints,
    })));

    CommodityPrice::Structs::CommodityPriceStruct::Type currentPrice;
    currentPrice.periodStart = 0;
    currentPrice.periodEnd.SetNull();
    currentPrice.price.SetValue(kPricePerKWh);
    ReturnErrorOnFailure(cluster.SetCurrentPrice(DataModel::MakeNullable(currentPrice)));

    return provider.AddCluster(mCommodityPriceCluster.Registration());
}

void SimulatedElectricalEnergyTariff::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mCommodityPriceCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCommodityPriceCluster.Cluster()));
        mCommodityPriceCluster.Destroy();
    }
}

CommodityPrice::CommodityPriceCluster & SimulatedElectricalEnergyTariff::CommodityPriceCluster()
{
    VerifyOrDie(mCommodityPriceCluster.IsConstructed());
    return mCommodityPriceCluster.Cluster();
}

} // namespace chip::app
