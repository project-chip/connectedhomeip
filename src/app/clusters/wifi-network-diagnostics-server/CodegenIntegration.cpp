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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-cluster.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-logic.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/static-cluster-config/WiFiNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

static constexpr size_t kWiFiNetworkDiagnosticsFixedClusterCount =
    WiFiNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kWiFiNetworkDiagnosticsMaxClusterCount =
    kWiFiNetworkDiagnosticsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<WiFiDiagnosticsServerCluster> gServers[kWiFiNetworkDiagnosticsMaxClusterCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // NOTE: Currently, diagnostics only support a single provider (DeviceLayer::GetDiagnosticDataProvider())
        // and do not properly support secondary network interfaces or per-endpoint diagnostics.
        // See issue:#40317
        gServers[emberEndpointIndex].Create(endpointId, DeviceLayer::GetDiagnosticDataProvider(),
                                            WiFiDiagnosticsServerLogic::OptionalAttributeSet(optionalAttributeBits),
                                            BitFlags<WiFiNetworkDiagnostics::Feature>(featureMap));
        return gServers[emberEndpointIndex].Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override
    {
        return gServers[emberEndpointIndex].Cluster();
    }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServers[emberEndpointIndex].Destroy(); }
};

} // namespace

// This callback is called for any endpoint (fixed or dynamic) that is registered with the Ember machinery.
void emberAfWiFiNetworkDiagnosticsClusterServerInitCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = WiFiNetworkDiagnostics::Id,
            .fixedClusterServerEndpointCount = kWiFiNetworkDiagnosticsFixedClusterCount,
            .maxEndpointCount                = kWiFiNetworkDiagnosticsMaxClusterCount,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = true,
        },
        integrationDelegate);
}

// This callback is called for any endpoint (fixed or dynamic) that is registered with the Ember machinery.
void MatterWiFiNetworkDiagnosticsClusterServerShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = WiFiNetworkDiagnostics::Id,
            .fixedClusterServerEndpointCount = kWiFiNetworkDiagnosticsFixedClusterCount,
            .maxEndpointCount                = kWiFiNetworkDiagnosticsMaxClusterCount,
        },
        integrationDelegate);
}

void MatterWiFiNetworkDiagnosticsPluginServerInitCallback() {}

void MatterWiFiNetworkDiagnosticsPluginServerShutdownCallback() {}
