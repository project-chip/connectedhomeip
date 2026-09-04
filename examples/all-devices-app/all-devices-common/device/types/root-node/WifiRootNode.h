/*
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

#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/clusters/wifi-network-diagnostics-server/WiFiNetworkDiagnosticsCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/types/root-node/RootNode.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {

class WifiRootNode : public RootNode
{
public:
    struct WifiContext
    {
        DeviceLayer::NetworkCommissioning::WiFiDriver & wifiDriver;
    };

    WifiRootNode(const Context & context, const WifiContext & wifiContext) : RootNode(context), mWifiContext(wifiContext) {}
    ~WifiRootNode() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    LazyRegisteredServerCluster<Clusters::NetworkCommissioningCluster> mNetworkCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::WiFiDiagnosticsServerCluster> mWifiDiagnosticsCluster;
    WifiContext mWifiContext;
};

} // namespace app
} // namespace chip
