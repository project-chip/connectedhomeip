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

CHIP_ERROR CASESessionManager::FindOrEstablishSession(NodeId nodeId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    Dnssd::ResolvedNodeData resolutionData;

    PeerId peerId = GetFabricInfo()->GetPeerIdForNode(nodeId);

    bool nodeIDWasResolved = (mConfig.dnsCache != nullptr && mConfig.dnsCache->Lookup(peerId, resolutionData) == CHIP_NO_ERROR);

    OperationalDeviceProxy * session = FindExistingSession(nodeId);
    if (session == nullptr)
    {
        // TODO - Implement LRU to evict least recently used session to handle mActiveSessions pool exhaustion
        if (nodeIDWasResolved)
        {
            session = mActiveSessions.CreateObject(mConfig.sessionInitParams, peerId, resolutionData);
        }
        else
        {
            session = mActiveSessions.CreateObject(mConfig.sessionInitParams, peerId);
        }

        if (session == nullptr)
        {
            onFailure->mCall(onFailure->mContext, nodeId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    else if (nodeIDWasResolved)
    {
        session->OnNodeIdResolved(resolutionData);
    }

    CHIP_ERROR err = session->Connect(onConnection, onFailure);
    if (err != CHIP_NO_ERROR)
    {
        ReleaseSession(session);
    }

    return err;
}

void CASESessionManager::ReleaseSession(NodeId nodeId)
{
    ReleaseSession(FindExistingSession(nodeId));
}

CHIP_ERROR CASESessionManager::ResolveDeviceAddress(NodeId nodeId)
{
    return Dnssd::Resolver::Instance().ResolveNodeId(GetFabricInfo()->GetPeerIdForNode(nodeId), Inet::IPAddressType::kAny,
                                                     Dnssd::Resolver::CacheBypass::On);
}

void CASESessionManager::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "Address resolved for node: 0x" ChipLogFormatX64, ChipLogValueX64(nodeData.mPeerId.GetNodeId()));

    if (mConfig.dnsCache != nullptr)
    {
        LogErrorOnFailure(mConfig.dnsCache->Insert(nodeData));
    }

    OperationalDeviceProxy * session = FindExistingSession(nodeData.mPeerId.GetNodeId());
    VerifyOrReturn(session != nullptr,
                   ChipLogDetail(Controller, "OnNodeIdResolved was called for a device with no active sessions, ignoring it."));

    LogErrorOnFailure(session->UpdateDeviceData(session->ToPeerAddress(nodeData), nodeData.GetMRPConfig()));
}

void CASESessionManager::OnNodeIdResolutionFailed(const PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
}

CHIP_ERROR CASESessionManager::GetPeerAddress(NodeId nodeId, Transport::PeerAddress & addr)
{
    if (mConfig.dnsCache != nullptr)
    {
        Dnssd::ResolvedNodeData resolutionData;
        ReturnErrorOnFailure(mConfig.dnsCache->Lookup(GetFabricInfo()->GetPeerIdForNode(nodeId), resolutionData));
        addr = OperationalDeviceProxy::ToPeerAddress(resolutionData);
        return CHIP_NO_ERROR;
    }

    OperationalDeviceProxy * session = FindExistingSession(nodeId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_CONNECTED);
    addr = session->GetPeerAddress();
    return CHIP_NO_ERROR;
}

void CASESessionManager::OnSessionReleased(SessionHandle sessionHandle)
{
    OperationalDeviceProxy * session = FindSession(sessionHandle);
    VerifyOrReturn(session != nullptr, ChipLogDetail(Controller, "OnSessionReleased was called for unknown device, ignoring it."));

    session->OnSessionReleased(sessionHandle);
}

OperationalDeviceProxy * CASESessionManager::FindSession(SessionHandle session)
{
    OperationalDeviceProxy * foundSession = nullptr;
    mActiveSessions.ForEachActiveObject([&](auto * activeSession) {
        if (activeSession->MatchesSession(session))
        {
            foundSession = activeSession;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    return foundSession;
}

OperationalDeviceProxy * CASESessionManager::FindExistingSession(NodeId id)
{
    OperationalDeviceProxy * foundSession = nullptr;
    mActiveSessions.ForEachActiveObject([&](auto * activeSession) {
        if (activeSession->GetDeviceId() == id)
        {
            foundSession = activeSession;
            return Loop::Break;
        }
        return Loop::Continue;
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
