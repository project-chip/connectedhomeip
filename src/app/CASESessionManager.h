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

#include <app/OperationalDeviceProxy.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/dnssd/DnssdCache.h>
#include <lib/support/Pool.h>
#include <messaging/ExchangeMgrDelegate.h>

#include <lib/dnssd/Resolver.h>

namespace chip {

struct CASESessionManagerConfig
{
    DeviceProxyInitParams sessionInitParams;
    Dnssd::DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE> * dnsCache = nullptr;
};

/**
 * This class provides the following
 * 1. Manage a pool of operational device proxy objects for peer nodes that have active message exchange with the local node.
 * 2. The pool contains atmost one device proxy object for a given peer node.
 * 3. API to lookup an existing proxy object, or allocate a new one by triggering session establishment with the peer node.
 * 4. During session establishment, trigger node ID resolution (if needed), and update the DNS-SD cache (if resolution is
 * successful)
 */
class CASESessionManager : public Messaging::ExchangeMgrDelegate, public Dnssd::ResolverDelegate
{
public:
    CASESessionManager(CASESessionManagerConfig & params)
    {
        VerifyOrReturn(params.sessionInitParams.Validate() == CHIP_NO_ERROR);

        mConfig      = params;
        mInitialized = true;
    }

    virtual ~CASESessionManager() {}

    void Shutdown() { mInitialized = false; }

    /**
     * Find an existing session for the given node ID, or trigger a new session request.
     * The caller can optionally provide `onConnection` and `onFailure` callback objects. If provided,
     * these will be used to inform the caller about successful or failed connection establishment.
     * If the connection is already established, the `onConnection` callback will be immediately called.
     */
    CHIP_ERROR FindOrEstablishSession(NodeId nodeId, Callback::Callback<OnDeviceConnected> * onConnection,
                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    OperationalDeviceProxy * FindExistingSession(NodeId nodeId);

    void ReleaseSession(NodeId nodeId);

    FabricInfo * GetFabricInfo()
    {
        if (!mInitialized)
        {
            return nullptr;
        }

        return mConfig.sessionInitParams.fabricInfo;
    }

    /**
     * This API triggers the DNS-SD resolution for the given node ID. The node ID will be looked up
     * on the fabric that was configured for the CASESessionManager object.
     *
     * The results of the DNS-SD resolution request is provided to the class via `ResolverDelegate`
     * implementation of CASESessionManager.
     */
    CHIP_ERROR ResolveDeviceAddress(NodeId nodeId);

    /**
     * This API returns the address and port for the given node ID. It is expected that there is
     * an ongoing session with the peer node. If the session doesn't exist, the API will return
     * `CHIP_ERROR_NOT_CONNECTED` error.
     */
    CHIP_ERROR GetDeviceAddressAndPort(NodeId nodeId, Inet::IPAddress & addr, uint16_t & port);

    //////////// ExchangeMgrDelegate Implementation ///////////////
    void OnNewConnection(SessionHandle session, Messaging::ExchangeManager * mgr) override;
    void OnConnectionExpired(SessionHandle session, Messaging::ExchangeManager * mgr) override;

    //////////// ResolverDelegate Implementation ///////////////
    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override;
    void OnNodeDiscoveryComplete(const Dnssd::DiscoveredNodeData & nodeData) override {}

private:
    OperationalDeviceProxy * FindSession(SessionHandle session);
    void ReleaseSession(OperationalDeviceProxy * device);

    BitMapObjectPool<OperationalDeviceProxy, CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES> mActiveSessions;

    CASESessionManagerConfig mConfig;

    bool mInitialized = false;
};

} // namespace chip
