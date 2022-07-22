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
                                                     Credentials::CertificateValidityPolicy * certificateValidityPolicy,
                                                     Credentials::GroupDataProvider * responderGroupDataProvider)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(responderGroupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSessionManager            = sessionManager;
    mSessionResumptionStorage  = sessionResumptionStorage;
    mCertificateValidityPolicy = certificateValidityPolicy;
    mFabrics                   = fabrics;
    mExchangeManager           = exchangeManager;
    mGroupDataProvider         = responderGroupDataProvider;

    // Set up the group state provider that persists across all handshakes.
    GetSession().SetGroupDataProvider(mGroupDataProvider);

    PrepareForSessionEstablishment();

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
        PrepareForSessionEstablishment();
    }

    return err;
}

void CASEServer::PrepareForSessionEstablishment(const ScopedNodeId & previouslyEstablishedPeer)
{
    // Let's re-register for CASE Sigma1 message, so that the next CASE session setup request can be processed.
    // https://github.com/project-chip/connectedhomeip/issues/8342
    ChipLogProgress(Inet, "CASE Server enabling CASE session setups");
    mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1, this);

    GetSession().Clear();

    //
    // This releases our reference to a previously pinned session. If that was a successfully established session and is now
    // active, this will have no effect (the session will remain in the session table).
    //
    // If we previously held a session still in the pairing state, it means PairingSession was owning that session. Since it
    // gave up its reference by the time we got here, releasing the pinned session here will actually result in it being
    // de-allocated since no one else is holding onto this session. This will mean that when we get to allocating a session below,
    // we'll at least have one free session available in the session table, and won't need to evict an arbitrary session.
    //
    mPinnedSecureSession.ClearValue();

    //
    // Indicate to the underlying CASE session to prepare for session establishment requests coming its way. This will
    // involve allocating a SecureSession that will be held until it's needed for the next CASE session handshake.
    //
    // Logically speaking, we're attempting to evict a session using details of the just-established session (to ensure
    // we're evicting sessions from the right fabric if needed) and then transferring the just established session into that
    // slot (and thereby free'ing up the slot for the next session attempt). However, this transfer isn't necessary - just
    // evicting a session will ensure it is available for the next attempt.
    //
    // This call can fail if we have run out memory to allocate SecureSessions. Continuing without taking any action
    // however will render this node deaf to future handshake requests, so it's better to die here to raise attention to the problem
    // / facilitate recovery.
    //
    // TODO(#17568): Once session eviction is actually in place, this call should NEVER fail and if so, is a logic bug.
    // Dying here on failure is even more appropriate then.
    //
    VerifyOrDie(GetSession().PrepareForSessionEstablishment(*mSessionManager, mFabrics, mSessionResumptionStorage,
                                                            mCertificateValidityPolicy, this, previouslyEstablishedPeer,
                                                            GetLocalMRPConfig()) == CHIP_NO_ERROR);

    //
    // PairingSession::mSecureSessionHolder is a weak-reference. If MarkForEviction is called on this session, the session is
    // going to get de-allocated from underneath us. This session that has just been allocated should *never* get evicted, and
    // remain available till the next hand-shake is received.
    //
    // TODO: Converting SessionHolder to a true weak-ref and making PairingSession hold a strong-ref (#18397) would avoid this
    // headache...
    //
    // Let's create a SessionHandle strong-reference to it to keep it resident.
    //
    mPinnedSecureSession = GetSession().CopySecureSession();

    //
    // If we've gotten this far, it means we have successfully allocated a SecureSession to back our next attempt. If we haven't,
    // there is a bug somewhere and we should raise attention to it by dying.
    //
    VerifyOrDie(mPinnedSecureSession.HasValue());
}

void CASEServer::OnSessionEstablishmentError(CHIP_ERROR err)
{
    ChipLogError(Inet, "CASE Session establishment failed: %" CHIP_ERROR_FORMAT, err.Format());

    //
    // We're not allowed to call methods that will eventually result in calling SessionManager::AllocateSecureSession
    // from a SessionDelegate::OnSessionReleased callback. Schedule the preparation as an async work item.
    //
    mSessionManager->SystemLayer()->ScheduleWork(
        [](auto * systemLayer, auto * appState) -> void {
            CASEServer * _this = static_cast<CASEServer *>(appState);
            _this->PrepareForSessionEstablishment();
        },
        this);
}

void CASEServer::OnSessionEstablished(const SessionHandle & session)
{
    ChipLogProgress(Inet, "CASE Session established to peer: " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(session->GetPeer()));
    PrepareForSessionEstablishment(session->GetPeer());
}
} // namespace chip
