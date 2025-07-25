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
#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-cluster.h>
#include <app/static-cluster-config/EthernetNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics;

namespace {

static constexpr size_t kEthernetNetworkDiagnosticsFixedClusterCount =
    EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kEthernetNetworkDiagnosticsMaxClusterCount =
    kEthernetNetworkDiagnosticsFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<EthernetDiagnosticsServerCluster> gServers[kEthernetNetworkDiagnosticsMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool findEndpointWithLog(EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, EthernetNetworkDiagnostics::Id,
                                                               kEthernetNetworkDiagnosticsFixedClusterCount);

    if (arrayIndex >= kEthernetNetworkDiagnosticsMaxClusterCount)
    {
        ChipLogError(AppServer, "Could not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    outArrayIndex = arrayIndex;
    return true;
}

// Runtime method to check if an attribute is enabled using Ember functions
bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    return emberAfContainsAttribute(endpointId, EthernetNetworkDiagnostics::Id, attributeId);
}

} // namespace

void emberAfEthernetNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    const EthernetDiagnosticsEnabledAttributes enabledAttributes{
        .enableCarrierDetect = IsAttributeEnabled(endpointId, Attributes::CarrierDetect::Id),
        .enableFullDuplex    = IsAttributeEnabled(endpointId, Attributes::FullDuplex::Id),
        .enablePacketCount   = IsAttributeEnabled(endpointId, Attributes::PacketRxCount::Id) ||
            IsAttributeEnabled(endpointId, Attributes::PacketTxCount::Id),
        .enablePHYRate        = IsAttributeEnabled(endpointId, Attributes::PHYRate::Id),
        .enableTimeSinceReset = IsAttributeEnabled(endpointId, Attributes::TimeSinceReset::Id),
        .enableErrCount       = IsAttributeEnabled(endpointId, Attributes::TxErrCount::Id) ||
            IsAttributeEnabled(endpointId, Attributes::CollisionCount::Id) ||
            IsAttributeEnabled(endpointId, Attributes::OverrunCount::Id),
    };

    // NOTE: Currently, diagnostics only support a single provider (DeviceLayer::GetDiagnosticDataProvider())
    // and do not properly support secondary network interfaces or per-endpoint diagnostics.
    // See issue:#40175
    gServers[arrayIndex].Create(DeviceLayer::GetDiagnosticDataProvider(), enabledAttributes);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register EthernetNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfEthernetNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister EthernetNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServers[arrayIndex].Destroy();
}

void MatterEthernetNetworkDiagnosticsPluginServerInitCallback() {}

void MatterEthernetNetworkDiagnosticsPluginServerShutdownCallback() {}
