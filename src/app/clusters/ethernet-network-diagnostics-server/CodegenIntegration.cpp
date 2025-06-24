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
#include <app/clusters/ethernet-network-diagnostics-server/ethernet-diagnostics-logic.h>
#include <app/static-cluster-config/EthernetNetworkDiagnostics.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics;

// for fixed endpoint, this file is ever only included IF ethernet diagnostics is enabled and that MUST happen only on endpoint 0
// the static assert is skipped in case of dynamic endpoints.
static_assert((EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 1 &&
               EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig[0].endpointNumber == kRootEndpointId) ||
              EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig.size() == 0);

namespace {

LazyRegisteredServerCluster<EthernetDiagnosticsServerCluster<DeviceLayerEthernetDiagnosticsLogic>> gServer;

// compile-time evaluated method if "is <EP>::EthernetNetworkDiagnostics::<ATTR>" enabled
constexpr bool IsAttributeEnabled(EndpointId endpointId, AttributeId attributeId)
{
    for (auto & config : EthernetNetworkDiagnostics::StaticApplicationConfig::kFixedClusterConfig)
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

void emberAfEthernetNetworkDiagnosticsClusterInitCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);

    const EthernetDiagnosticsEnabledAttributes enabledAttributes{
        .enableCarrierDetect  = IsAttributeEnabled(kRootEndpointId, Attributes::CarrierDetect::Id),
        .enableCollisionCount = IsAttributeEnabled(kRootEndpointId, Attributes::CollisionCount::Id),
        .enableFullDuplex     = IsAttributeEnabled(kRootEndpointId, Attributes::FullDuplex::Id),
        .enableOverrunCount   = IsAttributeEnabled(kRootEndpointId, Attributes::OverrunCount::Id),
        .enablePacketRxCount  = IsAttributeEnabled(kRootEndpointId, Attributes::PacketRxCount::Id),
        .enablePacketTxCount  = IsAttributeEnabled(kRootEndpointId, Attributes::PacketTxCount::Id),
        .enablePHYRate        = IsAttributeEnabled(kRootEndpointId, Attributes::PHYRate::Id),
        .enableTimeSinceReset = IsAttributeEnabled(kRootEndpointId, Attributes::TimeSinceReset::Id),
        .enableTxErrCount     = IsAttributeEnabled(kRootEndpointId, Attributes::TxErrCount::Id),
    };

    gServer.Create(enabledAttributes);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(gServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register EthernetNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
}

void emberAfEthernetNetworkDiagnosticsClusterShutdownCallback(EndpointId endpointId)
{
    VerifyOrReturn(endpointId == kRootEndpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&gServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister EthernetNetworkDiagnostics on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId,
                     err.Format());
    }
    gServer.Destroy();
}

void MatterEthernetNetworkDiagnosticsPluginServerInitCallback() {}

void MatterEthernetNetworkDiagnosticsPluginServerShutdownCallback() {}
