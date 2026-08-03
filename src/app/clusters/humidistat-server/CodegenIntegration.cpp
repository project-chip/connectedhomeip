/*
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

#include "CodegenIntegration.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/humidistat-server/HumidistatCluster.h>
#include <app/static-cluster-config/Humidistat.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Humidistat;

namespace {

constexpr size_t kHumidistatFixedClusterCount = Humidistat::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kHumidistatMaxClusterCount   = kHumidistatFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// Per-instance: each cluster instance gets one LazyRegisteredServerCluster slot.
LazyRegisteredServerCluster<HumidistatCluster> gServers[kHumidistatMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        using namespace chip::app::Clusters::Humidistat::Attributes;
        using chip::Protocols::InteractionModel::Status;

        BitFlags<Humidistat::Feature> features(featureMap);

        // Validate feature dependencies from spec constraints.
        VerifyOrDie(!features.Has(Feature::kOptimal) || features.Has(Feature::kSensor));
        VerifyOrDie(!features.HasAny(Feature::kColdMist, Feature::kWarmMist) || features.Has(Feature::kHumidifier));
        VerifyOrDie(!features.Has(Feature::kAuto) ||
                    (features.Has(Feature::kHumidifier) && features.Has(Feature::kDehumidifier) && features.Has(Feature::kSensor)));

        HumidistatCluster::OptionalAttributeSet optionalAttributes(optionalAttributeBits);

        // TargetSetpoint is optional only when SENSOR is present.
        // When OPTIMAL is enabled (which requires SENSOR), TargetSetpoint is effectively mandatory.
        if (!features.Has(Feature::kSensor))
        {
            optionalAttributes.Set<TargetSetpoint::Id>(false);
        }

        HumidistatCluster::StartupConfiguration config;

        if (features.Has(Feature::kSensor))
        {
            VerifyOrDie(MinSetpoint::GetDefault(endpointId, &config.minSetpoint) == Status::Success);
            VerifyOrDie(MaxSetpoint::GetDefault(endpointId, &config.maxSetpoint) == Status::Success);
            VerifyOrDie(Step::GetDefault(endpointId, &config.step) == Status::Success);

            chip::Percent percentVal{};
            VerifyOrDie(UserSetpoint::GetDefault(endpointId, &percentVal) == Status::Success);
            config.userSetpoint = percentVal;

            if (TargetSetpoint::GetDefault(endpointId, &percentVal) == Status::Success)
            {
                config.targetSetpoint = percentVal;
            }
        }

        VerifyOrDie(Mode::GetDefault(endpointId, &config.mode) == Status::Success);
        VerifyOrDie(SystemState::GetDefault(endpointId, &config.systemState) == Status::Success);

        if (features.Has(Feature::kHumidifier))
        {
            VerifyOrDie(MistType::GetDefault(endpointId, &config.mistType) == Status::Success);
            VerifyOrDie((config.mode == ModeEnum::kHumidifier) ? config.mistType.HasAny() : !config.mistType.HasAny());
        }

        if (features.Has(Feature::kContinuous))
        {
            VerifyOrDie(Continuous::GetDefault(endpointId, &config.continuous) == Status::Success);
        }

        if (optionalAttributes.IsSet(Sleep::Id))
        {
            if (Sleep::GetDefault(endpointId, &config.sleep) != Status::Success)
            {
                config.sleep = false;
            }
        }

        if (features.Has(Feature::kOptimal))
        {
            VerifyOrDie(Optimal::GetDefault(endpointId, &config.optimal) == Status::Success);
            chip::Percent percentVal{};
            VerifyOrDie(TargetSetpoint::GetDefault(endpointId, &percentVal) == Status::Success);
            config.targetSetpoint = percentVal;
        }

        gServers[clusterInstanceIndex].Create(endpointId, features, optionalAttributes, config);
        return gServers[clusterInstanceIndex].Registration();
    }

    ServerClusterInterface * FindRegistration(unsigned clusterInstanceIndex) override
    {
        VerifyOrReturnValue(gServers[clusterInstanceIndex].IsConstructed(), nullptr);
        return &gServers[clusterInstanceIndex].Cluster();
    }

    void ReleaseRegistration(unsigned clusterInstanceIndex) override { gServers[clusterInstanceIndex].Destroy(); }
};

} // namespace

void MatterHumidistatClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Humidistat::Id,
            .fixedClusterInstanceCount = kHumidistatFixedClusterCount,
            .maxClusterInstanceCount   = kHumidistatMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterHumidistatClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = Humidistat::Id,
            .fixedClusterInstanceCount = kHumidistatFixedClusterCount,
            .maxClusterInstanceCount   = kHumidistatMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterHumidistatPluginServerInitCallback() {}

namespace chip::app::Clusters::Humidistat {

HumidistatCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = Humidistat::Id,
            .fixedClusterInstanceCount = kHumidistatFixedClusterCount,
            .maxClusterInstanceCount   = kHumidistatMaxClusterCount,
        },
        integrationDelegate);

    return cluster != nullptr ? &static_cast<HumidistatCluster &>(*cluster) : nullptr;
}

} // namespace chip::app::Clusters::Humidistat
