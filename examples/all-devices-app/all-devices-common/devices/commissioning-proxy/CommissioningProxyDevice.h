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
#include <devices/interface/SingleEndpointDevice.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

#if CONFIG_NETWORK_LAYER_BLE
#include "CommissioningProxyBleTransport.h"
#endif

namespace chip {
namespace app {

class CommissioningProxyDevice : public SingleEndpointDevice
{
public:
    CommissioningProxyDevice();
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
