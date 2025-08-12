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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

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

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool FindEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, WiFiNetworkDiagnostics::Id, kWiFiNetworkDiagnosticsFixedClusterCount);

    if (arrayIndex >= kWiFiNetworkDiagnosticsMaxClusterCount)
    {
        ChipLogError(AppServer, "Could not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    outArrayIndex = arrayIndex;

    return true;
}
// Runtime method to check if an attribute is enabled using Ember functions.
bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    return emberAfContainsAttribute(endpointId, WiFiNetworkDiagnostics::Id, attributeId);
}

} // namespace

// This callback is called for any endpoint (fixed or dynamic) that is registered with the Ember machinery.
void emberAfWiFiNetworkDiagnosticsClusterServerInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u, defaulting to 0", endpointId);
        rawFeatureMap = 0;
    }

    // NOTE: Currently, diagnostics only support a single provider (DeviceLayer::GetDiagnosticDataProvider())
    // and do not properly support secondary network interfaces or per-endpoint diagnostics.
    // See issue:#40317
    gServers[arrayIndex].Create(endpointId, DeviceLayer::GetDiagnosticDataProvider(),
                                WiFiDiagnosticsServerLogic::OptionalAttributeSet().Set<CurrentMaxRate::Id>(
                                    IsAttributeEnabled(endpointId, CurrentMaxRate::Id)),
                                BitFlags<WiFiNetworkDiagnostics::Feature>(rawFeatureMap));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register WiFiNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

// This callback is called for any endpoint (fixed or dynamic) that is registered with the Ember machinery.
void MatterWiFiNetworkDiagnosticsClusterServerShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!FindEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister WiFiNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }

    gServers[arrayIndex].Destroy();
}

void MatterWiFiNetworkDiagnosticsPluginServerInitCallback() {}

void MatterWiFiNetworkDiagnosticsPluginServerShutdownCallback() {}
