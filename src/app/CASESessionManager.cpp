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
#include <platform/CHIPDeviceLayer.h>

namespace chip {

CHIP_ERROR CASESessionManager::FindOrEstablishSession(PeerId peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    Dnssd::ResolvedNodeData resolutionData;

    bool nodeIDWasResolved = (mConfig.dnsCache != nullptr && mConfig.dnsCache->Lookup(peerId, resolutionData) == CHIP_NO_ERROR);

    OperationalDeviceProxy * session = FindExistingSession(peerId);
    if (session == nullptr)
    {
        // TODO - Implement LRU to evict least recently used session to handle mActiveSessions pool exhaustion
        if (nodeIDWasResolved)
        {
            session = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId, resolutionData);
        }
        else
        {
            session = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId);
        }

        if (session == nullptr)
        {
            onFailure->mCall(onFailure->mContext, peerId, CHIP_ERROR_NO_MEMORY);
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    else if (nodeIDWasResolved)
    {
        session->OnNodeIdResolved(resolutionData);
    }

    CHIP_ERROR err = session->Connect(onConnection, onFailure, mConfig.dnsResolver);
    if (err != CHIP_NO_ERROR)
    {
        // Release the peer rather than the pointer in case the failure handler has already released the session.
        ReleaseSession(peerId);
    }

    return err;
}

void CASESessionManager::ReleaseSession(PeerId peerId)
{
    ReleaseSession(FindExistingSession(peerId));
}

void CASESessionManager::ReleaseSessionForFabric(CompressedFabricId compressedFabricId)
{
    mConfig.devicePool->ReleaseDeviceForFabric(compressedFabricId);
}

CHIP_ERROR CASESessionManager::ResolveDeviceAddress(FabricInfo * fabric, NodeId nodeId)
{
    VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mConfig.dnsResolver->ResolveNodeId(fabric->GetPeerIdForNode(nodeId), Inet::IPAddressType::kAny,
                                              Dnssd::Resolver::CacheBypass::On);
}

void CASESessionManager::OnNodeIdResolved(const Dnssd::ResolvedNodeData & nodeData)
{
    ChipLogProgress(Controller, "Address resolved for node: 0x" ChipLogFormatX64, ChipLogValueX64(nodeData.mPeerId.GetNodeId()));

    if (mConfig.dnsCache != nullptr)
    {
        LogErrorOnFailure(mConfig.dnsCache->Insert(nodeData));
    }

    OperationalDeviceProxy * session = FindExistingSession(nodeData.mPeerId);
    VerifyOrReturn(session != nullptr,
                   ChipLogDetail(Controller, "OnNodeIdResolved was called for a device with no active sessions, ignoring it."));

    LogErrorOnFailure(session->UpdateDeviceData(OperationalDeviceProxy::ToPeerAddress(nodeData), nodeData.GetMRPConfig()));
}

void CASESessionManager::OnNodeIdResolutionFailed(const PeerId & peer, CHIP_ERROR error)
{
    ChipLogError(Controller, "Error resolving node id: %s", ErrorStr(error));
}

CHIP_ERROR CASESessionManager::GetPeerAddress(PeerId peerId, Transport::PeerAddress & addr)
{
    if (mConfig.dnsCache != nullptr)
    {
        Dnssd::ResolvedNodeData resolutionData;
        ReturnErrorOnFailure(mConfig.dnsCache->Lookup(peerId, resolutionData));
        addr = OperationalDeviceProxy::ToPeerAddress(resolutionData);
        return CHIP_NO_ERROR;
    }

    OperationalDeviceProxy * session = FindExistingSession(peerId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NOT_CONNECTED);
    addr = session->GetPeerAddress();
    return CHIP_NO_ERROR;
}

OperationalDeviceProxy * CASESessionManager::FindSession(const SessionHandle & session)
{
    return mConfig.devicePool->FindDevice(session);
}

OperationalDeviceProxy * CASESessionManager::FindExistingSession(PeerId peerId)
{
    return mConfig.devicePool->FindDevice(peerId);
}

void CASESessionManager::ReleaseSession(OperationalDeviceProxy * session)
{
    if (session != nullptr)
    {
        mConfig.devicePool->Release(session);
    }
}

} // namespace chip
