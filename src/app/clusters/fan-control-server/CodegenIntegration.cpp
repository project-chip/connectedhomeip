/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include <app-common/zap-generated/callback.h>
#include <app/clusters/fan-control-server/CodegenIntegration.h>
#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/clusters/fan-control-server/fan-control-delegate.h>
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/FanControl.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kFanControlFixedClusterCount = FanControl::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kFanControlMaxClusterCount   = kFanControlFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

constexpr size_t kFanControlDelegateTableSize =
    MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kFanControlDelegateTableSize <= kEmberInvalidEndpointIndex, "FanControl Delegate table size error");

FanControl::Delegate * gDelegateTable[kFanControlDelegateTableSize] = { nullptr };

LazyRegisteredServerCluster<FanControlCluster> gServers[kFanControlMaxClusterCount];
FanControl::DefaultDelegate gDefaultDelegate(kRootEndpointId);

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        BitFlags<FanControl::Feature> features(featureMap);

        FanControl::Delegate * delegate = FanControl::GetDelegate(endpointId);
        if (delegate == nullptr)
            delegate = &gDefaultDelegate;
        FanControlCluster::Config config(endpointId, *delegate);

        config.WithFanModeSequence(delegate->GetFanModeSequence().value_or(FanModeSequenceEnum::kOffLowHigh));

        if (features.Has(FanControl::Feature::kMultiSpeed))
            config.WithSpeedMax(delegate->GetSpeedMax().value_or(1));
        if (features.Has(FanControl::Feature::kRocking))
            config.WithRockSupport(delegate->GetRockSupport().value_or(
                BitMask<RockBitmap>(RockBitmap::kRockLeftRight, RockBitmap::kRockUpDown, RockBitmap::kRockRound)));
        if (features.Has(FanControl::Feature::kWind))
            config.WithWindSupport(
                delegate->GetWindSupport().value_or(BitMask<WindBitmap>(WindBitmap::kSleepWind, WindBitmap::kNaturalWind)));
        if (features.Has(FanControl::Feature::kAirflowDirection))
            config.WithAirflowDirection();
        if (features.Has(FanControl::Feature::kStep))
            config.WithStep();

        gServers[clusterInstanceIndex].Create(config);
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

void MatterFanControlClusterInitCallback(EndpointId endpointId)
{
    // Allow app to set delegate before we create the cluster (e.g. via emberAfFanControlClusterInitCallback)
    emberAfFanControlClusterInitCallback(endpointId);

    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterFanControlClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

namespace chip::app::Clusters::FanControl {

FanControlCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * cluster = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = FanControl::Id,
            .fixedClusterInstanceCount = kFanControlFixedClusterCount,
            .maxClusterInstanceCount   = kFanControlMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<FanControlCluster *>(cluster);
}

Delegate * GetDelegate(EndpointId aEndpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kFanControlDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kFanControlDelegateTableSize)
    {
        gDelegateTable[ep] = aDelegate;
    }
}

} // namespace chip::app::Clusters::FanControl
