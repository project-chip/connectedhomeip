/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements the the CHIP CASE Session object that provides
 *      APIs for constructing secure session using certificate from device's
 *      operational credentials.
 *
 */
#include <transport/CertificatePairingSession.h>

#include <inttypes.h>
#include <string.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <protocols/Protocols.h>
#include <support/BufBound.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

using namespace Crypto;

CertificatePairingSession::CertificatePairingSession() {}

CertificatePairingSession::~CertificatePairingSession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void CertificatePairingSession::Clear()
{
    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;
    mLocalNodeId     = kUndefinedNodeId;
    mPairingComplete = false;
    mConnectionState.Reset();
}

CHIP_ERROR CertificatePairingSession::WaitForPairing(NodeId myNodeId, uint16_t myKeyId, PairingSessionDelegate * delegate)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR CertificatePairingSession::AttachHeaderAndSend(Protocols::SecureChannel::MsgType msgType,
                                                          System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PayloadHeader payloadHeader;

    payloadHeader
        .SetMessageType(static_cast<uint8_t>(msgType)) //
        .SetProtocolID(Protocols::kProtocol_SecureChannel);

    uint16_t headerSize              = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = payloadHeader.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err = mDelegate->SendPairingMessage(PacketHeader()
                                            .SetSourceNodeId(mLocalNodeId)
                                            .SetDestinationNodeId(mConnectionState.GetPeerNodeId())
                                            .SetEncryptionKeyID(mConnectionState.GetLocalKeyID()),
                                        mConnectionState.GetPeerAddress(), std::move(msgBuf));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CertificatePairingSession::Pair(const Transport::PeerAddress peerAddress, NodeId myNodeId, NodeId peerNodeId,
                                           uint16_t myKeyId, PairingSessionDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err);

    mConnectionState.SetPeerAddress(peerAddress);
    mConnectionState.SetPeerNodeId(peerNodeId);

    err = SendSigmaR1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR CertificatePairingSession::DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(info != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info_len > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mPairingComplete, err = CHIP_ERROR_INCORRECT_STATE);

    // ToDo: Use asymmetric keypair to create the secure session
    // err = session.Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CertificatePairingSession::SendSigmaR1()
{
    uint16_t data_len = 0;

    System::PacketBufferHandle msg_R1;

    msg_R1 = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_SYSTEM_ERROR_NO_MEMORY);

    // ToDo: Construct SigmaR1 message in form of the following structure
    /*
    struct SigmaR1
    {
        uint8_t random[32];
        uint8_t sessionid[2];
        TrustedRootIdentifier trusted_roots[];
        CipherSuite cipher_suites[];
        KeyShare key_shares[];
        Extensions extensions[];
    };
    */

    msg_R1->SetDataLength(data_len);
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR2;

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR1, std::move(msg_R1)));

    ChipLogDetail(Inet, "Sent SigmaR1 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CertificatePairingSession::HandleSigmaR1_and_SendSigmaR2(const PacketHeader & header,
                                                                    const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t data_len = 0;

    const uint8_t * buf = msg->Start();

    System::PacketBufferHandle msg_R2;

    ChipLogDetail(Inet, "Received SigmaR1 msg");

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    // ToDo: Verify received SigmaR1 message.

    mConnectionState.SetPeerKeyID(header.GetEncryptionKeyID());

    msg_R2 = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrExit(!msg_R2.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    // ToDo: Construct SigmaR2 message in form of the following structure
    /*
    struct SigmaR2Encrypted
    {
        opaque responder_identity;
        opaque signature;
    };

    struct SigmaR2
    {
        uint8_t[32] random;
        uint8_t sessionid[2];
        TrustedRootIdentifier trusted_root;
        CipherSuite cipher_suite;
        KeyShare key_share;
        SigmaR2Encrypted encrypted_r2;
    };
    */

    msg_R2->SetDataLength(data_len);
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR3;

    // Call delegate to send the msg to peer
    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR2, std::move(msg_R2));
    SuccessOrExit(err);

    ChipLogDetail(Inet, "Sent SigmaR2 msg");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CertificatePairingSession::HandleSigmaR2_and_SendSigmaR3(const PacketHeader & header,
                                                                    const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t data_len = 0;

    const uint8_t * buf = msg->Start();

    System::PacketBufferHandle msg_R3;

    ChipLogDetail(Inet, "Received SigmaR2 msg");

    // ToDo: Verify received SigmaR2 message.

    mConnectionState.SetPeerKeyID(header.GetEncryptionKeyID());

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    msg_R3 = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    // ToDo: Construct SigmaR3 message in form of the following structure
    /*
    struct SigmaR3Encrypted
    {
        opaque initiator_identity;
        opaque signature;
    };

    struct SigmaR3
    {
        SigmaR3Encrypted encrypted_r3;
    };
    */

    msg_R3->SetDataLength(data_len);

    // Call delegate to send the Msg3 to peer
    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR3, std::move(msg_R3));
    SuccessOrExit(err);

    ChipLogDetail(Inet, "Sent SigmaR3 msg");

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CertificatePairingSession::HandleSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(Inet, "Received SigmaR3 msg");

    // ToDo: Verify received SigmaR3 message.

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    VerifyOrExit(header.GetSourceNodeId().ValueOr(kUndefinedNodeId) == mConnectionState.GetPeerNodeId(),
                 err = CHIP_ERROR_WRONG_NODE_ID);
    VerifyOrExit(header.GetEncryptionKeyID() == mConnectionState.GetPeerKeyID(), err = CHIP_ERROR_INVALID_KEY_ID);

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

void CertificatePairingSession::SendErrorMsg(SigmaErrorType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg;
    uint16_t msglen      = sizeof(SigmaErrorMsg);
    SigmaErrorMsg * pMsg = nullptr;

    msg = System::PacketBuffer::NewWithAvailableSize(msglen);
    VerifyOrExit(!msg.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    pMsg        = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    pMsg->error = errorCode;

    msg->SetDataLength(msglen);

    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaErr, std::move(msg));
    SuccessOrExit(err);

exit:
    Clear();
}

void CertificatePairingSession::HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    // Error message processing
    const uint8_t * buf  = msg->Start();
    size_t buflen        = msg->DataLength();
    SigmaErrorMsg * pMsg = nullptr;

    VerifyOrExit(buf != nullptr, ChipLogError(Inet, "Null error msg received during pairing"));
    VerifyOrExit(buflen == sizeof(SigmaErrorMsg), ChipLogError(Inet, "Error msg with incorrect length received during pairing"));

    pMsg = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    ChipLogError(Inet, "Received error (%d) during CASE pairing process", pMsg->error);

exit:
    Clear();
}

CHIP_ERROR CertificatePairingSession::HandlePeerMessage(const PacketHeader & packetHeader,
                                                        const Transport::PeerAddress & peerAddress, System::PacketBufferHandle msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.Decode(msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    VerifyOrExit(payloadHeader.GetProtocolID() == Protocols::kProtocol_SecureChannel, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    VerifyOrExit(payloadHeader.GetMessageType() == (uint8_t) mNextExpectedMsg, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mConnectionState.SetPeerAddress(peerAddress);
    VerifyOrExit(mLocalNodeId == packetHeader.GetDestinationNodeId().Value(), err = CHIP_ERROR_WRONG_NODE_ID);

    switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
    {
    case Protocols::SecureChannel::MsgType::CASE_SigmaR1:
        err = HandleSigmaR1_and_SendSigmaR2(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR2:
        err = HandleSigmaR2_and_SendSigmaR3(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR3:
        err = HandleSigmaR3(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaErr:
        HandleErrorMsg(packetHeader, msg);
        break;

    default:
        SendErrorMsg(SigmaErrorType::kUnexpected);
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    };

exit:

    // Call delegate to indicate pairing failure
    if (err != CHIP_NO_ERROR)
    {
        mDelegate->OnPairingError(err);
    }

    return err;
}

} // namespace chip
