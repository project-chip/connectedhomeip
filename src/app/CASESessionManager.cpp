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
    params.sessionInitParams.exchangeMgr->GetReliableMessageMgr()->RegisterSessionUpdateDelegate(this);
    return AddressResolve::Resolver::Instance().Init(systemLayer);
}

void CASESessionManager::FindOrEstablishSession(const ScopedNodeId & peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                Callback::Callback<OnDeviceConnectionFailure> * onFailure
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                                ,
                                                uint8_t attemptCount, Callback::Callback<OnDeviceConnectionRetry> * onRetry
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
)
{
    FindOrEstablishSessionHelper(peerId, onConnection, onFailure, nullptr
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                 ,
                                 attemptCount, onRetry
#endif
    );
}

void CASESessionManager::FindOrEstablishSession(const ScopedNodeId & peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                Callback::Callback<OperationalSessionSetup::OnSetupFailure> * onSetupFailure
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                                ,
                                                uint8_t attemptCount, Callback::Callback<OnDeviceConnectionRetry> * onRetry
#endif
)
{
    FindOrEstablishSessionHelper(peerId, onConnection, nullptr, onSetupFailure
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                 ,
                                 attemptCount, onRetry
#endif
    );
}

void CASESessionManager::FindOrEstablishSession(const ScopedNodeId & peerId, Callback::Callback<OnDeviceConnected> * onConnection,
                                                nullptr_t
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                                ,
                                                uint8_t attemptCount, Callback::Callback<OnDeviceConnectionRetry> * onRetry
#endif
)
{
    FindOrEstablishSessionHelper(peerId, onConnection, nullptr, nullptr
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                 ,
                                 attemptCount, onRetry
#endif
    );
}

void CASESessionManager::FindOrEstablishSessionHelper(const ScopedNodeId & peerId,
                                                      Callback::Callback<OnDeviceConnected> * onConnection,
                                                      Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                                                      Callback::Callback<OperationalSessionSetup::OnSetupFailure> * onSetupFailure
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
                                                      ,
                                                      uint8_t attemptCount, Callback::Callback<OnDeviceConnectionRetry> * onRetry
#endif
)
{
    ChipLogDetail(CASESessionManager, "FindOrEstablishSession: PeerId = [%d:" ChipLogFormatX64 "]", peerId.GetFabricIndex(),
                  ChipLogValueX64(peerId.GetNodeId()));

    bool forAddressUpdate             = false;
    OperationalSessionSetup * session = FindExistingSessionSetup(peerId, forAddressUpdate);
    if (session == nullptr)
    {
        ChipLogDetail(CASESessionManager, "FindOrEstablishSession: No existing OperationalSessionSetup instance found");
        session = mConfig.sessionSetupPool->Allocate(mConfig.sessionInitParams, mConfig.clientPool, peerId, this);

        if (session == nullptr)
        {
            if (onFailure != nullptr)
            {
                onFailure->mCall(onFailure->mContext, peerId, CHIP_ERROR_NO_MEMORY);
            }

            if (onSetupFailure != nullptr)
            {
                OperationalSessionSetup::ConnnectionFailureInfo failureInfo(peerId, CHIP_ERROR_NO_MEMORY,
                                                                            SessionEstablishmentStage::kUnknown);
                onSetupFailure->mCall(onSetupFailure->mContext, failureInfo);
            }
            return;
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
    session->UpdateAttemptCount(attemptCount);
    if (onRetry)
    {
        session->AddRetryHandler(onRetry);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES

    if (onFailure != nullptr)
    {
        session->Connect(onConnection, onFailure);
    }

    if (onSetupFailure != nullptr)
    {
        session->Connect(onConnection, onSetupFailure);
    }
}

void CASESessionManager::ReleaseSessionsForFabric(FabricIndex fabricIndex)
{
    mConfig.sessionSetupPool->ReleaseAllSessionSetupsForFabric(fabricIndex);
}

void CASESessionManager::ReleaseAllSessions()
{
    mConfig.sessionSetupPool->ReleaseAllSessionSetup();
}

CHIP_ERROR CASESessionManager::GetPeerAddress(const ScopedNodeId & peerId, Transport::PeerAddress & addr)
{
    ReturnErrorOnFailure(mConfig.sessionInitParams.Validate());
    auto optionalSessionHandle = FindExistingSession(peerId);
    ReturnErrorCodeIf(!optionalSessionHandle.HasValue(), CHIP_ERROR_NOT_CONNECTED);
    addr = optionalSessionHandle.Value()->AsSecureSession()->GetPeerAddress();
    return CHIP_NO_ERROR;
}

void CASESessionManager::UpdatePeerAddress(ScopedNodeId peerId)
{
    bool forAddressUpdate             = true;
    OperationalSessionSetup * session = FindExistingSessionSetup(peerId, forAddressUpdate);
    if (session == nullptr)
    {
        ChipLogDetail(CASESessionManager, "UpdatePeerAddress: No existing OperationalSessionSetup instance found");

        session = mConfig.sessionSetupPool->Allocate(mConfig.sessionInitParams, mConfig.clientPool, peerId, this);
        if (session == nullptr)
        {
            ChipLogDetail(CASESessionManager, "UpdatePeerAddress: Failed to allocate OperationalSessionSetup instance");
            return;
        }
    }
    else
    {
        ChipLogDetail(CASESessionManager,
                      "UpdatePeerAddress: Found existing OperationalSessionSetup instance for peerId[" ChipLogFormatX64 "]",
                      ChipLogValueX64(peerId.GetNodeId()));
    }

    session->PerformAddressUpdate();
}

OperationalSessionSetup * CASESessionManager::FindExistingSessionSetup(const ScopedNodeId & peerId, bool forAddressUpdate) const
{
    return mConfig.sessionSetupPool->FindSessionSetup(peerId, forAddressUpdate);
}

Optional<SessionHandle> CASESessionManager::FindExistingSession(const ScopedNodeId & peerId) const
{
    return mConfig.sessionInitParams.sessionManager->FindSecureSessionForNode(peerId,
                                                                              MakeOptional(Transport::SecureSession::Type::kCASE));
}

void CASESessionManager::ReleaseSession(OperationalSessionSetup * session)
{
    if (session != nullptr)
    {
        mConfig.sessionSetupPool->Release(session);
    }
}

} // namespace chip
