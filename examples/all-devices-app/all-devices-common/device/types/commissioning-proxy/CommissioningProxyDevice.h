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
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <credentials/FabricTable.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

#include <cstddef>

namespace chip {
namespace app {

/**
 * @brief Generic Commissioning Proxy device.
 *
 * A commissioning proxy relays commissioning traffic for a device the commissioner
 * cannot reach directly, over a transport that is not IP: BLE today, Wi-Fi PAF next.
 * It is not an on-network path — a proxy with no transport registered still answers
 * reads, but reports an empty Capabilities and fails every ProxyConnectRequest and
 * ProxyScanRequest. Such a build is only useful to keep a no-transport configuration
 * compiling; a real product registers at least one transport.
 *
 * Transports are supplied by subclasses, one per transport the platform has: see
 * impl/CommissioningProxyBleDevice.h. A subclass owns its transport as a member and
 * hands it to AddTransport() from its constructor body.
 */
class CommissioningProxyDevice : public SingleEndpoint
{
public:
    struct Context
    {
        /// Watched by the cluster so a removed fabric's sessions and background scans
        /// go with it.
        FabricTable & fabricTable;
        /// Supplies the cluster's response-timeout, scan-watchdog and cache-sweep
        /// timers, and any timer a registered transport needs.
        TimerDelegate & timerDelegate;
    };

    /// @param config the cluster's feature map and supported Wi-Fi bands. Injected
    ///               rather than fixed here because which features a proxy offers is a
    ///               product decision; subclasses supply a default that matches the
    ///               transport they add.
    CommissioningProxyDevice(const Context & context,
                             const Clusters::CommissioningProxy::CommissioningProxyCluster::Config & config);
    ~CommissioningProxyDevice() override = default;

    // Non-copyable / non-movable: subclasses pass AddTransport() a reference to a
    // transport they own as a member, and copying the device would leave that
    // reference pointing at the original's member.
    CommissioningProxyDevice(const CommissioningProxyDevice &)             = delete;
    CommissioningProxyDevice & operator=(const CommissioningProxyDevice &) = delete;
    CommissioningProxyDevice(CommissioningProxyDevice &&)                  = delete;
    CommissioningProxyDevice & operator=(CommissioningProxyDevice &&)      = delete;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    /**
     * Add a transport this device will expose, to be registered on the cluster when
     * the device is registered.
     *
     * Call this from a subclass constructor body, not from its member initializer
     * list: the transport is a subclass member and so is constructed after this base.
     * @p transport must outlive the device.
     */
    void AddTransport(Clusters::CommissioningProxy::CommissioningProxyTransport & transport);

private:
    // One slot per transport type; matches CommissioningProxyCluster's own limit.
    static constexpr size_t kMaxTransports = 3;

    const Context mContext;

    const Clusters::CommissioningProxy::CommissioningProxyCluster::Config mConfig;

    Clusters::CommissioningProxy::CommissioningProxyTransport * mTransports[kMaxTransports] = {};
    size_t mTransportCount                                                                  = 0;

    LazyRegisteredServerCluster<Clusters::CommissioningProxy::CommissioningProxyCluster> mCluster;
};

} // namespace app
} // namespace chip
