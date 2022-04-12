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
#include <lib/core/CHIPTLV.h>
#include <messaging/ExchangeContext.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <transport/CryptoContext.h>
#include <transport/SecureSession.h>

namespace chip {

class DLL_EXPORT PairingSession
{
public:
    PairingSession(Transport::SecureSession::Type secureSessionType) : mSecureSessionType(secureSessionType) {}
    virtual ~PairingSession() {}

    Transport::SecureSession::Type GetSecureSessionType() const { return mSecureSessionType; }

    // TODO: the session should know which peer we are trying to connect to at start
    // mPeerNodeId should be const and assigned at the construction, such that GetPeerNodeId will never return kUndefinedNodeId, and
    // SetPeerNodeId is not necessary.
    NodeId GetPeerNodeId() const { return mPeerNodeId; }

    CATValues GetPeerCATs() const { return mPeerCATs; }

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

    auto GetSecureSessionHandle() const { return mSecureSessionHolder.ToOptional(); }

    uint16_t GetPeerSessionId() const
    {
        VerifyOrDie(mPeerSessionId.HasValue());
        return mPeerSessionId.Value();
    }
    bool IsValidPeerSessionId() const { return mPeerSessionId.HasValue(); }

    // TODO: decouple peer address into transport, such that pairing session do not need to handle peer address
    const Transport::PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    Transport::PeerAddress & GetPeerAddress() { return mPeerAddress; }

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param session     Reference to the secure session that will be
     *                    initialized once pairing is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role) = 0;

    /**
     * @brief
     *   Get the MRP config that was communicated during the session establishment.
     */
    virtual const ReliableMessageProtocolConfig & GetMRPConfig() const { return mMRPConfig; }

    void SetMRPConfig(const ReliableMessageProtocolConfig & config) { mMRPConfig = config; }

    /**
     * Encode the provided MRP parameters using the provided TLV tag.
     */
    static CHIP_ERROR EncodeMRPParameters(TLV::Tag tag, const ReliableMessageProtocolConfig & mrpConfig,
                                          TLV::TLVWriter & tlvWriter);

protected:
    /**
     * Allocate a secure session object from the passed session manager for the
     * pending session establishment operation.
     *
     * @param sessionManager session manager from which to allocate a secure session object
     * @return CHIP_ERROR The outcome of the allocation attempt
     */
    CHIP_ERROR AllocateSecureSession(SessionManager & sessionManager);

    /**
     * Allocate a secure session object from the passed session manager with the
     * specified session ID.
     *
     * This variant of the interface may be used in test scenarios where
     * session IDs need to be predetermined.

     * @param sessionManager session manager from which to allocate a secure session object
     * @param sessionId caller-requested session ID
     * @return CHIP_ERROR The outcome of the allocation attempt
     */
    CHIP_ERROR AllocateSecureSession(SessionManager & sessionManager, uint16_t sessionId);

    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }
    void SetPeerSessionId(uint16_t id) { mPeerSessionId.SetValue(id); }
    void SetPeerAddress(const Transport::PeerAddress & address) { mPeerAddress = address; }
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
        uint32_t protocolId = Protocols::SecureChannel::Id.ToFullyQualifiedSpecForm();

        ChipLogDetail(SecureChannel, "Sending status report. Protocol code %d, exchange %d", protocolCode,
                      exchangeCtxt->GetExchangeId());

        Protocols::SecureChannel::StatusReport statusReport(generalCode, protocolId, protocolCode);

        Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(statusReport.Size()));
        statusReport.WriteToBuffer(bbuf);

        System::PacketBufferHandle msg = bbuf.Finalize();
        VerifyOrReturn(!msg.IsNull(), ChipLogError(SecureChannel, "Failed to allocate status report message"));

        CHIP_ERROR err = exchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::StatusReport, std::move(msg));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel, "Failed to send status report message. %s", ErrorStr(err));
        }
    }

    CHIP_ERROR HandleStatusReport(System::PacketBufferHandle && msg, bool successExpected)
    {
        Protocols::SecureChannel::StatusReport report;
        CHIP_ERROR err = report.Parse(std::move(msg));
        ReturnErrorOnFailure(err);
        VerifyOrReturnError(report.GetProtocolId() == Protocols::SecureChannel::Id.ToFullyQualifiedSpecForm(),
                            CHIP_ERROR_INVALID_ARGUMENT);

        if (report.GetGeneralCode() == Protocols::SecureChannel::GeneralStatusCode::kSuccess &&
            report.GetProtocolCode() == Protocols::SecureChannel::kProtocolCodeSuccess && successExpected)
        {
            OnSuccessStatusReport();
        }
        else
        {
            err = OnFailureStatusReport(report.GetGeneralCode(), report.GetProtocolCode());
        }

        return err;
    }

    /**
     * Try to decode the current element (pointed by the TLV reader) as MRP parameters.
     * If the MRP parameters are found, mMRPConfig is updated with the devoded values.
     *
     * MRP parameters are optional. So, if the TLV reader is not pointing to the MRP parameters,
     * the function is a noop.
     *
     * If the parameters are present, but TLV reader fails to correctly parse it, the function will
     * return the corresponding error.
     */
    CHIP_ERROR DecodeMRPParametersIfPresent(TLV::Tag expectedTag, TLV::ContiguousBufferTLVReader & tlvReader);

    // TODO: remove Clear, we should create a new instance instead reset the old instance.
    void Clear()
    {
        mPeerNodeId  = kUndefinedNodeId;
        mPeerCATs    = kUndefinedCATs;
        mPeerAddress = Transport::PeerAddress::Uninitialized();
        mPeerSessionId.ClearValue();
        mSecureSessionHolder.Release();
    }

private:
    const Transport::SecureSession::Type mSecureSessionType;

protected:
    NodeId mPeerNodeId = kUndefinedNodeId;
    CATValues mPeerCATs;

private:
    SessionHolder mSecureSessionHolder;

    // TODO: decouple peer address into transport, such that pairing session do not need to handle peer address
    Transport::PeerAddress mPeerAddress = Transport::PeerAddress::Uninitialized();

    Optional<uint16_t> mPeerSessionId;

    ReliableMessageProtocolConfig mMRPConfig = GetLocalMRPConfig();
};

} // namespace chip
