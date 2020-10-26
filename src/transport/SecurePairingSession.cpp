/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements the CHIP SPAKE2P Session object that provides
 *      APIs for constructing spake2p messages and establishing encryption
 *      keys.
 *
 *      The protocol for handling pA, pB, cB and cA is defined in SPAKE2
 *      Plus specifications.
 *      (https://www.ietf.org/id/draft-bar-cfrg-spake2plus-01.html)
 *
 */

#include <inttypes.h>

#include <core/CHIPSafeCasts.h>
#include <protocols/CHIPProtocols.h>
#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <transport/SecurePairingSession.h>

namespace chip {

using namespace Crypto;

const char * kSpake2pContext        = "CHIP 1.0 Provisioning";
const char * kSpake2pI2RSessionInfo = "Commissioning I2R Key";
const char * kSpake2pR2ISessionInfo = "Commissioning R2I Key";

SecurePairingSession::SecurePairingSession() {}

SecurePairingSession::~SecurePairingSession()
{
    memset(&mPoint[0], 0, sizeof(mPoint));
    memset(&mWS[0][0], 0, sizeof(mWS));
    memset(&mKe[0], 0, sizeof(mKe));
}

CHIP_ERROR SecurePairingSession::Serialize(SecurePairingSessionSerialized & output)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    const NodeId localNodeId = (mLocalNodeId.HasValue()) ? mLocalNodeId.Value() : kUndefinedNodeId;
    const NodeId peerNodeId  = (mPeerNodeId.HasValue()) ? mPeerNodeId.Value() : kUndefinedNodeId;
    VerifyOrExit(CanCastTo<uint16_t>(mKeLen), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(localNodeId), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(peerNodeId), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint16_t>(sizeof(SecurePairingSessionSerializable)), error = CHIP_ERROR_INTERNAL);

    {
        SecurePairingSessionSerializable serializable;
        memset(&serializable, 0, sizeof(serializable));
        serializable.mKeLen           = static_cast<uint16_t>(mKeLen);
        serializable.mPairingComplete = (mPairingComplete) ? 1 : 0;
        serializable.mLocalNodeId     = localNodeId;
        serializable.mPeerNodeId      = peerNodeId;
        serializable.mLocalKeyId      = mLocalKeyId;
        serializable.mPeerKeyId       = mPeerKeyId;

        memcpy(serializable.mKe, mKe, mKeLen);

        uint16_t serializedLen = 0;

        VerifyOrExit(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);

        serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                           static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
        VerifyOrExit(serializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(serializedLen < sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);
        output.inner[serializedLen] = '\0';
    }

exit:
    return error;
}

CHIP_ERROR SecurePairingSession::Deserialize(SecurePairingSessionSerialized & input)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    SecurePairingSessionSerializable serializable;
    size_t maxlen            = BASE64_ENCODED_LEN(sizeof(serializable));
    size_t len               = strnlen(Uint8::to_char(input.inner), maxlen);
    uint16_t deserializedLen = 0;

    VerifyOrExit(len < sizeof(SecurePairingSessionSerialized), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<uint16_t>(len), error = CHIP_ERROR_INVALID_ARGUMENT);

    memset(&serializable, 0, sizeof(serializable));
    deserializedLen =
        Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len), Uint8::to_uchar((uint8_t *) &serializable));
    VerifyOrExit(deserializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(deserializedLen <= sizeof(serializable), error = CHIP_ERROR_INVALID_ARGUMENT);

    mPairingComplete = (serializable.mPairingComplete == 1);
    mKeLen           = static_cast<size_t>(serializable.mKeLen);

    VerifyOrExit(mKeLen <= sizeof(mKe), error = CHIP_ERROR_INVALID_ARGUMENT);
    memset(mKe, 0, sizeof(mKe));
    memcpy(mKe, serializable.mKe, mKeLen);

    mLocalNodeId = Optional<NodeId>::Value(serializable.mLocalNodeId);
    mPeerNodeId  = Optional<NodeId>::Value(serializable.mPeerNodeId);

    mLocalKeyId = serializable.mLocalKeyId;
    mPeerKeyId  = serializable.mPeerKeyId;

exit:
    return error;
}

CHIP_ERROR SecurePairingSession::Init(uint32_t setupCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                                      Optional<NodeId> myNodeId, uint16_t myKeyId, SecurePairingSessionDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(salt != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(saltLen > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mSpake2p.Init(Uint8::from_const_char(kSpake2pContext), strlen(kSpake2pContext));
    SuccessOrExit(err);

    err = pbkdf2_sha256(reinterpret_cast<const uint8_t *>(&setupCode), sizeof(setupCode), salt, saltLen, pbkdf2IterCount,
                        sizeof(mWS), &mWS[0][0]);
    SuccessOrExit(err);

    mDelegate    = delegate;
    mLocalNodeId = myNodeId;
    mLocalKeyId  = myKeyId;

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt,
                                                size_t saltLen, Optional<NodeId> myNodeId, uint16_t myKeyId,
                                                SecurePairingSessionDelegate * delegate)
{
    size_t sizeof_point = sizeof(mPoint);

    CHIP_ERROR err = Init(mySetUpPINCode, pbkdf2IterCount, salt, saltLen, myNodeId, myKeyId, delegate);
    SuccessOrExit(err);

    err = mSpake2p.ComputeL(mPoint, &sizeof_point, &mWS[1][0], kSpake2p_WS_Length);
    SuccessOrExit(err);

    mNextExpectedMsg = Spake2pMsgType::kSpake2pCompute_pA;
    mPairingComplete = false;

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::AttachHeaderAndSend(uint8_t msgType, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PacketHeader packetHeader;
    PayloadHeader payloadHeader;

    packetHeader
        .SetSourceNodeId(mLocalNodeId) //
        .SetEncryptionKeyID(mLocalKeyId);

    payloadHeader
        .SetMessageType(msgType) //
        .SetProtocolID(Protocols::kChipProtocol_SecurePairing);

    uint16_t headerSize              = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = payloadHeader.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    headerSize              = packetHeader.EncodeSizeBytes();
    actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err =
        packetHeader.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize, payloadHeader.GetEncodePacketFlags());
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err    = mDelegate->SendMessage(msgBuf);
    msgBuf = nullptr;
    SuccessOrExit(err);

exit:
    if (msgBuf)
        System::PacketBuffer::Free(msgBuf);
    return err;
}

CHIP_ERROR SecurePairingSession::Pair(uint32_t peerSetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                                      Optional<NodeId> myNodeId, uint16_t myKeyId, SecurePairingSessionDelegate * delegate)
{
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    uint16_t data_len; // Will be the same as X_len in practice.

    System::PacketBuffer * resp = nullptr;

    CHIP_ERROR err = Init(peerSetUpPINCode, pbkdf2IterCount, salt, saltLen, myNodeId, myKeyId, delegate);
    SuccessOrExit(err);

    err = mSpake2p.BeginProver(reinterpret_cast<const uint8_t *>(""), 0, reinterpret_cast<const uint8_t *>(""), 0, &mWS[0][0],
                               kSpake2p_WS_Length, &mWS[1][0], kSpake2p_WS_Length);
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundOne(X, &X_len);
    SuccessOrExit(err);
    VerifyOrExit(CanCastTo<uint16_t>(X_len), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    data_len = static_cast<uint16_t>(X_len);

    resp = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrExit(resp != nullptr, err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), data_len);
        VerifyOrExit(bbuf.Put(&X[0], X_len) == X_len, err = CHIP_ERROR_NO_MEMORY);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(data_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pCompute_pB_cB;

    // Call delegate to send the Compute_pA to peer
    err  = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_pA, resp);
    resp = nullptr;
    SuccessOrExit(err);

    return err;

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;

    if (resp != nullptr)
    {
        System::PacketBuffer::Free(resp);
    }

    return err;
}

CHIP_ERROR SecurePairingSession::DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(info != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info_len > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mPairingComplete, err = CHIP_ERROR_INCORRECT_STATE);

    err = session.InitFromSecret(mKe, mKeLen, nullptr, 0, info, info_len);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::HandleCompute_pA(const PacketHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    uint16_t data_len; // To be initialized once we compute it.

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBuffer * resp = nullptr;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == kMAX_Point_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.BeginVerifier(reinterpret_cast<const uint8_t *>(""), 0, reinterpret_cast<const uint8_t *>(""), 0, &mWS[0][0],
                                 kSpake2p_WS_Length, mPoint, sizeof(mPoint));
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundOne(Y, &Y_len);
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundTwo(buf, buf_len, verifier, &verifier_len);
    SuccessOrExit(err);

    mPeerKeyId  = header.GetEncryptionKeyID();
    mPeerNodeId = header.GetSourceNodeId();

    // Make sure our addition doesn't overflow.
    VerifyOrExit(UINTMAX_MAX - verifier_len >= Y_len, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(CanCastTo<uint16_t>(Y_len + verifier_len), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    data_len = static_cast<uint16_t>(Y_len + verifier_len);

    resp = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrExit(resp != nullptr, err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), data_len);
        bbuf.Put(&Y[0], Y_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(data_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pCompute_cA;

    // Call delegate to send the Compute_pB_cB to peer
    err  = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_pB_cB, resp);
    resp = nullptr;
    SuccessOrExit(err);

    return err;

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;

    if (resp != nullptr)
    {
        System::PacketBuffer::Free(resp);
    }

    return err;
}

CHIP_ERROR SecurePairingSession::HandleCompute_pB_cB(const PacketHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len_raw = kMAX_Hash_Length;
    uint16_t verifier_len; // To be inited one we check length is small enough

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBuffer * resp = nullptr;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == kMAX_Point_Length + kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.ComputeRoundTwo(buf, kMAX_Point_Length, verifier, &verifier_len_raw);
    SuccessOrExit(err);
    VerifyOrExit(CanCastTo<uint16_t>(verifier_len_raw), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    verifier_len = static_cast<uint16_t>(verifier_len_raw);

    mPeerKeyId  = header.GetEncryptionKeyID();
    mPeerNodeId = header.GetSourceNodeId();

    resp = System::PacketBuffer::NewWithAvailableSize(verifier_len);
    VerifyOrExit(resp != nullptr, err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), verifier_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(verifier_len);

    // Call delegate to send the Compute_cA to peer
    err  = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_cA, resp);
    resp = nullptr;
    SuccessOrExit(err);

    {
        const uint8_t * hash = &buf[kMAX_Point_Length];
        err                  = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
        SuccessOrExit(err);

        err = mSpake2p.GetKeys(mKe, &mKeLen);
        SuccessOrExit(err);
    }

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;

    if (resp != nullptr)
    {
        System::PacketBuffer::Free(resp);
    }

    return err;
}

CHIP_ERROR SecurePairingSession::HandleCompute_cA(const PacketHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    const uint8_t * hash = msg->Start();

    VerifyOrExit(hash != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(msg->TotalLength() == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    VerifyOrExit(header.GetSourceNodeId() == mPeerNodeId, err = CHIP_ERROR_WRONG_NODE_ID);
    VerifyOrExit(header.GetEncryptionKeyID() == mPeerKeyId, err = CHIP_ERROR_INVALID_KEY_ID);

    err = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
    SuccessOrExit(err);

    err = mSpake2p.GetKeys(mKe, &mKeLen);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;
    return err;
}

CHIP_ERROR SecurePairingSession::HandlePeerMessage(const PacketHeader & packetHeader, System::PacketBuffer * msg)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint16_t headerSize = 0;
    PayloadHeader payloadHeader;

    VerifyOrExit(msg != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.Decode(packetHeader.GetFlags(), msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    VerifyOrExit(payloadHeader.GetProtocolID() == Protocols::kChipProtocol_SecurePairing, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    VerifyOrExit(payloadHeader.GetMessageType() == (uint8_t) mNextExpectedMsg, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    switch (static_cast<Spake2pMsgType>(payloadHeader.GetMessageType()))
    {
    case Spake2pMsgType::kSpake2pCompute_pA:
        err = HandleCompute_pA(packetHeader, msg);
        break;

    case Spake2pMsgType::kSpake2pCompute_pB_cB:
        err = HandleCompute_pB_cB(packetHeader, msg);
        break;

    case Spake2pMsgType::kSpake2pCompute_cA:
        err = HandleCompute_cA(packetHeader, msg);
        break;

    default:
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    };

exit:

    // Call delegate to indicate pairing failure
    if (err != CHIP_NO_ERROR)
    {
        mDelegate->OnPairingError(err);
    }
    else if (msg != nullptr)
    {
        // Free buffer only if we successfully processed it
        System::PacketBuffer::Free(msg);
    }

    return err;
}

} // namespace chip
