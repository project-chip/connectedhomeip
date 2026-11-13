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

#pragma once

#include <app/clusters/commodity-price-server/CommodityPriceCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/types/electrical-energy-tariff/ElectricalEnergyTariff.h>

namespace chip::app {

/**
 * Active Electrical Energy Tariff that exposes a Commodity Price cluster with a fixed price.
 *
 * Commodity Price is only allowed on the active tariff, so this device reports IsActiveTariff() and
 * its default TagList is ActiveTariffTagList().
 *
 * The Commodity Price cluster uses no features and no optional commands. On registration the
 * cluster is set to a currency of EUR with 2 decimal points and a current price of 25 (0.25 EUR per
 * kWh) that has no scheduled end.
 */
class SimulatedElectricalEnergyTariff : public ElectricalEnergyTariff
{
public:
    /// ISO 4217 numeric code for EUR.
    static constexpr uint16_t kCurrencyEur          = 978;
    static constexpr uint8_t kCurrencyDecimalPoints = 2;
    /// Price per kWh in units of 10^-kCurrencyDecimalPoints of the currency.
    static constexpr int64_t kPricePerKWh = 25;

    explicit SimulatedElectricalEnergyTariff(Span<const EndpointComposition::SemanticTag> tagList = ActiveTariffTagList()) :
        ElectricalEnergyTariff(tagList)
    {}
    ~SimulatedElectricalEnergyTariff() override = default;

    Clusters::CommodityPrice::CommodityPriceCluster & CommodityPriceCluster();

protected:
    bool IsActiveTariff() const override { return true; }
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    LazyRegisteredServerCluster<Clusters::CommodityPrice::CommodityPriceCluster> mCommodityPriceCluster;
};

} // namespace chip::app
