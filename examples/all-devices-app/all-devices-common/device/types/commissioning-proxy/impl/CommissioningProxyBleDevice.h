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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h>
#include <device/types/commissioning-proxy/CommissioningProxyDevice.h>

namespace chip {
namespace app {

/**
 * @brief A Commissioning Proxy device that proxies over BLE.
 *
 * Only built where the platform has a BLE stack: this file is compiled into the
 * "ble" source_set, which is depended on only when chip_config_network_layer_ble
 * is set. That keeps the transport choice in the build graph rather than in
 * preprocessor conditionals inside the device.
 */
class CommissioningProxyBleDevice : public CommissioningProxyDevice
{
public:
    /**
     * BackgroundScan is offered unconditionally over BLE. The transport implements
     * ProxyBackgroundScanStart/Stop for every platform — the only limit is that BLE
     * has a single scanner shared with foreground scan and connect, and the transport
     * arbitrates that itself, reporting BUSY rather than refusing the feature.
     *
     * WiFiNetworkInterface is not set: a BLE proxy reports no Wi-Fi band, so the
     * WiFiBand attribute stays absent.
     */
    static Clusters::CommissioningProxy::CommissioningProxyCluster::Config DefaultConfig()
    {
        return Clusters::CommissioningProxy::CommissioningProxyCluster::Config(
            BitMask<Clusters::CommissioningProxy::Feature>(Clusters::CommissioningProxy::Feature::kBackgroundScan));
    }

    /// @param bleAdapter the platform BLE hooks the transport needs. Owned by the
    ///                   caller and must outlive this device.
    CommissioningProxyBleDevice(const Context & context, Clusters::CommissioningProxy::CommissioningProxyBleAdapter & bleAdapter,
                                const Clusters::CommissioningProxy::CommissioningProxyCluster::Config & config = DefaultConfig());
    ~CommissioningProxyBleDevice() override = default;

private:
    Clusters::CommissioningProxy::CommissioningProxyBleTransport mBleTransport;
};

} // namespace app
} // namespace chip
