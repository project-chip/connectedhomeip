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

#pragma once

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

#if CONFIG_NETWORK_LAYER_BLE
// The ble-transport dependency in BUILD.gn is conditional on chip_config_network_layer_ble
// The gn check does not evaluate the preprocessor, hence the use of nogncheck below
// Otherwise the CI, REPL Tests Linux (BUILD) will fail
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleAdapter.h>   // nogncheck
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h> // nogncheck
#endif

namespace chip {
namespace app {

class CommissioningProxyDevice : public SingleEndpoint
{
public:
#if CONFIG_NETWORK_LAYER_BLE
    /// @param bleAdapter platform BLE hooks for the BLE transport; must outlive this
    ///                   device. Supplied by the platform device factory.
    explicit CommissioningProxyDevice(Clusters::CommissioningProxy::CommissioningProxyBleAdapter & bleAdapter);
#else
    CommissioningProxyDevice();
#endif
    ~CommissioningProxyDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

private:
    LazyRegisteredServerCluster<Clusters::CommissioningProxy::CommissioningProxyCluster> mCluster;

    // Platform transport drivers registered on the cluster (owned here).
#if CONFIG_NETWORK_LAYER_BLE
    Clusters::CommissioningProxy::CommissioningProxyBleTransport mBleTransport;
#endif
};

} // namespace app
} // namespace chip
