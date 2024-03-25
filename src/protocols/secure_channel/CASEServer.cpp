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
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <tracing/macros.h>
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

    ChipLogProgress(Inet, "CASE Server enabling CASE session setups");
    mExchangeManager->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1, this);

    PrepareForSessionEstablishment();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASEServer::InitCASEHandshake(Messaging::ExchangeContext * ec)
{
    MATTER_TRACE_SCOPE("InitCASEHandshake", "CASEServer");
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
    MATTER_TRACE_SCOPE("OnMessageReceived", "CASEServer");

    bool busy = GetSession().GetState() != CASESession::State::kInitialized;
    CHIP_FAULT_INJECT(FaultInjection::kFault_CASEServerBusy, busy = true);
    if (busy)
    {
        // We are in the middle of CASE handshake

        // Invoke watchdog to fix any stuck handshakes
        bool watchdogFired = GetSession().InvokeBackgroundWorkWatchdog();
        if (!watchdogFired)
        {
            // Handshake wasn't stuck, send the busy status report and let the existing handshake continue.

            // A successful CASE handshake can take several seconds and some may time out (30 seconds or more).

            System::Clock::Milliseconds16 delay = System::Clock::kZero;
            if (GetSession().GetState() == CASESession::State::kSentSigma2)
            {
                // The delay should be however long we think it will take for
                // that to time out.
                auto sigma2Timeout = CASESession::ComputeSigma2ResponseTimeout(GetSession().GetRemoteMRPConfig());
                if (sigma2Timeout < System::Clock::Milliseconds16::max())
                {
                    delay = std::chrono::duration_cast<System::Clock::Milliseconds16>(sigma2Timeout);
                }
                else
                {
                    // Avoid overflow issues, just wait for as long as we can to
                    // get close to our expected Sigma2 timeout.
                    delay = System::Clock::Milliseconds16::max();
                }
            }
            else
            {
                // For now, setting minimum wait time to 5000 milliseconds if we
                // have no other information.
                delay = System::Clock::Milliseconds16(5000);
            }
            CHIP_ERROR err = SendBusyStatusReport(ec, delay);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Inet, "Failed to send the busy status report, err:%" CHIP_ERROR_FORMAT, err.Format());
            }
            return err;
        }
    }

    if (!ec->GetSessionHandle()->IsUnauthenticatedSession())
    {
        ChipLogError(Inet, "CASE Server received Sigma1 message %s EC %p", "over encrypted session. Ignoring.", ec);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ChipLogProgress(Inet, "CASE Server received Sigma1 message %s EC %p", ". Starting handshake.", ec);

    CHIP_ERROR err = InitCASEHandshake(ec);
    SuccessOrExit(err);

    // TODO - Enable multiple concurrent CASE session establishment
    // https://github.com/project-chip/connectedhomeip/issues/8342

    err = GetSession().OnMessageReceived(ec, payloadHeader, std::move(payload));
    SuccessOrExit(err);

exit:
    // CASESession::OnMessageReceived guarantees that it will call
    // OnSessionEstablishmentError if it returns error, so nothing else to do here.
    return err;
}

void CASEServer::PrepareForSessionEstablishment(const ScopedNodeId & previouslyEstablishedPeer)
{
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
    MATTER_TRACE_SCOPE("OnSessionEstablishmentError", "CASEServer");
    ChipLogError(Inet, "CASE Session establishment failed: %" CHIP_ERROR_FORMAT, err.Format());

    MATTER_TRACE_SCOPE("CASEFail", "CASESession");
    PrepareForSessionEstablishment();
}

void CASEServer::OnSessionEstablished(const SessionHandle & session)
{
    MATTER_TRACE_SCOPE("OnSessionEstablished", "CASEServer");
    ChipLogProgress(Inet, "CASE Session established to peer: " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(session->GetPeer()));
    PrepareForSessionEstablishment(session->GetPeer());
}

CHIP_ERROR CASEServer::SendBusyStatusReport(Messaging::ExchangeContext * ec, System::Clock::Milliseconds16 minimumWaitTime)
{
    MATTER_TRACE_SCOPE("SendBusyStatusReport", "CASEServer");
    ChipLogProgress(Inet, "Already in the middle of CASE handshake, sending busy status report");

    System::PacketBufferHandle handle = Protocols::SecureChannel::StatusReport::MakeBusyStatusReportMessage(minimumWaitTime);
    VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);

    ChipLogProgress(Inet, "Sending status report, exchange " ChipLogFormatExchange, ChipLogValueExchange(ec));
    return ec->SendMessage(Protocols::SecureChannel::MsgType::StatusReport, std::move(handle));
}

} // namespace chip
