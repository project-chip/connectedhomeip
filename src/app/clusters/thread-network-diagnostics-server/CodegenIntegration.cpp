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
        const BitFlags<Feature> featureMap(rawFeatureMap);

        // We only support minimal and full set of attributes because of flash considerations
        VerifyOrDie(featureMap.Raw() == 0 || featureMap.Raw() == kFeaturesAll);

        // For the full set of attributes, every optional attribute needs to be enabled in ember as a precondition
        if (featureMap.Raw() == kFeaturesAll)
        {
            for (uint16_t i = kOptionalAttributesBegin; i <= kOptionalAttributesEnd; ++i)
            {
                auto id = kFullAttributes[i].attributeId;
                VerifyOrDie(emberAfContainsAttribute(endpointId, ThreadNetworkDiagnostics::Id, id));
            }
        }

        using ClusterType       = ThreadNetworkDiagnosticsCluster::ClusterType;
        ClusterType clusterType = (featureMap.Raw() == 0 ? ClusterType::kMinimal : ClusterType::kFull);
        gServers[clusterInstanceIndex].Create(endpointId, clusterType);
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
