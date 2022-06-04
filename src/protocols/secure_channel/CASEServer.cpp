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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/SessionManager.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::Credentials;

namespace chip {

CHIP_ERROR CASEServer::ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, SessionManager * sessionManager,
                                                     FabricTable * fabrics, SessionResumptionStorage * sessionResumptionStorage,
                                                     Credentials::GroupDataProvider * responderGroupDataProvider)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(responderGroupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSessionManager           = sessionManager;
    mSessionResumptionStorage = sessionResumptionStorage;
    mFabrics                  = fabrics;
    mExchangeManager          = exchangeManager;
    mGroupDataProvider        = responderGroupDataProvider;

    // Setup CASE state machine using the credentials for the current fabric.
    GetSession().SetGroupDataProvider(mGroupDataProvider);

    Cleanup();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::InitCASEHandshake(Messaging::ExchangeContext * ec)
{
    ReturnErrorCodeIf(ec == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Hand over the exchange context to the CASE session.
    ec->SetDelegate(&GetSession());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // TODO: assign newDelegate to CASESession, let CASESession handle future messages.
    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                         System::PacketBufferHandle && payload)
{
    ChipLogProgress(Inet, "CASE Server received Sigma1 message. Starting handshake. EC %p", ec);
    CHIP_ERROR err = InitCASEHandshake(ec);
    SuccessOrExit(err);

    // TODO - Enable multiple concurrent CASE session establishment
    // https://github.com/project-chip/connectedhomeip/issues/8342
    ChipLogProgress(Inet, "CASE Server disabling CASE session setups");
    mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1);

    err = GetSession().OnMessageReceived(ec, payloadHeader, std::move(payload));
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Cleanup();
    }

    return err;
}

void CASEServer::Cleanup()
{
    // Let's re-register for CASE Sigma1 message, so that the next CASE session setup request can be processed.
    // https://github.com/project-chip/connectedhomeip/issues/8342
    ChipLogProgress(Inet, "CASE Server enabling CASE session setups");
    mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1, this);

    //
    // If we previously had a session to a peer, latch it its details. If we didn't, this will
    // contain kInvalidPeerNodeId.
    //
    ScopedNodeId previouslyEstablishedPeer = GetSession().GetPeer();

    GetSession().Clear();

    //
    // Indicate to the underlying CASE session to prepare for session establishment requests coming its way. This will
    // involve allocating a SecureSession that will be held until it's needed for the next CASE session handshake.
    //
    // Logically speaking, we're attempting to evict a session using details of the just-established session (to ensure
    // we're evicting sessions from the right fabric if needed) and then transferring the just established session into that
    // slot (and there-by free'ing up the slot for the next session attempt). However, this transfer isn't necessary - just
    // evicting a session will ensure it is available for the next attempt.
    //
    VerifyOrDie(GetSession().PrepareForSessionEstablishment(
                    *mSessionManager, mFabrics, mSessionResumptionStorage, this, previouslyEstablishedPeer,
                    Optional<ReliableMessageProtocolConfig>::Value(GetLocalMRPConfig())) == CHIP_NO_ERROR);

    //
    // PairingSession::mSecureSessionHolder is a weak-reference. If MarkForRemoval is called on this session, the session is
    // going to get de-allocated from underneath us. This session that has just been allocated should *never* get evicted, and
    // remain available till the next hand-shake is received.
    //
    // TODO: Converting SessionHolder to a true weak-ref and making PairingSession hold a strong-ref (#18397) would avoid this
    // headache...
    //
    // Let's create a SessionHandle strong-reference to it to keep it resident.
    //
    mCaseSession = GetSession().CopySecureSession();
    VerifyOrDie(mCaseSession.HasValue());
}

void CASEServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogError(Inet, "CASE Session establishment failed: %s", ErrorStr(err));
    Cleanup();
}

void CASEServer::OnSessionEstablished(const SessionHandle & session)
{
    ChipLogProgress(Inet, "CASE Session established to peer: " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(session->GetPeer()));
    Cleanup();
}
} // namespace chip
