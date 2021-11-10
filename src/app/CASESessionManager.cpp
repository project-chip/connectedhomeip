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

#include <app/CASESessionManager.h>

namespace chip {

CHIP_ERROR CASESessionManager::FindOrEstablishSession(NodeId deviceId, Transport::PeerAddress address,
                                                      Callback::Callback<OnDeviceConnected> * onConnection,
                                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    PeerId peerId                    = mInitParams.sessionInitParams.fabricInfo->GetPeerIdForNode(deviceId);
    OperationalDeviceProxy * session = FindSession(deviceId);
    if (session == nullptr)
    {
        session = mActiveSessions.CreateObject(mInitParams.sessionInitParams, peerId);
        if (session == nullptr)
        {
            onFailure->mCall(onFailure->mContext, deviceId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    bool validAddress = (address != Transport::PeerAddress::UDP(Inet::IPAddress::Any));

    if (!validAddress && mInitParams.dnsCache != nullptr)
    {
        Dnssd::ResolvedNodeData nodeData;
        if (mInitParams.dnsCache->Lookup(peerId, nodeData) == CHIP_NO_ERROR)
        {
            address      = ToPeerAddress(nodeData);
            validAddress = true;
        }
    }

    if (validAddress)
    {
        uint32_t idleInterval;
        uint32_t activeInterval;
        session->GetMRPIntervals(idleInterval, activeInterval);
        CHIP_ERROR err = session->UpdateDeviceData(address, idleInterval, activeInterval);
        if (err != CHIP_NO_ERROR)
        {
            ReleaseSession(session);
            return err;
        }
    }

    CHIP_ERROR err = session->Connect(onConnection, onFailure);
    if (err != CHIP_NO_ERROR)
    {
        ReleaseSession(session);
    }

    return err;
}

void CASESessionManager::ReleaseSession(NodeId deviceId)
{
    ReleaseSession(FindSession(deviceId));
}

CHIP_ERROR CASESessionManager::ResolveDeviceAddress(NodeId deviceId)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    return Dnssd::Resolver::Instance().ResolveNodeId(mInitParams.sessionInitParams.fabricInfo->GetPeerIdForNode(deviceId),
                                                     Inet::IPAddressType::kAny);
}

Transport::PeerAddress CASESessionManager::ToPeerAddress(const Dnssd::ResolvedNodeData & nodeData) const
{
    Inet::InterfaceId interfaceId;

    // Only use the mDNS resolution's InterfaceID for addresses that are IPv6 LLA.
    // For all other addresses, we should rely on the device's routing table to route messages sent.
    // Forcing messages down an InterfaceId might fail. For example, in bridged networks like Thread,
    // mDNS advertisements are not usually received on the same interface the peer is reachable on.
    if (nodeData.mAddress[0].IsIPv6LinkLocal())
    {
        interfaceId = nodeData.mInterfaceId;
    }

    return Transport::PeerAddress::UDP(nodeData.mAddress[0], nodeData.mPort, interfaceId);
}

void CASESessionManager::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    VerifyOrReturn(mInitialized);

    if (mInitParams.dnsCache != nullptr)
    {
        LogErrorOnFailure(mInitParams.dnsCache->Insert(nodeData));
    }

    OperationalDeviceProxy * session = FindSession(nodeData.mPeerId.GetNodeId());
    VerifyOrReturn(session != nullptr);

    LogErrorOnFailure(session->UpdateDeviceData(
        ToPeerAddress(nodeData), nodeData.GetMrpRetryIntervalIdle().ValueOr(CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL),
        nodeData.GetMrpRetryIntervalActive().ValueOr(CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL)));
}

void CASESessionManager::OnNodeIdResolutionFailed(const PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
}

CHIP_ERROR CASESessionManager::GetDeviceAddressAndPort(NodeId deviceId, Inet::IPAddress & addr, uint16_t & port)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    OperationalDeviceProxy * session = FindSession(deviceId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_CONNECTED);
    VerifyOrReturnError(session->GetAddress(addr, port), CHIP_ERROR_NOT_CONNECTED);
    return CHIP_NO_ERROR;
}

void CASESessionManager::OnNewConnection(SessionHandle sessionHandle, Messaging::ExchangeManager * mgr) {}

void CASESessionManager::OnConnectionExpired(SessionHandle sessionHandle, Messaging::ExchangeManager * mgr)
{
    VerifyOrReturn(mInitialized);

    OperationalDeviceProxy * session = FindSession(sessionHandle);
    VerifyOrReturn(session != nullptr,
                   ChipLogDetail(Controller, "OnConnectionExpired was called for unknown device, ignoring it."));

    session->OnConnectionExpired(sessionHandle);
}

OperationalDeviceProxy * CASESessionManager::FindSession(SessionHandle session)
{
    OperationalDeviceProxy * foundSession = nullptr;
    mActiveSessions.ForEachActiveObject([&](auto * activeSession) {
        if (activeSession->MatchesSession(session))
        {
            foundSession = activeSession;
            return false;
        }
        return true;
    });

    return foundSession;
}

OperationalDeviceProxy * CASESessionManager::FindSession(NodeId id)
{
    OperationalDeviceProxy * foundSession = nullptr;
    mActiveSessions.ForEachActiveObject([&](auto * activeSession) {
        if (activeSession->GetDeviceId() == id)
        {
            foundSession = activeSession;
            return false;
        }
        return true;
    });

    return foundSession;
}

void CASESessionManager::ReleaseSession(OperationalDeviceProxy * session)
{
    if (session != nullptr)
    {
        mActiveSessions.ReleaseObject(session);
    }
}

} // namespace chip
