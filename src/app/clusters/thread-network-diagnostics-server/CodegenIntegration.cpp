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

#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/static-cluster-config/ThreadNetworkDiagnostics.h>
#include <app/util/endpoint-config-api.h>
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
        // We only support minimal and full set of attributes because of flash considerations
        VerifyOrDie(rawFeatureMap == 0 || rawFeatureMap == kFeaturesAll.Raw());

        // NOTE: we do NOT validate what selection ZAP made here and assume that full diagnostics enable
        //       ALL features. This is to save flash and since even if we would validate, all we would do is force
        //       people to select all attributes as enabled in zap.
        //
        //       This also technically means that zap could reduce flash by not including metdata (only featuremap has to be
        //       set and all attributes can be deselected ... it will make not difference here).

        gServers[clusterInstanceIndex].Create(endpointId,
                                              rawFeatureMap == 0 ? ThreadNetworkDiagnosticsCluster::ClusterType::kMinimal
                                                                 : ThreadNetworkDiagnosticsCluster::ClusterType::kFull);
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
            .fetchOptionalAttributes   = false,
        },
        integrationDelegate);
}

void MatterThreadNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = ThreadNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kThreadNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kThreadNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate, shutdownType);
}

void MatterThreadNetworkDiagnosticsPluginServerInitCallback() {}
