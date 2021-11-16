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
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    Dnssd::ResolvedNodeData * nodeResolutionData = nullptr;
    Dnssd::ResolvedNodeData cachedResolutionData;

    PeerId peerId = GetFabricInfo()->GetPeerIdForNode(nodeId);

    if (mConfig.dnsCache != nullptr && mConfig.dnsCache->Lookup(peerId, cachedResolutionData) == CHIP_NO_ERROR)
    {
        nodeResolutionData = &cachedResolutionData;
    }

    OperationalDeviceProxy * session = FindExistingSession(nodeId);
    if (session == nullptr)
    {
        // TODO - Implement LRU to evict least recently used session to handle mActiveSessions pool exhaustion
        session = mActiveSessions.CreateObject(mConfig.sessionInitParams, peerId, nodeResolutionData);
        if (session == nullptr)
        {
            onFailure->mCall(onFailure->mContext, nodeId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    else if (nodeResolutionData != nullptr)
    {
        session->OnNodeIdResolved(nodeResolutionData);
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
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    return Dnssd::Resolver::Instance().ResolveNodeId(GetFabricInfo()->GetPeerIdForNode(nodeId), Inet::IPAddressType::kAny);
}

void CASESessionManager::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "Address resolved for node: 0x" ChipLogFormatX64, ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
    VerifyOrReturn(mInitialized, ChipLogError(Controller, "OnNodeIdResolved was called in uninitialized state"));

    if (mConfig.dnsCache != nullptr)
    {
        LogErrorOnFailure(mConfig.dnsCache->Insert(nodeData));
    }

    OperationalDeviceProxy * session = FindExistingSession(nodeData.mPeerId.GetNodeId());
    VerifyOrReturn(session != nullptr,
                   ChipLogDetail(Controller, "OnNodeIdResolved was called for a device with no active sessions, ignoring it."));

    LogErrorOnFailure(session->UpdateDeviceData(
        session->ToPeerAddress(nodeData), nodeData.GetMrpRetryIntervalIdle().ValueOr(CHIP_CONFIG_MRP_DEFAULT_IDLE_RETRY_INTERVAL),
        nodeData.GetMrpRetryIntervalActive().ValueOr(CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL)));
}

void CASESessionManager::OnNodeIdResolutionFailed(const PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
}

CHIP_ERROR CASESessionManager::GetDeviceAddressAndPort(NodeId nodeId, Inet::IPAddress & addr, uint16_t & port)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);

    OperationalDeviceProxy * session = FindExistingSession(nodeId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_CONNECTED);
    VerifyOrReturnError(session->GetAddress(addr, port), CHIP_ERROR_NOT_CONNECTED);
    return CHIP_NO_ERROR;
}

void CASESessionManager::OnNewConnection(SessionHandle sessionHandle, Messaging::ExchangeManager * mgr)
{
    // TODO Update the MRP params based on the MRP params extracted from CASE, when this is available.
}

void CASESessionManager::OnConnectionExpired(SessionHandle sessionHandle, Messaging::ExchangeManager * mgr)
{
    VerifyOrReturn(mInitialized, ChipLogError(Controller, "OnConnectionExpired was called in uninitialized state"));

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

OperationalDeviceProxy * CASESessionManager::FindExistingSession(NodeId id)
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
