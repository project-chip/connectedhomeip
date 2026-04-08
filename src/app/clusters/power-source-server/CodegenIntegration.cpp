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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>
#include <app/static-cluster-config/PowerSource.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <include/platform/CHIPDeviceLayer.h>

#include <variant>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;
using namespace chip::app::Clusters::PowerSource::Attributes;
using namespace chip::app::Clusters::PowerSource::StaticApplicationConfig;

namespace {

constexpr size_t kPowerSourceFixedClusterCount = kFixedClusterConfig.size();

// invariant: endpointId is null IFF configOrCluster is std::monostate
struct ConfigOrCluster
{
    TrivialConstexprOptional<EndpointId> endpointId;
    std::variant<std::monostate, PowerSourceCluster::WiredConfiguration, PowerSourceCluster::BatteryConfiguration, RegisteredServerCluster<PowerSourceCluster>> configOrCluster;
};

template <size_t index>
constexpr auto GetClusterConfigFromIndex()
{
    if constexpr (index >= kPowerSourceFixedClusterCount)
    {
        // Dynamic endpoint, no config at compile time
        return std::monostate{};
    }
    else
    {
        // Static endpoint, get config from StaticClusterConfig
        constexpr auto endpointConfig = kFixedClusterConfig[index];
        constexpr auto defaults = GetAttributeDefaultValuesOnEndpoint<endpointConfig.endpointNumber>();

        static_assert(endpointConfig.featureMap.Has(Feature::kWired) ^ endpointConfig.featureMap.Has(Feature::kBattery),
                    "PowerSource cluster must have exactly one of Wired and Battery features.");
        if constexpr (endpointConfig.featureMap.Has(Feature::kWired))
        {
            CharSpan description{};
            WiredCurrentTypeEnum currentType{};

            if constexpr (defaults.HasDefaultValue(Description::Id))
            {
                description = defaults.description;
            }
            static_assert(defaults.HasDefaultValue(WiredCurrentType::Id), "WiredCurrentType is required to have a default value when the Wired feature is enabled.");
            currentType = defaults.wiredCurrentType;

            PowerSourceCluster::WiredConfiguration config(description, currentType);
            if constexpr (defaults.HasDefaultValue(Status::Id))
            {
                config.status = defaults.status;
            }
            if constexpr (defaults.HasDefaultValue(Order::Id))
            {
                config.order = defaults.order;
            }
            if constexpr (defaults.HasDefaultValue(WiredNominalVoltage::Id))
            {
                config.nominalVoltage = defaults.wiredNominalVoltage;
            }
            if constexpr (defaults.HasDefaultValue(WiredMaximumCurrent::Id))
            {
                config.maximumCurrent = defaults.wiredMaximumCurrent;
            }
            if constexpr (defaults.HasDefaultValue(WiredAssessedInputVoltage::Id))
            {
                config.assessedInputVoltage = defaults.wiredAssessedInputVoltage;
            }
            if constexpr (defaults.HasDefaultValue(WiredAssessedInputFrequency::Id))
            {
                config.assessedInputFrequency = defaults.wiredAssessedInputFrequency;
            }
            if constexpr (defaults.HasDefaultValue(WiredAssessedCurrent::Id))
            {
                config.assessedCurrent = defaults.wiredAssessedCurrent;
            }
            if constexpr (defaults.HasDefaultValue(WiredPresent::Id))
            {
                config.isPresent = defaults.wiredIsPresent;
            }
            return config;
        }
        else
        {
            CharSpan description{};
            BatReplaceabilityEnum replaceability = BatReplaceabilityEnum::kUnspecified;

            if constexpr (defaults.HasDefaultValue(Description::Id))
            {
                description = defaults.description;
            }
            if constexpr (defaults.HasDefaultValue(BatReplaceability::Id))
            {
                replaceability = defaults.batReplaceability;
            }

            PowerSourceCluster::BatteryConfiguration config(description, replaceability);
            if constexpr (defaults.HasDefaultValue(Status::Id))
            {
                config.status = defaults.status;
            }
            if constexpr (defaults.HasDefaultValue(Order::Id))
            {
                config.order = defaults.order;
            }

            if constexpr (defaults.HasDefaultValue(BatVoltage::Id))
            {
                config.voltage = defaults.batVoltage;
            }
            if constexpr (defaults.HasDefaultValue(BatPercentRemaining::Id))
            {
                config.percentRemaining = defaults.batPercentRemaining;
            }
            if constexpr (defaults.HasDefaultValue(BatTimeRemaining::Id))
            {
                config.timeRemaining = defaults.batTimeRemaining;
            }
            if constexpr (defaults.HasDefaultValue(BatChargeLevel::Id))
            {
                config.chargeLevel = defaults.batChargeLevel;
            }
            if constexpr (defaults.HasDefaultValue(BatReplacementNeeded::Id))
            {
                config.replacementNeeded = defaults.batReplacementNeeded;
            }
            if constexpr (defaults.HasDefaultValue(BatPresent::Id))
            {
                config.isPresent = defaults.batPresent;
            }

            if constexpr (endpointConfig.featureMap.Has(Feature::kReplaceable))
            {
                CharSpan replacementDescription{};
                if constexpr (defaults.HasDefaultValue(BatReplacementDescription::Id))
                {
                    replacementDescription = defaults.batReplacementDescription;
                }
                uint8_t quantity = 0;
                if constexpr (defaults.HasDefaultValue(BatQuantity::Id))
                {
                    quantity = defaults.batQuantity;
                }

                config.MakeReplaceable(replacementDescription, quantity);

                if constexpr (defaults.HasDefaultValue(BatCommonDesignation::Id))
                {
                    config.commonDesignation = defaults.batCommonDesignation;
                }
                if constexpr (defaults.HasDefaultValue(BatANSIDesignation::Id))
                {
                    config.ansiDesignation = defaults.batANSIDesignation;
                }
                if constexpr (defaults.HasDefaultValue(BatIECDesignation::Id))
                {
                    config.iecDesignation = defaults.batIECDesignation;
                }
                if constexpr (defaults.HasDefaultValue(BatApprovedChemistry::Id))
                {
                    config.approvedChemistry = defaults.batApprovedChemistry;
                }
            }

            if constexpr (endpointConfig.featureMap.Has(Feature::kRechargeable))
            {
                config.MakeRechargeable();

                if constexpr (defaults.HasDefaultValue(BatChargeState::Id))
                {
                    config.chargeState = defaults.batChargeState;
                }
                if constexpr (defaults.HasDefaultValue(BatTimeToFullCharge::Id))
                {
                    config.timeToFullCharge = defaults.batTimeToFullCharge;
                }
                if constexpr (defaults.HasDefaultValue(BatFunctionalWhileCharging::Id))
                {
                    config.functionalWhileCharging = defaults.batFunctionalWhileCharging;
                }
                if constexpr (defaults.HasDefaultValue(BatChargingCurrent::Id))
                {
                    config.chargingCurrent = defaults.batChargingCurrent;
                }
            }

            if constexpr (endpointConfig.featureMap.HasAny(Feature::kReplaceable, Feature::kRechargeable))
            {
                if constexpr (defaults.HasDefaultValue(BatCapacity::Id))
                {
                    config.capacity = defaults.batCapacity;
                }
            }
            return config;
        }
    }
}

// Force compile time evaluation of GetClusterConfigFromIndex
// This will make all the generated objects in the StaticClusterConfig to be used only at compile time, and thus can be optimized away.
template <size_t I>
constexpr auto kConfigForIndex = GetClusterConfigFromIndex<I>();
template <size_t I>
constexpr TrivialConstexprOptional<EndpointId> kEndpointIdForIndex =
    (I < kPowerSourceFixedClusterCount) ?
    TrivialConstexprOptional<EndpointId>{kFixedClusterConfig[I].endpointNumber} :
    TrivialConstexprOptional<EndpointId>{};

// Helper
template <std::size_t... Is>
auto GetClusterConfigArrayImpl(std::index_sequence<Is...>) {
    // If it is for a static endpoint, create the config using information from StaticClusterConfig.
    // For a dynamic endpoint, create empty std::monostate variant which can be replaced with a config later
    return std::array{ ConfigOrCluster{kEndpointIdForIndex<Is>, kConfigForIndex<Is>}... };
}

template <std::size_t N>
std::array<ConfigOrCluster, N> GetClusterConfigArray() {
    return GetClusterConfigArrayImpl(std::make_index_sequence<N>{});
}

constexpr size_t kPowerSourceMaxClusterCount   = kPowerSourceFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
auto gConfigsOrClusters = GetClusterConfigArray<kPowerSourceMaxClusterCount>();


bool FindIndexForEndpoint(EndpointId endpointId, size_t & index)
{
    for (size_t i = 0; i < gConfigsOrClusters.size(); ++i)
    {
        if (gConfigsOrClusters[i].endpointId.hasValue && gConfigsOrClusters[i].endpointId.value == endpointId)
        {
            index = i;
            return true;
        }
    }
    return false;
}

} // namespace

void MatterPowerSourceClusterInitCallback(EndpointId endpointId)
{
    size_t index{};
    VerifyOrDieWithMsg(FindIndexForEndpoint(endpointId, index), Zcl, "Invalid endpoint for power source cluster: %u", endpointId);
    VerifyOrDieWithMsg(!std::holds_alternative<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster) &&
                       !std::holds_alternative<std::monostate>(gConfigsOrClusters[index].configOrCluster),
                        Zcl, "PowerSource cluster for endpoint %u is already initialized", endpointId);

    if (std::holds_alternative<PowerSourceCluster::WiredConfiguration>(gConfigsOrClusters[index].configOrCluster))
    {
        auto config = std::get<PowerSourceCluster::WiredConfiguration>(gConfigsOrClusters[index].configOrCluster);
        gConfigsOrClusters[index].configOrCluster.emplace<RegisteredServerCluster<PowerSourceCluster>>(endpointId, DeviceLayer::SystemLayer(), config);
    }
    else if (std::holds_alternative<PowerSourceCluster::BatteryConfiguration>(gConfigsOrClusters[index].configOrCluster))
    {
        auto config = std::get<PowerSourceCluster::BatteryConfiguration>(gConfigsOrClusters[index].configOrCluster);
        gConfigsOrClusters[index].configOrCluster.emplace<RegisteredServerCluster<PowerSourceCluster>>(endpointId, DeviceLayer::SystemLayer(), config);
    }

    SuccessOrDie(CodegenDataModelProvider::Instance().Registry().Register(
        std::get<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster).Registration()));
}

void MatterPowerSourceClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    size_t index{};
    VerifyOrDieWithMsg(FindIndexForEndpoint(endpointId, index), Zcl, "Invalid endpoint for power source cluster: %u", endpointId);

    if (!std::holds_alternative<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster))
    {
        // cluster was never created, nothing to do
        return;
    }

    auto & cluster = std::get<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster);
    SuccessOrDie(CodegenDataModelProvider::Instance().Registry().Unregister(&cluster.Cluster()));

    gConfigsOrClusters[index].configOrCluster.emplace<std::monostate>(); // reset to default state
    gConfigsOrClusters[index].endpointId.reset();
}

void MatterPowerSourcePluginServerInitCallback() {}

namespace chip::app::Clusters::PowerSource {

PowerSourceCluster * CreateClusterOnEndpoint(EndpointId id, const PowerSourceCluster::WiredConfiguration & config)
{
    for (auto & configOrCluster : gConfigsOrClusters)
    {
        if (!configOrCluster.endpointId.hasValue)
        {
            configOrCluster.endpointId = id;
            configOrCluster.configOrCluster.emplace<RegisteredServerCluster<PowerSourceCluster>>(id, DeviceLayer::SystemLayer(), config);
            VerifyOrReturnValue(CodegenDataModelProvider::Instance().Registry().Register(
                std::get<RegisteredServerCluster<PowerSourceCluster>>(configOrCluster.configOrCluster).Registration()) == CHIP_NO_ERROR, nullptr);
            return &std::get<RegisteredServerCluster<PowerSourceCluster>>(configOrCluster.configOrCluster).Cluster();
        }
    }
    ChipLogError(Zcl, "Dynamic endpoint count reached maximum for power source cluster");
    return nullptr;
}

PowerSourceCluster * CreateClusterOnEndpoint(EndpointId id, const PowerSourceCluster::BatteryConfiguration & config)
{
    for (auto & configOrCluster : gConfigsOrClusters)
    {
        if (!configOrCluster.endpointId.hasValue)
        {
            configOrCluster.endpointId = id;
            configOrCluster.configOrCluster.emplace<RegisteredServerCluster<PowerSourceCluster>>(id, DeviceLayer::SystemLayer(), config);
            VerifyOrReturnValue(CodegenDataModelProvider::Instance().Registry().Register(
                std::get<RegisteredServerCluster<PowerSourceCluster>>(configOrCluster.configOrCluster).Registration()) == CHIP_NO_ERROR, nullptr);
            return &std::get<RegisteredServerCluster<PowerSourceCluster>>(configOrCluster.configOrCluster).Cluster();
        }
    }
    ChipLogError(Zcl, "Dynamic endpoint count reached maximum for power source cluster");
    return nullptr;
}

PowerSourceCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    size_t index{};
    VerifyOrReturnValue(FindIndexForEndpoint(endpointId, index), nullptr);

    if (!std::holds_alternative<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster))
    {
        // cluster was never created, return null
        return nullptr;
    }

    auto & cluster = std::get<RegisteredServerCluster<PowerSourceCluster>>(gConfigsOrClusters[index].configOrCluster);
    return static_cast<PowerSourceCluster *>(&cluster.Cluster());
}

} // namespace chip::app::Clusters::PowerSource
