/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/CASEClient.h>

namespace chip {

CASEClient::CASEClient(const CASEClientInitParams & params) : mInitParams(params) {}

void CASEClient::SetMRPIntervals(const ReliableMessageProtocolConfig & mrpConfig)
{
    mCASESession.SetMRPConfig(mrpConfig);
}

CHIP_ERROR CASEClient::EstablishSession(PeerId peer, const Transport::PeerAddress & peerAddress,
                                        const ReliableMessageProtocolConfig & mrpConfig, OnCASEConnected onConnection,
                                        OnCASEConnectionFailure onFailure, void * context)
{
    // Create a UnauthenticatedSession for CASE pairing.
    // Don't use mSecureSession here, because mSecureSession is for encrypted communication.
    Optional<SessionHandle> session = mInitParams.sessionManager->CreateUnauthenticatedSession(peerAddress, mrpConfig);
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    uint16_t keyID = 0;
    ReturnErrorOnFailure(mInitParams.idAllocator->Allocate(keyID));

    // Allocate the exchange immediately before calling CASESession::EstablishSession.
    //
    // CASESession::EstablishSession takes ownership of the exchange and will
    // free it on error, but can only do this if it is actually called.
    // Allocating the exchange context right before calling EstablishSession
    // ensures that if allocation succeeds, CASESession has taken ownership.
    Messaging::ExchangeContext * exchange = mInitParams.exchangeMgr->NewContext(session.Value(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(mCASESession.EstablishSession(peerAddress, mInitParams.fabricInfo, peer.GetNodeId(), keyID, exchange, this,
                                                       mInitParams.mrpLocalConfig));
    mConnectionSuccessCallback = onConnection;
    mConnectionFailureCallback = onFailure;
    mConectionContext          = context;
    mPeerId                    = peer;
    mPeerAddress               = peerAddress;

    return CHIP_NO_ERROR;
}

void CASEClient::OnSessionEstablishmentError(CHIP_ERROR error)
{
    mInitParams.idAllocator->Free(mCASESession.GetLocalSessionId());

    if (mConnectionFailureCallback)
    {
        mConnectionFailureCallback(mConectionContext, this, error);
    }
}

void CASEClient::OnSessionEstablished()
{
    // On successful CASE connection, the local session ID will be used for the derived secure session.
    if (mConnectionSuccessCallback)
    {
        mConnectionSuccessCallback(mConectionContext, this);
    }
}

CHIP_ERROR CASEClient::DeriveSecureSessionHandle(SessionHolder & handle)
{
    CHIP_ERROR err = mInitParams.sessionManager->NewPairing(
        handle, Optional<Transport::PeerAddress>::Value(mPeerAddress), mPeerId.GetNodeId(), &mCASESession,
        CryptoContext::SessionRole::kInitiator, mInitParams.fabricInfo->GetFabricIndex());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed in setting up CASE secure channel: err %s", ErrorStr(err));
        return err;
    }

    return CHIP_NO_ERROR;
}

}; // namespace chip
