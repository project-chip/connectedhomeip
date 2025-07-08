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
#include "wifi-network-diagnostics-cluster.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/static-cluster-config/WiFiNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics;
using namespace chip::app::Clusters::WiFiNetworkDiagnostics::Attributes;
using chip::Protocols::InteractionModel::Status;

// for fixed endpoint, this file is ever only included IF WiFi network diagnostics is enabled and that MUST happen only on endpoint
// 0 the static assert is skipped in case of dynamic endpoints.
static_assert((WiFiNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               WiFiNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              WiFiNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<WiFiDiagnosticsServer> gServer;

// compile-time evaluated method if "is <EP>::WiFiNetworkDiagnostics::<ATTR>" enabled
constexpr bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    for (auto & config : WiFiNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig)
    {
        if (config.endpointNumber != endpointId)
        {
            continue;
        }
        for (auto & attr : config.enabledAttributes)
        {
            if (attr == attributeId)
            {
                return true;
            }
        }
    }
    return false;
}

} // namespace

void emberAfWiFiNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    const WiFiNetworkDiagnosticsEnabledAttributes enabledAttributes{
        .enableBeaconLostCount        = IsAttributeEnabled(kRootEndpointId, Attributes::BeaconLostCount::Id),
        .enableBeaconRxCount          = IsAttributeEnabled(kRootEndpointId, Attributes::BeaconRxCount::Id),
        .enablePacketMulticastRxCount = IsAttributeEnabled(kRootEndpointId, Attributes::PacketMulticastRxCount::Id),
        .enablePacketMulticastTxCount = IsAttributeEnabled(kRootEndpointId, Attributes::PacketMulticastTxCount::Id),
        .enablePacketUnicastRxCount   = IsAttributeEnabled(kRootEndpointId, Attributes::PacketUnicastRxCount::Id),
        .enablePacketUnicastTxCount   = IsAttributeEnabled(kRootEndpointId, Attributes::PacketUnicastTxCount::Id),
        .enableCurrentMaxRate         = IsAttributeEnabled(kRootEndpointId, Attributes::CurrentMaxRate::Id),
        .enableOverrunCount           = IsAttributeEnabled(kRootEndpointId, Attributes::OverrunCount::Id),
    };

    uint32_t rawFeatureMap;
    if (FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
    {
        ChipLogError(AppServer, "Failed to get feature map for endpoint %u", endpointId);
        rawFeatureMap = 0;
    }
    DeviceLayer::GetDiagnosticDataProvider().SetWiFiDiagnosticsDelegate(&gServer.Cluster().GetLogic());
    gServer.Create(kRootEndpointId, DeviceLayer::GetDiagnosticDataProvider(), enabledAttributes,
                   BitFlags<WiFiNetworkDiagnostics::Feature>(rawFeatureMap));

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register WiFiNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfWiFiNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    DeviceLayer::GetDiagnosticDataProvider().SetWiFiDiagnosticsDelegate(nullptr);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister WiFiNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterWiFiNetworkDiagnosticsPluginServerInitCallback() {}

void MatterWiFiNetworkDiagnosticsPluginServerShutdownCallback() {}
