/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

/**
 *    @file
 *      This file defines a common interface to access various types of secure
 *      pairing sessions (e.g. PASE, CASE)
 *
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <messaging/ExchangeContext.h>
#include <messaging/SessionParameters.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>
#include <protocols/secure_channel/StatusReport.h>
#include <transport/CryptoContext.h>
#include <transport/SecureSession.h>

namespace chip {

class SessionManager;

class DLL_EXPORT PairingSession : public SessionDelegate
{
public:
    PairingSession() : mSecureSessionHolder(*this) {}
    virtual ~PairingSession() { Clear(); }

    virtual Transport::SecureSession::Type GetSecureSessionType() const = 0;
    virtual ScopedNodeId GetPeer() const                                = 0;
    virtual ScopedNodeId GetLocalScopedNodeId() const                   = 0;
    virtual CATValues GetPeerCATs() const                               = 0;

    // Implement SessionDelegate
    NewSessionHandlingPolicy GetNewSessionHandlingPolicy() override { return NewSessionHandlingPolicy::kStayAtOldSession; }
    void OnSessionReleased() override;

    Optional<uint16_t> GetLocalSessionId() const
    {
        Optional<uint16_t> localSessionId;
        VerifyOrExit(mSecureSessionHolder, localSessionId = NullOptional);
        VerifyOrExit(mSecureSessionHolder->GetSessionType() == Transport::Session::SessionType::kSecure,
                     localSessionId = Optional<uint16_t>::Missing());
        localSessionId.SetValue(mSecureSessionHolder->AsSecureSession()->GetLocalSessionId());
    exit:
        return localSessionId;
    }

    /**
     * Copy the underlying session (if present) into a SessionHandle that a caller can use to
     * obtain a reference to the session.
     */
    Optional<SessionHandle> CopySecureSession()
    {
        if (mSecureSessionHolder)
        {
            VerifyOrDie(mSecureSessionHolder->GetSessionType() == Transport::Session::SessionType::kSecure);
            return MakeOptional<SessionHandle>(*mSecureSessionHolder->AsSecureSession());
        }

        return Optional<SessionHandle>::Missing();
    }

    uint16_t GetPeerSessionId() const
    {
        VerifyOrDie(mPeerSessionId.HasValue());
        return mPeerSessionId.Value();
    }

    bool IsValidPeerSessionId() const { return mPeerSessionId.HasValue(); }

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error if called before pairing is established.
     *
     * @param session     Reference to the secure session that will be initialized once pairing is complete
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(CryptoContext & session) const = 0;

    const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const { return mRemoteSessionParams.GetMRPConfig(); }
    const SessionParameters & GetRemoteSessionParameters() const { return mRemoteSessionParams; }
    void SetRemoteMRPConfig(const ReliableMessageProtocolConfig & config) { mRemoteSessionParams.SetMRPConfig(config); }

    /**
     * Encode the Session Parameters using the provided TLV tag.
     */
    static CHIP_ERROR EncodeSessionParameters(TLV::Tag tag, const Optional<ReliableMessageProtocolConfig> & mrpLocalConfig,
                                              TLV::TLVWriter & tlvWriter);

protected:
    /**
     * Allocate a secure session object from the passed session manager for the
     * pending session establishment operation.
     *
     * @param sessionManager        Session manager from which to allocate a secure session object
     * @param sessionEvictionHint   If we're either establishing or just finished establishing a session to a peer in either
     * initiator or responder roles, the node id of that peer should be provided in this argument. Else, it should be initialized to
     * a default-constructed ScopedNodeId().
     *
     * @return CHIP_ERROR The outcome of the allocation attempt
     */
    CHIP_ERROR AllocateSecureSession(SessionManager & sessionManager, const ScopedNodeId & sessionEvictionHint = ScopedNodeId());

    CHIP_ERROR ActivateSecureSession(const Transport::PeerAddress & peerAddress);

    void Finish();

    void DiscardExchange(); // Clear our reference to our exchange context pointer so that it can close itself at some later time.

    void SetPeerSessionId(uint16_t id) { mPeerSessionId.SetValue(id); }
    virtual void OnSuccessStatusReport() {}
    virtual CHIP_ERROR OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode)
    {
        return CHIP_ERROR_INTERNAL;
    }

    void SendStatusReport(Messaging::ExchangeContext * exchangeCtxt, uint16_t protocolCode)
    {
        Protocols::SecureChannel::GeneralStatusCode generalCode = (protocolCode == Protocols::SecureChannel::kProtocolCodeSuccess)
            ? Protocols::SecureChannel::GeneralStatusCode::kSuccess
            : Protocols::SecureChannel::GeneralStatusCode::kFailure;

        ChipLogDetail(SecureChannel, "Sending status report. Protocol code %d, exchange %d", protocolCode,
                      exchangeCtxt->GetExchangeId());

        Protocols::SecureChannel::StatusReport statusReport(generalCode, Protocols::SecureChannel::Id, protocolCode);

        auto handle = System::PacketBufferHandle::New(statusReport.Size());
        VerifyOrReturn(!handle.IsNull(), ChipLogError(SecureChannel, "Failed to allocate status report message"));
        Encoding::LittleEndian::PacketBufferWriter bbuf(std::move(handle));

        statusReport.WriteToBuffer(bbuf);

        System::PacketBufferHandle msg = bbuf.Finalize();
        VerifyOrReturn(!msg.IsNull(), ChipLogError(SecureChannel, "Failed to allocate status report message"));

        CHIP_ERROR err = exchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::StatusReport, std::move(msg));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel, "Failed to send status report message: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    CHIP_ERROR HandleStatusReport(System::PacketBufferHandle && msg, bool successExpected)
    {
        Protocols::SecureChannel::StatusReport report;
        ReturnErrorOnFailure(report.Parse(std::move(msg)));
        VerifyOrReturnError(report.GetProtocolId() == Protocols::SecureChannel::Id, CHIP_ERROR_INVALID_ARGUMENT);

        if (report.GetGeneralCode() == Protocols::SecureChannel::GeneralStatusCode::kSuccess &&
            report.GetProtocolCode() == Protocols::SecureChannel::kProtocolCodeSuccess && successExpected)
        {
            OnSuccessStatusReport();
            return CHIP_NO_ERROR;
        }

        if (report.GetGeneralCode() == Protocols::SecureChannel::GeneralStatusCode::kBusy &&
            report.GetProtocolCode() == Protocols::SecureChannel::kProtocolCodeBusy)
        {
            if (!report.GetProtocolData().IsNull())
            {
                Encoding::LittleEndian::Reader reader(report.GetProtocolData()->Start(), report.GetProtocolData()->DataLength());

                uint16_t minimumWaitTime = 0;
                CHIP_ERROR waitTimeErr   = reader.Read16(&minimumWaitTime).StatusCode();
                if (waitTimeErr != CHIP_NO_ERROR)
                {
                    ChipLogError(SecureChannel, "Failed to read the minimum wait time: %" CHIP_ERROR_FORMAT, waitTimeErr.Format());
                }
                else
                {
                    // TODO: CASE: Notify minimum wait time to clients on receiving busy status report #28290
                    ChipLogProgress(SecureChannel, "Received busy status report with minimum wait time: %u ms", minimumWaitTime);
                }
            }
        }

        // It's very important that we propagate the return value from
        // OnFailureStatusReport out to the caller.  Make sure we return it directly.
        return OnFailureStatusReport(report.GetGeneralCode(), report.GetProtocolCode());
    }

    /**
     * Try to decode the current element (pointed by the TLV reader) as MRP parameters.
     * If the MRP parameters are found, mRemoteSessionParams is updated with the devoded values.
     *
     * MRP parameters are optional. So, if the TLV reader is not pointing to the MRP parameters,
     * the function is a noop.
     *
     * If the parameters are present, but TLV reader fails to correctly parse it, the function will
     * return the corresponding error.
     */
    CHIP_ERROR DecodeMRPParametersIfPresent(TLV::Tag expectedTag, TLV::ContiguousBufferTLVReader & tlvReader);

    bool IsSessionEstablishmentInProgress();

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    void Clear();

    /**
     * Notify our delegate about a session establishment error and the stage when the error occurs
     * if we have not already notified it of an error or success before.
     *
     * @param error The error code to report.
     * @param stage The stage of the session when the error occurs, defaults to kNotInKeyExchange.
     */
    void NotifySessionEstablishmentError(CHIP_ERROR error,
                                         SessionEstablishmentStage stage = SessionEstablishmentStage::kNotInKeyExchange);

protected:
    CryptoContext::SessionRole mRole;
    SessionHolderWithDelegate mSecureSessionHolder;
    // mSessionManager is set if we actually allocate a secure session, so we
    // can clean it up later as needed.
    SessionManager * mSessionManager           = nullptr;
    Messaging::ExchangeContext * mExchangeCtxt = nullptr;
    SessionEstablishmentDelegate * mDelegate   = nullptr;

    // mLocalMRPConfig is our config which is sent to the other end and used by the peer session.
    // mRemoteSessionParams is received from other end and set to our session.
    Optional<ReliableMessageProtocolConfig> mLocalMRPConfig;
    SessionParameters mRemoteSessionParams;

private:
    Optional<uint16_t> mPeerSessionId;
};

} // namespace chip
