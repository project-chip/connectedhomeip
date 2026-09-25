/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <array>

#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/clusters/wifi-network-diagnostics-server/WiFiNetworkDiagnosticsCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/root-node/RootNode.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {

/// Feature policy that adds WiFi support to the root endpoint.
///
/// Owns the WiFi flavored `NetworkCommissioning` cluster and the WiFi
/// `NetworkDiagnostics` cluster, and wires the network-commissioning cluster
/// up to the root-node breadcrumb (via `GeneralCommissioningCluster`).
class WifiFeature
{
public:
    struct Context
    {
        DeviceLayer::NetworkCommissioning::WiFiDriver & wifiDriver;
    };

    explicit WifiFeature(const Context & context) : mContext(context) {}

    static constexpr std::array<DataModel::DeviceTypeEntry, 0> kExtraDeviceTypes{};
    static constexpr std::array<ClusterId, 0> kExtraClientClusters{};

    CHIP_ERROR RegisterFeatureClusters(EndpointId endpointId, CodeDrivenDataModelProvider & provider,
                                       RootNode::Context & rootContext,
                                       Clusters::GeneralCommissioningCluster & generalCommissioning);

    void UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider);

private:
    Context mContext;
    LazyRegisteredServerCluster<Clusters::NetworkCommissioningCluster> mNetworkCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::WiFiDiagnosticsServerCluster> mWifiDiagnosticsCluster;
};

} // namespace app
} // namespace chip
