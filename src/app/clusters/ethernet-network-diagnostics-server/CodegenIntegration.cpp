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
#include <app/clusters/ethernet-network-diagnostics-server/EthernetDiagnosticsCluster.h>
#include <app/static-cluster-config/EthernetNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;

namespace {

static constexpr size_t kEthernetNetworkDiagnosticsFixedClusterCount =
    EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kEthernetNetworkDiagnosticsMaxClusterCount =
    kEthernetNetworkDiagnosticsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<EthernetDiagnosticsServerCluster> gServers[kEthernetNetworkDiagnosticsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // Create OptionalAttributeSet from optionalAttributeBits
        EthernetDiagnosticsServerCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        // Create the cluster with all required parameters
        gServers[clusterInstanceIndex].Create(DeviceLayer::GetDiagnosticDataProvider(), BitFlags<Feature>(featureMap),
                                              optionalAttributeSet);

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

void MatterEthernetNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = EthernetNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kEthernetNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kEthernetNetworkDiagnosticsMaxClusterCount,
            .fetchFeatureMap           = true,
            .fetchOptionalAttributes   = true,
        },
        integrationDelegate);
}

void MatterEthernetNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                = endpointId,
            .clusterId                 = EthernetNetworkDiagnostics::Id,
            .fixedClusterInstanceCount = kEthernetNetworkDiagnosticsFixedClusterCount,
            .maxClusterInstanceCount   = kEthernetNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate);
}

void MatterEthernetNetworkDiagnosticsPluginServerInitCallback() {}

void MatterEthernetNetworkDiagnosticsPluginServerShutdownCallback() {}
