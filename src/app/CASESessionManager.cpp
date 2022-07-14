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
#include <lib/address_resolve/AddressResolve.h>

namespace chip {

CHIP_ERROR CASESessionManager::Init(chip::System::Layer * systemLayer, const CASESessionManagerConfig & params)
{
    ReturnErrorOnFailure(params.sessionInitParams.Validate());
    mConfig = params;
    return AddressResolve::Resolver::Instance().Init(systemLayer);
}

void CASESessionManager::FindOrEstablishSession(const ScopedNodeId & peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                Callback::Callback<OnDeviceConnectionFailure> * onFailure)
{
    ChipLogDetail(CASESessionManager, "FindOrEstablishSession: PeerId = [%d:" ChipLogFormatX64 "]", peerId.GetFabricIndex(),
                  ChipLogValueX64(peerId.GetNodeId()));

    OperationalSessionSetup * session = FindExistingSession(peerId);
    if (session == nullptr)
    {
        ChipLogDetail(CASESessionManager, "FindOrEstablishSession: No existing OperationalSessionSetup instance found");

        session = mConfig.devicePool->Allocate(mConfig.sessionInitParams, peerId, this);

        if (session == nullptr)
        {
            if (onFailure != nullptr)
            {
                onFailure->mCall(onFailure->mContext, peerId, CHIP_ERROR_NO_MEMORY);
            }
            return;
        }
    }

    session->Connect(onConnection, onFailure);
}

void CASESessionManager::ReleaseSession(const ScopedNodeId & peerId)
{
    ReleaseSession(FindExistingSession(peerId));
}

void CASESessionManager::ReleaseSessionsForFabric(FabricIndex fabricIndex)
{
    mConfig.devicePool->ReleaseDevicesForFabric(fabricIndex);
}

void CASESessionManager::ReleaseAllSessions()
{
    mConfig.devicePool->ReleaseAllDevices();
}

CHIP_ERROR CASESessionManager::GetPeerAddress(const ScopedNodeId & peerId, Transport::PeerAddress & addr)
{
    ReturnErrorOnFailure(mConfig.sessionInitParams.Validate());
    auto sessionHandle = mConfig.sessionInitParams.sessionManager->FindSecureSessionForNode(
        peerId, MakeOptional(Transport::SecureSession::Type::kCASE));
    ReturnErrorCodeIf(!sessionHandle.HasValue(), CHIP_ERROR_NOT_CONNECTED);
    addr = sessionHandle.Value()->AsSecureSession()->GetPeerAddress();
    return CHIP_NO_ERROR;
}

OperationalSessionSetup * CASESessionManager::FindExistingSession(const ScopedNodeId & peerId) const
{
    return mConfig.devicePool->FindDevice(peerId);
}

void CASESessionManager::ReleaseSession(OperationalSessionSetup * session) const
{
    if (session != nullptr)
    {
        mConfig.devicePool->Release(session);
    }
}

} // namespace chip
