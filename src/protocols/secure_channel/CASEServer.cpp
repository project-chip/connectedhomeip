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

#include <protocols/secure_channel/CASEServer.h>

#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <transport/SecureSessionMgr.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::Credentials;

namespace chip {

CHIP_ERROR CASEServer::ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, TransportMgrBase * transportMgr,
                                                     SecureSessionMgr * sessionMgr, Transport::AdminPairingTable * admins,
                                                     SessionIDAllocator * idAllocator)
{
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(admins != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSessionMgr      = sessionMgr;
    mAdmins          = admins;
    mExchangeManager = exchangeManager;
    mIDAllocator     = idAllocator;

    ReturnErrorOnFailure(mPairingSession.MessageDispatch().Init(transportMgr));

    ExchangeDelegate * delegate = this;
    ReturnErrorOnFailure(
        mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_SigmaR1, delegate));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::InitCASEHandshake(Messaging::ExchangeContext * ec)
{
    ReturnErrorCodeIf(ec == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Cleanup();

    // TODO - Use PK of the root CA for the initiator to figure out the admin.
    mAdminId = ec->GetSecureSession().GetAdminId();

    // TODO - Use section [4.368] and definition of `Destination Identifier` to find admin ID for CASE SigmaR1 message
    //    ReturnErrorCodeIf(mAdminId == Transport::kUndefinedAdminId, CHIP_ERROR_INVALID_ARGUMENT);
    mAdminId = 0;

    Transport::AdminPairingInfo * admin = mAdmins->FindAdminWithId(mAdminId);

    if (admin == nullptr)
    {
        ReturnErrorOnFailure(mAdmins->LoadFromStorage(mAdminId));
        admin = mAdmins->FindAdminWithId(mAdminId);
    }
    ReturnErrorCodeIf(admin == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(admin->GetCredentials(mCredentials, mCertificates, mRootKeyId));

    ReturnErrorOnFailure(mIDAllocator->Allocate(mSessionKeyId));

    // Setup CASE state machine using the credentials for the current admin.
    ReturnErrorOnFailure(mPairingSession.ListenForSessionEstablishment(&mCredentials, mSessionKeyId, this));

    // Hand over the exchange context to the CASE session.
    ec->SetDelegate(&mPairingSession);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                         const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    ChipLogProgress(Inet, "CASE Server received SigmaR1 message. Starting handshake. EC %p", ec);
    ReturnErrorOnFailure(InitCASEHandshake(ec));

    mPairingSession.OnMessageReceived(ec, packetHeader, payloadHeader, std::move(payload));

    return CHIP_NO_ERROR;
    // TODO - Enable multiple concurrent CASE session establishment
}

void CASEServer::Cleanup()
{
    // Let's re-register for CASE SigmaR1 message, so that the next CASE session setup request can be processed.
    mAdminId = Transport::kUndefinedAdminId;
    mCredentials.Release();
    mCertificates.Release();
    mPairingSession.Clear();
}

void CASEServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogProgress(Inet, "CASE Session establishment failed: %s", ErrorStr(err));
    mIDAllocator->Free(mSessionKeyId);
    Cleanup();
}

void CASEServer::OnSessionEstablished()
{
    ChipLogProgress(Inet, "CASE Session established. Setting up the secure channel.");
    mSessionMgr->ExpireAllPairings(mPairingSession.PeerConnection().GetPeerNodeId(), mAdminId);

    CHIP_ERROR err = mSessionMgr->NewPairing(
        Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
        mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession, SecureSession::SessionRole::kResponder, mAdminId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed in setting up secure channel: err %s", ErrorStr(err));
        OnSessionEstablishmentError(err);
        return;
    }

    ChipLogProgress(Inet, "CASE secure channel is available now.");
    Cleanup();
}
} // namespace chip
