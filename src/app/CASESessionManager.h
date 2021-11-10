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

struct CASESessionManagerInitParams
{
    DeviceProxyInitParams sessionInitParams;
    Dnssd::DnssdCache<CHIP_CONFIG_MDNS_CACHE_SIZE> * dnsCache = nullptr;
};

class CASESessionManager : public Messaging::ExchangeMgrDelegate, public Dnssd::ResolverDelegate
{
public:
    CASESessionManager(CASESessionManagerInitParams & params)
    {
        VerifyOrReturn(params.sessionInitParams.Validate() == CHIP_NO_ERROR);

        mInitParams  = params;
        mInitialized = true;
    }

    virtual ~CASESessionManager() {}

    void Shutdown() { mInitialized = false; }

    CHIP_ERROR FindOrEstablishSession(NodeId deviceId, Transport::PeerAddress addr,
                                      Callback::Callback<OnDeviceConnected> * onConnection,
                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure);

    OperationalDeviceProxy * FindExistingSession(NodeId deviceId) { return FindSession(deviceId); }

    void ReleaseSession(NodeId deviceId);

    CHIP_ERROR ResolveDeviceAddress(NodeId deviceId);
    CHIP_ERROR GetDeviceAddressAndPort(NodeId deviceId, Inet::IPAddress & addr, uint16_t & port);

    //////////// ExchangeMgrDelegate Implementation ///////////////
    void OnNewConnection(SessionHandle session, Messaging::ExchangeManager * mgr) override;
    void OnConnectionExpired(SessionHandle session, Messaging::ExchangeManager * mgr) override;

    //////////// ResolverDelegate Implementation ///////////////
    void OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData) override;
    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override;
    void OnNodeDiscoveryComplete(const Dnssd::DiscoveredNodeData & nodeData) override {}

private:
    OperationalDeviceProxy * FindSession(SessionHandle session);
    OperationalDeviceProxy * FindSession(NodeId id);
    void ReleaseSession(OperationalDeviceProxy * device);

    Transport::PeerAddress ToPeerAddress(const Dnssd::ResolvedNodeData & nodeData) const;

    BitMapObjectPool<OperationalDeviceProxy, CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES> mActiveSessions;

    CASESessionManagerInitParams mInitParams;

    bool mInitialized = false;
};

} // namespace chip
