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
#include <transport/SecurePairingSession.h>

#include <inttypes.h>
#include <string.h>

#include <core/CHIPSafeCasts.h>
#include <protocols/Protocols.h>
#include <support/BufBound.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>

namespace chip {

using namespace Crypto;

const char * kSpake2pContext        = "SPAKE2+ Commissioning";
const char * kSpake2pI2RSessionInfo = "Commissioning I2R Key";
const char * kSpake2pR2ISessionInfo = "Commissioning R2I Key";

SecurePairingSession::SecurePairingSession() {}

SecurePairingSession::~SecurePairingSession()
{
    Clear();
}

void SecurePairingSession::Clear()
{
    memset(&mPoint[0], 0, sizeof(mPoint));
    memset(&mWS[0][0], 0, sizeof(mWS));
    memset(&mKe[0], 0, sizeof(mKe));
    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgError;
    mSpake2p.Init(nullptr);
    mCommissioningHash.Clear();
    mIterationCount = 0;
    mSaltLength     = 0;
    if (mSalt != nullptr)
    {
        chip::Platform::MemoryFree(mSalt);
        mSalt = nullptr;
    }
    mLocalNodeId     = Optional<NodeId>::Value(kUndefinedNodeId);
    mPeerNodeId      = Optional<NodeId>::Value(kUndefinedNodeId);
    mLocalKeyId      = 0;
    mPeerKeyId       = 0;
    mPeerAddress     = Transport::PeerAddress::Uninitialized();
    mKeLen           = sizeof(mKe);
    mPairingComplete = false;
}

CHIP_ERROR SecurePairingSession::Serialize(SecurePairingSessionSerialized & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    SecurePairingSessionSerializable serializable;

    VerifyOrExit(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);

    error = ToSerializable(serializable);
    SuccessOrExit(error);

    serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                       static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
    VerifyOrExit(serializedLen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(serializedLen < sizeof(output.inner), error = CHIP_ERROR_INVALID_ARGUMENT);
    output.inner[serializedLen] = '\0';

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

    error = FromSerializable(serializable);

exit:
    return error;
}

CHIP_ERROR SecurePairingSession::ToSerializable(SecurePairingSessionSerializable & serializable)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    const NodeId localNodeId = mLocalNodeId.ValueOr(kUndefinedNodeId);
    const NodeId peerNodeId  = mPeerNodeId.ValueOr(kUndefinedNodeId);
    VerifyOrExit(CanCastTo<uint16_t>(mKeLen), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(localNodeId), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(peerNodeId), error = CHIP_ERROR_INTERNAL);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mKeLen           = static_cast<uint16_t>(mKeLen);
    serializable.mPairingComplete = (mPairingComplete) ? 1 : 0;
    serializable.mLocalNodeId     = localNodeId;
    serializable.mPeerNodeId      = peerNodeId;
    serializable.mLocalKeyId      = mLocalKeyId;
    serializable.mPeerKeyId       = mPeerKeyId;

    memcpy(serializable.mKe, mKe, mKeLen);

exit:
    return error;
}

CHIP_ERROR SecurePairingSession::FromSerializable(const SecurePairingSessionSerializable & serializable)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

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

CHIP_ERROR SecurePairingSession::Init(Optional<NodeId> myNodeId, uint16_t myKeyId, uint32_t setupCode,
                                      SecurePairingSessionDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mCommissioningHash.Begin();
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(Uint8::from_const_char(kSpake2pContext), strlen(kSpake2pContext));
    SuccessOrExit(err);

    mDelegate     = delegate;
    mLocalNodeId  = myNodeId;
    mLocalKeyId   = myKeyId;
    mSetupPINCode = setupCode;

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::SetupSpake2p(uint32_t pbkdf2IterCount, const uint8_t * salt, size_t saltLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(salt != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(saltLen > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mSpake2p.Init(&mCommissioningHash);
    SuccessOrExit(err);

    err = pbkdf2_sha256(reinterpret_cast<const uint8_t *>(&mSetupPINCode), sizeof(mSetupPINCode), salt, saltLen, pbkdf2IterCount,
                        sizeof(mWS), &mWS[0][0]);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const uint8_t * salt,
                                                size_t saltLen, Optional<NodeId> myNodeId, uint16_t myKeyId,
                                                SecurePairingSessionDelegate * delegate)
{
    //    size_t sizeof_point = sizeof(mPoint);
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(salt != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(saltLen > 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = Init(myNodeId, myKeyId, mySetUpPINCode, delegate);
    SuccessOrExit(err);

    VerifyOrExit(CanCastTo<uint16_t>(saltLen), err = CHIP_ERROR_INVALID_ARGUMENT);
    mSaltLength = static_cast<uint16_t>(saltLen);

    if (mSalt != nullptr)
    {
        chip::Platform::MemoryFree(mSalt);
        mSalt = nullptr;
    }

    mSalt = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(mSaltLength));
    VerifyOrExit(mSalt != nullptr, err = CHIP_ERROR_NO_MEMORY);

    memmove(mSalt, salt, mSaltLength);

    mIterationCount = pbkdf2IterCount;

    mNextExpectedMsg = Spake2pMsgType::kPBKDFParamRequest;
    mPairingComplete = false;

    ChipLogDetail(Ble, "Waiting for PBKDF param request");

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR SecurePairingSession::AttachHeaderAndSend(uint8_t msgType, System::PacketBuffer * msgIn)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle msgBuf;

    PayloadHeader payloadHeader;

    payloadHeader
        .SetMessageType(msgType) //
        .SetProtocolID(Protocols::kProtocol_SecureChannel);

    uint16_t headerSize              = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize = 0;

    msgBuf.Adopt(msgIn);
    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    err = payloadHeader.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == actualEncodedHeaderSize, err = CHIP_ERROR_INTERNAL);

    err = mDelegate->SendPairingMessage(PacketHeader().SetSourceNodeId(mLocalNodeId).SetEncryptionKeyID(mLocalKeyId), mPeerAddress,
                                        msgBuf.Release_ForNow());
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode,
                                      Optional<NodeId> myNodeId, uint16_t myKeyId, SecurePairingSessionDelegate * delegate)
{
    CHIP_ERROR err = Init(myNodeId, myKeyId, peerSetUpPINCode, delegate);
    SuccessOrExit(err);

    mPeerAddress = peerAddress;

    err = SendPBKDFParamRequest();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
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

CHIP_ERROR SecurePairingSession::SendPBKDFParamRequest()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle req;
    uint16_t reqlen          = sizeof(PBKDFParamRequest);
    PBKDFParamRequest * pReq = nullptr;

    req = System::PacketBuffer::NewWithAvailableSize(reqlen);
    VerifyOrExit(!req.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    pReq = reinterpret_cast<PBKDFParamRequest *>(req->Start());
    err  = DRBG_get_bytes(pReq->mRandom, sizeof(pReq->mRandom));
    SuccessOrExit(err);

    req->SetDataLength(reqlen);

    // Update commissioning hash with the pbkdf2 param request that's being sent.
    err = mCommissioningHash.AddData(req->Start(), req->TotalLength());
    SuccessOrExit(err);

    mNextExpectedMsg = Spake2pMsgType::kPBKDFParamResponse;

    err = AttachHeaderAndSend(Spake2pMsgType::kPBKDFParamRequest, req.Release_ForNow());
    SuccessOrExit(err);

    ChipLogDetail(Ble, "Sent PBKDF param request");

exit:

    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR SecurePairingSession::HandlePBKDFParamRequest(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Request message processing
    const uint8_t * req = msg->Start();
    size_t reqlen       = msg->TotalLength();

    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == sizeof(PBKDFParamRequest), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    ChipLogDetail(Ble, "Received PBKDF param request");

    // Update commissioning hash with the received pbkdf2 param request
    err = mCommissioningHash.AddData(req, reqlen);
    SuccessOrExit(err);

    err = SendPBKDFParamResponse();
    SuccessOrExit(err);

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(Spake2pErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR SecurePairingSession::SendPBKDFParamResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle resp;
    uint16_t resplen           = 0;
    PBKDFParamResponse * pResp = nullptr;

    size_t sizeof_point = sizeof(mPoint);

    VerifyOrExit(CanCastTo<uint16_t>(sizeof(PBKDFParamResponse) + mSaltLength), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    resplen = static_cast<uint16_t>(sizeof(PBKDFParamResponse) + mSaltLength);

    resp = System::PacketBuffer::NewWithAvailableSize(resplen);
    VerifyOrExit(!resp.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    pResp = reinterpret_cast<PBKDFParamResponse *>(resp->Start());
    err   = DRBG_get_bytes(pResp->mRandom, sizeof(pResp->mRandom));
    SuccessOrExit(err);

    pResp->mIterations = mIterationCount;
    pResp->mSaltLength = mSaltLength;
    {
        BufBound bbuf(pResp->mSalt, mSaltLength);
        bbuf.Put(mSalt, mSaltLength);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(resplen);

    // Update commissioning hash with the pbkdf2 param response that's being sent.
    err = mCommissioningHash.AddData(resp->Start(), resp->TotalLength());
    SuccessOrExit(err);

    err = SetupSpake2p(mIterationCount, mSalt, mSaltLength);
    SuccessOrExit(err);

    err = mSpake2p.ComputeL(mPoint, &sizeof_point, &mWS[1][0], kSpake2p_WS_Length);
    SuccessOrExit(err);

    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsg1;

    err = AttachHeaderAndSend(Spake2pMsgType::kPBKDFParamResponse, resp.Release_ForNow());
    SuccessOrExit(err);

    ChipLogDetail(Ble, "Sent PBKDF param response");

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::HandlePBKDFParamResponse(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Response message processing
    const uint8_t * resp             = msg->Start();
    size_t resplen                   = msg->TotalLength();
    const PBKDFParamResponse * pResp = reinterpret_cast<const PBKDFParamResponse *>(resp);
    ;

    ChipLogDetail(Ble, "Received PBKDF param response");

    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen >= sizeof(PBKDFParamResponse), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(resplen == sizeof(PBKDFParamResponse) + pResp->mSaltLength, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Specifications allow message to carry a uint64_t sized iteration count. Current APIs are
    // limiting it to uint32_t. Let's make sure it'll fit the size limit.
    VerifyOrExit(CanCastTo<uint32_t>(pResp->mIterations), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Update commissioning hash with the received pbkdf2 param response
    err = mCommissioningHash.AddData(resp, resplen);
    SuccessOrExit(err);

    err = SetupSpake2p(static_cast<uint32_t>(pResp->mIterations), pResp->mSalt, pResp->mSaltLength);
    SuccessOrExit(err);

    err = SendMsg1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(Spake2pErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR SecurePairingSession::SendMsg1()
{
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    uint16_t data_len; // Will be the same as X_len in practice.

    System::PacketBufferHandle msg_pA;

    CHIP_ERROR err = mSpake2p.BeginProver(reinterpret_cast<const uint8_t *>(""), 0, reinterpret_cast<const uint8_t *>(""), 0,
                                          &mWS[0][0], kSpake2p_WS_Length, &mWS[1][0], kSpake2p_WS_Length);
    SuccessOrExit(err);

    err = mSpake2p.ComputeRoundOne(X, &X_len);
    SuccessOrExit(err);
    VerifyOrExit(CanCastTo<uint16_t>(X_len), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    data_len = static_cast<uint16_t>(X_len);

    msg_pA = System::PacketBuffer::NewWithAvailableSize(data_len);
    VerifyOrExit(!msg_pA.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(msg_pA->Start(), data_len);
        bbuf.Put(&X[0], X_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_pA->SetDataLength(data_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsg2;

    // Call delegate to send the Msg1 to peer
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pMsg1, msg_pA.Release_ForNow());
    SuccessOrExit(err);

    ChipLogDetail(Ble, "Sent spake2p msg1");

exit:
    return err;
}

CHIP_ERROR SecurePairingSession::HandleMsg1_and_SendMsg2(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    uint16_t data_len; // To be initialized once we compute it.

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBufferHandle resp;

    ChipLogDetail(Ble, "Received spake2p msg1");

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
    VerifyOrExit(!resp.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), data_len);
        bbuf.Put(&Y[0], Y_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(data_len);
    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsg3;

    // Call delegate to send the Msg2 to peer
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pMsg2, resp.Release_ForNow());
    SuccessOrExit(err);

    ChipLogDetail(Ble, "Sent spake2p msg2");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(Spake2pErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR SecurePairingSession::HandleMsg2_and_SendMsg3(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len_raw = kMAX_Hash_Length;
    uint16_t verifier_len; // To be inited one we check length is small enough

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->TotalLength();

    System::PacketBufferHandle resp;

    Spake2pErrorType spake2pErr = Spake2pErrorType::kUnexpected;

    ChipLogDetail(Ble, "Received spake2p msg2");

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == kMAX_Point_Length + kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.ComputeRoundTwo(buf, kMAX_Point_Length, verifier, &verifier_len_raw);
    SuccessOrExit(err);
    VerifyOrExit(CanCastTo<uint16_t>(verifier_len_raw), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    verifier_len = static_cast<uint16_t>(verifier_len_raw);

    mPeerKeyId  = header.GetEncryptionKeyID();
    mPeerNodeId = header.GetSourceNodeId();

    resp = System::PacketBuffer::NewWithAvailableSize(verifier_len);
    VerifyOrExit(!resp.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        BufBound bbuf(resp->Start(), verifier_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(verifier_len);

    // Call delegate to send the Msg3 to peer
    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pMsg3, resp.Release_ForNow());
    SuccessOrExit(err);

    ChipLogDetail(Ble, "Sent spake2p msg3");

    {
        const uint8_t * hash = &buf[kMAX_Point_Length];
        err                  = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
        if (err != CHIP_NO_ERROR)
        {
            spake2pErr = Spake2pErrorType::kInvalidKeyConfirmation;
            SuccessOrExit(err);
        }

        err = mSpake2p.GetKeys(mKe, &mKeLen);
        SuccessOrExit(err);
    }

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(spake2pErr);
    }
    return err;
}

CHIP_ERROR SecurePairingSession::HandleMsg3(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    const uint8_t * hash        = msg->Start();
    Spake2pErrorType spake2pErr = Spake2pErrorType::kUnexpected;

    ChipLogDetail(Ble, "Received spake2p msg3");

    // We will set NextExpectedMsg to kSpake2pMsgError in all cases
    // However, when we are using IP rendezvous, we might set it to kSpake2pMsg1.
    mNextExpectedMsg = Spake2pMsgType::kSpake2pMsgError;

    VerifyOrExit(hash != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(msg->TotalLength() == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    VerifyOrExit(header.GetSourceNodeId() == mPeerNodeId, err = CHIP_ERROR_WRONG_NODE_ID);
    VerifyOrExit(header.GetEncryptionKeyID() == mPeerKeyId, err = CHIP_ERROR_INVALID_KEY_ID);

    err = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
    if (err != CHIP_NO_ERROR)
    {
        spake2pErr = Spake2pErrorType::kInvalidKeyConfirmation;
        SuccessOrExit(err);
    }

    err = mSpake2p.GetKeys(mKe, &mKeLen);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnPairingComplete();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(spake2pErr);
    }
    return err;
}

void SecurePairingSession::SendErrorMsg(Spake2pErrorType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg;
    uint16_t msglen        = sizeof(Spake2pErrorMsg);
    Spake2pErrorMsg * pMsg = nullptr;

    msg = System::PacketBuffer::NewWithAvailableSize(msglen);
    VerifyOrExit(!msg.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    pMsg        = reinterpret_cast<Spake2pErrorMsg *>(msg->Start());
    pMsg->error = errorCode;

    msg->SetDataLength(msglen);

    err = AttachHeaderAndSend(Spake2pMsgType::kSpake2pMsgError, msg.Release_ForNow());
    SuccessOrExit(err);

exit:
    Clear();
}

void SecurePairingSession::HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    // Request message processing
    const uint8_t * buf    = msg->Start();
    size_t buflen          = msg->TotalLength();
    Spake2pErrorMsg * pMsg = nullptr;

    VerifyOrExit(buf != nullptr, ChipLogError(Ble, "Null error msg received during pairing"));
    VerifyOrExit(buflen == sizeof(Spake2pErrorMsg), ChipLogError(Ble, "Error msg with incorrect length received during pairing"));

    pMsg = reinterpret_cast<Spake2pErrorMsg *>(msg->Start());
    ChipLogError(Ble, "Received error (%d) during pairing process", pMsg->error);

exit:
    Clear();
}

CHIP_ERROR SecurePairingSession::HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                   System::PacketBufferHandle msg)
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

    mPeerAddress = peerAddress;

    switch (static_cast<Spake2pMsgType>(payloadHeader.GetMessageType()))
    {
    case Spake2pMsgType::kPBKDFParamRequest:
        err = HandlePBKDFParamRequest(packetHeader, msg);
        break;

    case Spake2pMsgType::kPBKDFParamResponse:
        err = HandlePBKDFParamResponse(packetHeader, msg);
        break;

    case Spake2pMsgType::kSpake2pMsg1:
        err = HandleMsg1_and_SendMsg2(packetHeader, msg);
        break;

    case Spake2pMsgType::kSpake2pMsg2:
        err = HandleMsg2_and_SendMsg3(packetHeader, msg);
        break;

    case Spake2pMsgType::kSpake2pMsg3:
        err = HandleMsg3(packetHeader, msg);
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

    return err;
}

} // namespace chip
