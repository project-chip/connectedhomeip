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
 * A commissioning proxy tunnels commissioning traffic for a device the commissioner
 * cannot reach directly, over a transport that is not IP: BLE today, Wi-Fi PAF next.
 * It is not an on-network path.
 *
 * At least one transport is required: Register() fails with CHIP_ERROR_INCORRECT_STATE
 * if none are present.
 *
 * Transports are composed in, not subclassed in: the platform factory override owns one
 * transport driver per technology it has compiled in and hands each to AddTransport().
 * Adding a technology is a transport driver plus one more AddTransport() call, with no
 * new device type and nothing here that knows which technologies exist.
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
    ///               rather than fixed here because which features a proxy offers
    ///               follows from the transports the platform built in, which only the
    ///               caller knows.
    CommissioningProxyDevice(const Context & context,
                             const Clusters::CommissioningProxy::CommissioningProxyCluster::Config & config);
    ~CommissioningProxyDevice() override = default;

    // Non-copyable / non-movable: the device holds pointers to transports it does not
    // own, and a registered transport holds a pointer back to the cluster this device
    // creates.
    CommissioningProxyDevice(const CommissioningProxyDevice &)             = delete;
    CommissioningProxyDevice & operator=(const CommissioningProxyDevice &) = delete;
    CommissioningProxyDevice(CommissioningProxyDevice &&)                  = delete;
    CommissioningProxyDevice & operator=(CommissioningProxyDevice &&)      = delete;

    /**
     * Add a transport this device will expose. Call at least once, and once per
     * transport, before Register(); each is registered on the cluster when the device
     * registers.
     *
     * The device does not take ownership: @p transport must outlive it, and must not be
     * shared with another device, since registering hands the transport a pointer back
     * to this device's cluster.
     */
    void AddTransport(Clusters::CommissioningProxy::CommissioningProxyTransport & transport);

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

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
