/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/CASEClientPool.h>
#include <app/OperationalDeviceProxy.h>
#include <app/OperationalDeviceProxyPool.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/Pool.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SessionDelegate.h>

#include <lib/dnssd/ResolverProxy.h>

namespace chip {

struct CASEDeviceManagerConfig
{
    DeviceProxyInitParams deviceInitParams;
    OperationalDeviceProxyPoolDelegate * devicePool = nullptr;
};

/**
 * This class provides the following
 * 1. Manage a pool of operational device proxy objects for peer nodes that have active message exchange with the local node.
 * 2. The pool contains atmost one device proxy object for a given peer node.
 * 3. API to lookup an existing proxy object, or allocate a new one by triggering session establishment with the peer node.
 * 4. During session establishment, trigger node ID resolution (if needed), and update the DNS-SD cache (if resolution is
 * successful)
 */
class CASEDeviceManager
{
public:
    CASEDeviceManager() = default;
    virtual ~CASEDeviceManager() {}

    CHIP_ERROR Init(chip::System::Layer * systemLayer, const CASEDeviceManagerConfig & params);
    void Shutdown() {}

    /**
     * Find an existing session for the given node ID, or trigger a new session
     * request.
     *
     * The caller can optionally provide `onConnection` and `onFailure` callback
     * objects. If provided, these will be used to inform the caller about
     * successful or failed connection establishment.
     *
     * If the connection is already established, the `onConnection` callback
     * will be immediately called, before FindOrEstablishSession returns.
     *
     * The `onFailure` callback may be called before the FindOrEstablishSession
     * call returns, for error cases that are detected synchronously.
     */
    void FindOrInitializeDevice(PeerId peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    OperationalDeviceProxy * FindExistingDevice(PeerId peerId) const;

    void ReleaseDevice(PeerId peerId);

    void ReleaseDevicesForFabric(FabricIndex fabricIndex);

    void ReleaseAllDevices();

    /**
     * This API returns the address for the given node ID.
     * If the CASEDeviceManager is configured with a DNS-SD cache, the cache is looked up
     * for the node ID.
     * If the DNS-SD cache is not available, the CASEDeviceManager looks up the list for
     * an ongoing session with the peer node. If the session doesn't exist, the API will return
     * `CHIP_ERROR_NOT_CONNECTED` error.
     */
    CHIP_ERROR GetPeerAddress(PeerId peerId, Transport::PeerAddress & addr);

private:
    void ReleaseDevice(OperationalDeviceProxy * device) const;

    CASEDeviceManagerConfig mConfig;
};

} // namespace chip
