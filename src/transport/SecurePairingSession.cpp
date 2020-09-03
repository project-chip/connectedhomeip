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

#include <core/CHIPSafeCasts.h>
#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <transport/SecurePairingSession.h>

namespace chip {

using namespace Crypto;

const char * kSpake2pContext        = "CHIP 1.0 Provisioning";
const char * kSpake2pI2RSessionInfo = "Commissioning I2R Key";
const char * kSpake2pR2ISessionInfo = "Commissioning R2I Key";

SecurePairingSession::SecurePairingSession(void) {}

SecurePairingSession::~SecurePairingSession(void)
{
    if (mDelegate != nullptr)
    {
        mDelegate->Release();
    }
    memset(&mPoint[0], 0, sizeof(mPoint));
    memset(&mWS[0][0], 0, sizeof(mWS));
    memset(&mKe[0], 0, sizeof(mKe));
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

    if (mDelegate != nullptr)
    {
        mDelegate->Release();
    }
    mDelegate    = delegate->Retain();
    mLocalNodeId = myNodeId;
    mKeyId       = myKeyId;

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

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::AttachHeaderAndSend(uint8_t msgType, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    MessageHeader header;
    header.SetMessageType(msgType);
    header.SetProtocolID(kSecurePairingProtocol);
    header.SetSourceNodeId(mLocalNodeId);
    header.SetEncryptionKeyID(mKeyId);

    size_t headerSize              = header.EncryptedHeaderSizeBytes();
    size_t actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = header.EncodeEncryptedHeader(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);

    headerSize              = header.EncodeSizeBytes();
    actualEncodedHeaderSize = 0;

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = header.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);

    // This is unexpected and means header changed while encoding
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err = mDelegate->OnNewMessageForPeer(msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::Pair(uint32_t peerSetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen,
                                      Optional<NodeId> myNodeId, uint16_t myKeyId, SecurePairingSessionDelegate * delegate)
{
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);

    System::PacketBuffer * resp = nullptr;

    CHIP_ERROR err = Init(peerSetUpPINCode, pbkdf2IterCount, salt, saltLen, myNodeId, myKeyId, delegate);
    SuccessOrExit(err);

    err = mSpake2p.BeginProver((const uint8_t *) "", 0, (const uint8_t *) "", 0, &mWS[0][0], kSpake2p_WS_Length, &mWS[1][0],
                               kSpake2p_WS_Length);
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundOne(X, &X_len);
    SuccessOrExit(err);

    resp = System::PacketBuffer::NewWithAvailableSize(X_len);
    VerifyOrExit(resp != nullptr, err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), X_len);
        VerifyOrExit(bbuf.Put(&X[0], X_len) == X_len, err = CHIP_ERROR_NO_MEMORY);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(X_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pCompute_pB_cB;

    // Call delegate to send the Compute_pA to peer
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_pA, resp);
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

CHIP_ERROR SecurePairingSession::HandleCompute_pA(const MessageHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBuffer * resp = nullptr;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == kMAX_Point_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.BeginVerifier((const uint8_t *) "", 0, (const uint8_t *) "", 0, &mWS[0][0], kSpake2p_WS_Length, mPoint,
                                 sizeof(mPoint));
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundOne(Y, &Y_len);
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundTwo(buf, buf_len, verifier, &verifier_len);
    SuccessOrExit(err);

    mPeerKeyId  = header.GetEncryptionKeyID();
    mPeerNodeId = header.GetSourceNodeId();

    resp = System::PacketBuffer::NewWithAvailableSize(Y_len + verifier_len);
    VerifyOrExit(resp != nullptr, err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), Y_len + verifier_len);
        bbuf.Put(&Y[0], Y_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(Y_len + verifier_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pCompute_cA;

    // Call delegate to send the Compute_pB_cB to peer
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_pB_cB, resp);
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

CHIP_ERROR SecurePairingSession::HandleCompute_pB_cB(const MessageHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBuffer * resp = nullptr;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == kMAX_Point_Length + kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.ComputeRoundTwo(buf, kMAX_Point_Length, verifier, &verifier_len);
    SuccessOrExit(err);

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
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pCompute_cA, resp);
    SuccessOrExit(err);

    resp = nullptr;

    {
        const uint8_t * hash = &buf[kMAX_Point_Length];
        err                  = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
        SuccessOrExit(err);

        err = mSpake2p.GetKeys(mKe, &mKeLen);
        SuccessOrExit(err);
    }

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete(mPeerNodeId, mPeerKeyId, mKeyId);

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;

    if (resp != nullptr)
    {
        System::PacketBuffer::Free(resp);
    }

    return err;
}

CHIP_ERROR SecurePairingSession::HandleCompute_cA(const MessageHeader & header, System::PacketBuffer * msg)
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
    mDelegate->OnPairingComplete(mPeerNodeId, mPeerKeyId, mKeyId);

exit:

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgTypeMax;
    return err;
}

CHIP_ERROR SecurePairingSession::HandlePeerMessage(MessageHeader & header, System::PacketBuffer * msg)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    size_t headerSize = 0;

    VerifyOrExit(msg != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = header.DecodeEncryptedHeader(msg->Start(), msg->DataLength(), &headerSize);
    SuccessOrExit(err);

    msg->ConsumeHead(headerSize);

    VerifyOrExit(header.GetProtocolID() == kSecurePairingProtocol, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    VerifyOrExit(header.GetMessageType() == (uint8_t) mNextExpectedMsg, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    switch ((Spake2pMsgType) header.GetMessageType())
    {
    case Spake2pMsgType::kSpake2pCompute_pA:
        err = HandleCompute_pA(header, msg);
        break;

    case Spake2pMsgType::kSpake2pCompute_pB_cB:
        err = HandleCompute_pB_cB(header, msg);
        break;

    case Spake2pMsgType::kSpake2pCompute_cA:
        err = HandleCompute_cA(header, msg);
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
