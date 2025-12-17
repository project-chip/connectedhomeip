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

#include "CodegenIntegration.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/static-cluster-config/ThreadNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr size_t kThreadNetworkDiagnosticsFixedClusterCount =
    ThreadNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kThreadNetworkDiagnosticsMaxClusterCount =
    kThreadNetworkDiagnosticsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ThreadNetworkDiagnosticsCluster> gServers[kThreadNetworkDiagnosticsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t rawFeatureMap) override
    {
        const BitFlags<Feature> featureMap(rawFeatureMap);
        ThreadNetworkDiagnosticsCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        ActiveTimestamp::TypeInfo::Type activeTs;
        if (optionalAttributeSet.IsSet(ActiveTimestamp::Id))
        {
            VerifyOrDie(ActiveTimestamp::Get(endpointId, activeTs) == Status::Success);
        }

        PendingTimestamp::TypeInfo::Type pendingTs;
        if (optionalAttributeSet.IsSet(PendingTimestamp::Id))
        {
            VerifyOrDie(PendingTimestamp::Get(endpointId, pendingTs) == Status::Success);
        }

        Delay::TypeInfo::Type delay;
        if (optionalAttributeSet.IsSet(Delay::Id))
        {
            VerifyOrDie(Delay::Get(endpointId, delay) == Status::Success);
        }

        gServers[clusterInstanceIndex].Create(
            endpointId, featureMap, optionalAttributeSet,
            ThreadNetworkDiagnosticsCluster::StartupConfiguration{ .activeTs = activeTs, .pendingTs = pendingTs, .delay = delay });
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

void MatterThreadNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterThreadNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate);
}

namespace chip::app::Clusters::ThreadNetworkDiagnostics {

ThreadNetworkDiagnosticsCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    ServerClusterInterface * threadNetworkDiagnostics = CodegenClusterIntegration::FindClusterOnEndpoint(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate);

    return static_cast<ThreadNetworkDiagnosticsCluster *>(threadNetworkDiagnostics);
}

} // namespace chip::app::Clusters::ThreadNetworkDiagnostics

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}
