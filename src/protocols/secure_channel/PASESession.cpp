/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <protocols/secure_channel/PASESession.h>

#include <inttypes.h>
#include <string.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <setup_payload/SetupPayload.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

using namespace Crypto;
using namespace Messaging;

const char * kSpake2pContext        = "CHIP PAKE V1 Commissioning";
const char * kSpake2pI2RSessionInfo = "Commissioning I2R Key";
const char * kSpake2pR2ISessionInfo = "Commissioning R2I Key";

const char * kSpake2pKeyExchangeSalt = "SPAKE2P Key Salt";

// Wait at most 30 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received with in timeout window.
static constexpr ExchangeContext::Timeout kSpake2p_Response_Timeout = 30000;

#ifdef ENABLE_HSM_PBKDF2
using PBKDF2_sha256_crypto = PBKDF2_sha256HSM;
#else
using PBKDF2_sha256_crypto = PBKDF2_sha256;
#endif

PASESession::PASESession() {}

PASESession::~PASESession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void PASESession::Clear()
{
    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    memset(&mPoint[0], 0, sizeof(mPoint));
    memset(&mPASEVerifier, 0, sizeof(mPASEVerifier));
    memset(&mKe[0], 0, sizeof(mKe));
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2pError;

    // Note: we don't need to explicitly clear the state of mSpake2p object.
    //       Clearing the following state takes care of it.
    mCommissioningHash.Clear();

    mIterationCount = 0;
    mSaltLength     = 0;
    if (mSalt != nullptr)
    {
        chip::Platform::MemoryFree(mSalt);
        mSalt = nullptr;
    }
    mKeLen           = sizeof(mKe);
    mPairingComplete = false;
    mComputeVerifier = true;
    PairingSession::Clear();
    CloseExchange();
}

void PASESession::CloseExchange()
{
    if (mExchangeCtxt != nullptr)
    {
        mExchangeCtxt->Close();
        mExchangeCtxt = nullptr;
    }
}

CHIP_ERROR PASESession::Serialize(PASESessionSerialized & output)
{
    PASESessionSerializable serializable;
    VerifyOrReturnError(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(ToSerializable(serializable));

    uint16_t serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                                static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
    VerifyOrReturnError(serializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(serializedLen < sizeof(output.inner), CHIP_ERROR_INVALID_ARGUMENT);
    output.inner[serializedLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::Deserialize(PASESessionSerialized & input)
{
    PASESessionSerializable serializable;
    size_t maxlen            = BASE64_ENCODED_LEN(sizeof(serializable));
    size_t len               = strnlen(Uint8::to_char(input.inner), maxlen);
    uint16_t deserializedLen = 0;

    VerifyOrReturnError(len < sizeof(PASESessionSerialized), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_INVALID_ARGUMENT);

    memset(&serializable, 0, sizeof(serializable));
    deserializedLen =
        Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len), Uint8::to_uchar((uint8_t *) &serializable));

    VerifyOrReturnError(deserializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(deserializedLen <= sizeof(serializable), CHIP_ERROR_INVALID_ARGUMENT);

    return FromSerializable(serializable);
}

CHIP_ERROR PASESession::ToSerializable(PASESessionSerializable & serializable)
{
    VerifyOrReturnError(CanCastTo<uint16_t>(mKeLen), CHIP_ERROR_INTERNAL);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mKeLen           = static_cast<uint16_t>(mKeLen);
    serializable.mPairingComplete = (mPairingComplete) ? 1 : 0;
    serializable.mLocalKeyId      = GetLocalKeyId();
    serializable.mPeerKeyId       = GetPeerKeyId();

    memcpy(serializable.mKe, mKe, mKeLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::FromSerializable(const PASESessionSerializable & serializable)
{
    mPairingComplete = (serializable.mPairingComplete == 1);
    mKeLen           = static_cast<size_t>(serializable.mKeLen);

    VerifyOrReturnError(mKeLen <= sizeof(mKe), CHIP_ERROR_INVALID_ARGUMENT);
    memset(mKe, 0, sizeof(mKe));
    memcpy(mKe, serializable.mKe, mKeLen);

    SetLocalKeyId(serializable.mLocalKeyId);
    SetPeerKeyId(serializable.mPeerKeyId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::Init(uint16_t myKeyId, uint32_t setupCode, SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Reset any state maintained by PASESession object (in case it's being reused for pairing)
    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ Uint8::from_const_char(kSpake2pContext), strlen(kSpake2pContext) }));

    mDelegate = delegate;

    ChipLogDetail(SecureChannel, "Assigned local session key ID %d", myKeyId);
    SetLocalKeyId(myKeyId);
    mSetupPINCode    = setupCode;
    mComputeVerifier = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::ComputePASEVerifier(uint32_t setUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                            PASEVerifier & verifier)
{
    ReturnErrorCodeIf(salt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(salt.data() == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(setUpPINCode >= (1 << kSetupPINCodeFieldLengthInBits), CHIP_ERROR_INVALID_ARGUMENT);

    PBKDF2_sha256_crypto mPBKDF;
    uint8_t littleEndianSetupPINCode[sizeof(uint32_t)];
    Encoding::LittleEndian::Put32(littleEndianSetupPINCode, setUpPINCode);

    return mPBKDF.pbkdf2_sha256(littleEndianSetupPINCode, sizeof(littleEndianSetupPINCode), salt.data(), salt.size(),
                                pbkdf2IterCount, sizeof(PASEVerifier), reinterpret_cast<uint8_t *>(&verifier));
}

CHIP_ERROR PASESession::GeneratePASEVerifier(PASEVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                             bool useRandomPIN, uint32_t & setupPIN)
{
    if (useRandomPIN)
    {
        ReturnErrorOnFailure(DRBG_get_bytes(reinterpret_cast<uint8_t *>(&setupPIN), sizeof(setupPIN)));

        // Use only kSetupPINCodeFieldLengthInBits bits out of the code
        setupPIN &= ((1 << kSetupPINCodeFieldLengthInBits) - 1);
    }

    return PASESession::ComputePASEVerifier(setupPIN, pbkdf2IterCount, salt, verifier);
}

CHIP_ERROR PASESession::SetupSpake2p(uint32_t pbkdf2IterCount, const ByteSpan & salt)
{
    uint8_t context[kSHA256_Hash_Length] = {
        0,
    };

    if (mComputeVerifier)
    {
        ReturnErrorOnFailure(PASESession::ComputePASEVerifier(mSetupPINCode, pbkdf2IterCount, salt, mPASEVerifier));
    }

    MutableByteSpan contextSpan{ context, sizeof(context) };

    ReturnErrorOnFailure(mCommissioningHash.Finish(contextSpan));
    ReturnErrorOnFailure(mSpake2p.Init(contextSpan.data(), contextSpan.size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt, uint16_t myKeyId,
                                       SessionEstablishmentDelegate * delegate)
{
    // Return early on error here, as we have not initalized any state yet
    ReturnErrorCodeIf(salt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(salt.data() == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = Init(myKeyId, mySetUpPINCode, delegate);
    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExit(err);

    VerifyOrExit(CanCastTo<uint16_t>(salt.size()), err = CHIP_ERROR_INVALID_ARGUMENT);
    mSaltLength = static_cast<uint16_t>(salt.size());

    if (mSalt != nullptr)
    {
        chip::Platform::MemoryFree(mSalt);
        mSalt = nullptr;
    }

    mSalt = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(mSaltLength));
    VerifyOrExit(mSalt != nullptr, err = CHIP_ERROR_NO_MEMORY);

    memmove(mSalt, salt.data(), mSaltLength);

    mIterationCount = pbkdf2IterCount;

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PBKDFParamRequest;
    mPairingComplete = false;
    mPasscodeID      = 0;

    ChipLogDetail(SecureChannel, "Waiting for PBKDF param request");

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR PASESession::WaitForPairing(const PASEVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                       uint16_t passcodeID, uint16_t myKeyId, SessionEstablishmentDelegate * delegate)
{
    ReturnErrorOnFailure(WaitForPairing(0, pbkdf2IterCount, salt, myKeyId, delegate));

    memmove(&mPASEVerifier, &verifier, sizeof(verifier));
    mComputeVerifier = false;
    mPasscodeID      = passcodeID;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode, uint16_t myKeyId,
                             Messaging::ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate)
{
    ReturnErrorCodeIf(exchangeCtxt == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CHIP_ERROR err = Init(myKeyId, peerSetUpPINCode, delegate);
    SuccessOrExit(err);

    mExchangeCtxt = exchangeCtxt;
    mExchangeCtxt->SetResponseTimeout(kSpake2p_Response_Timeout);

    SetPeerAddress(peerAddress);

    err = SendPBKDFParamRequest();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

void PASESession::OnResponseTimeout(ExchangeContext * ec)
{
    VerifyOrReturn(ec != nullptr, ChipLogError(SecureChannel, "PASESession::OnResponseTimeout was called by null exchange"));
    VerifyOrReturn(mExchangeCtxt == nullptr || mExchangeCtxt == ec,
                   ChipLogError(SecureChannel, "PASESession::OnResponseTimeout exchange doesn't match"));
    ChipLogError(SecureChannel,
                 "PASESession timed out while waiting for a response from the peer. Expected message type was %" PRIu8,
                 to_underlying(mNextExpectedMsg));
    mDelegate->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
    // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    mExchangeCtxt = nullptr;
    Clear();
}

CHIP_ERROR PASESession::DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role)
{
    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);
    return session.InitFromSecret(ByteSpan(mKe, mKeLen), ByteSpan(nullptr, 0),
                                  SecureSession::SessionInfoType::kSessionEstablishment, role);
}

CHIP_ERROR PASESession::SendPBKDFParamRequest()
{
    System::PacketBufferHandle req = System::PacketBufferHandle::New(kPBKDFParamRandomNumberSize);
    VerifyOrReturnError(!req.IsNull(), CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(DRBG_get_bytes(req->Start(), kPBKDFParamRandomNumberSize));

    req->SetDataLength(kPBKDFParamRandomNumberSize);

    // Update commissioning hash with the pbkdf2 param request that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ req->Start(), req->DataLength() }));

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PBKDFParamResponse;

    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PBKDFParamRequest, std::move(req),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    ChipLogDetail(SecureChannel, "Sent PBKDF param request");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamRequest(const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Request message processing
    const uint8_t * req = msg->Start();
    size_t reqlen       = msg->DataLength();

    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == kPBKDFParamRandomNumberSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    ChipLogDetail(SecureChannel, "Received PBKDF param request");

    // Update commissioning hash with the received pbkdf2 param request
    err = mCommissioningHash.AddData(ByteSpan{ req, reqlen });
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

CHIP_ERROR PASESession::SendPBKDFParamResponse()
{
    System::PacketBufferHandle resp;
    static_assert(CHAR_BIT == 8, "Assuming sizeof() returns octets here and for sizeof(mPoint)");
    size_t resplen = kPBKDFParamRandomNumberSize + sizeof(uint64_t) + sizeof(uint32_t) + mSaltLength;

    size_t sizeof_point = sizeof(mPoint);

    uint8_t * msg = nullptr;

    resp = System::PacketBufferHandle::New(resplen);
    VerifyOrReturnError(!resp.IsNull(), CHIP_ERROR_NO_MEMORY);

    msg = resp->Start();

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(msg, kPBKDFParamRandomNumberSize));

    // Let's construct the rest of the message using BufferWriter
    {
        Encoding::LittleEndian::BufferWriter bbuf(&msg[kPBKDFParamRandomNumberSize], resplen - kPBKDFParamRandomNumberSize);
        bbuf.Put64(mIterationCount);
        bbuf.Put32(mSaltLength);
        bbuf.Put(mSalt, mSaltLength);
        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);
    }

    resp->SetDataLength(static_cast<uint16_t>(resplen));

    // Update commissioning hash with the pbkdf2 param response that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ resp->Start(), resp->DataLength() }));
    ReturnErrorOnFailure(SetupSpake2p(mIterationCount, ByteSpan(mSalt, mSaltLength)));
    ReturnErrorOnFailure(mSpake2p.ComputeL(mPoint, &sizeof_point, mPASEVerifier.mL, kSpake2p_WS_Length));

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2p1;

    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PBKDFParamResponse, std::move(resp),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent PBKDF param response");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamResponse(const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Response message processing
    const uint8_t * resp = msg->Start();
    size_t resplen       = msg->DataLength();

    // This the fixed part of the message. The variable part of the message contains the salt.
    // The length of the variable part is determined by the salt length in the fixed header.
    static_assert(CHAR_BIT == 8, "Assuming that sizeof returns octets");
    size_t fixed_resplen = kPBKDFParamRandomNumberSize + sizeof(uint64_t) + sizeof(uint32_t);

    ChipLogDetail(SecureChannel, "Received PBKDF param response");

    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen >= fixed_resplen, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    {
        // Let's skip the random number portion of the message
        const uint8_t * msgptr = &resp[kPBKDFParamRandomNumberSize];
        uint64_t iterCount     = chip::Encoding::LittleEndian::Read64(msgptr);
        uint32_t saltlen       = chip::Encoding::LittleEndian::Read32(msgptr);

        VerifyOrExit(resplen == fixed_resplen + saltlen, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        // Specifications allow message to carry a uint64_t sized iteration count. Current APIs are
        // limiting it to uint32_t. Let's make sure it'll fit the size limit.
        VerifyOrExit(CanCastTo<uint32_t>(iterCount), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        // Update commissioning hash with the received pbkdf2 param response
        err = mCommissioningHash.AddData(ByteSpan{ resp, resplen });
        SuccessOrExit(err);

        err = SetupSpake2p(static_cast<uint32_t>(iterCount), ByteSpan(msgptr, saltlen));
        SuccessOrExit(err);
    }

    err = SendMsg1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(Spake2pErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR PASESession::SendMsg1()
{
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);

    ReturnErrorOnFailure(
        mSpake2p.BeginProver(nullptr, 0, nullptr, 0, mPASEVerifier.mW0, kSpake2p_WS_Length, mPASEVerifier.mL, kSpake2p_WS_Length));

    ReturnErrorOnFailure(mSpake2p.ComputeRoundOne(NULL, 0, X, &X_len));

    Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(sizeof(uint16_t) + X_len));
    VerifyOrReturnError(!bbuf.IsNull(), CHIP_ERROR_NO_MEMORY);
    bbuf.Put16(GetLocalKeyId());
    bbuf.Put(&X[0], X_len);
    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2p2;

    // Call delegate to send the Msg1 to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PASE_Spake2p1, bbuf.Finalize(),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent spake2p msg1");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandleMsg1_and_SendMsg2(const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    uint16_t data_len; // To be initialized once we compute it.

    const uint8_t * buf = msg->Start();
    size_t buf_len      = msg->DataLength();

    uint16_t encryptionKeyId = 0;

    ChipLogDetail(SecureChannel, "Received spake2p msg1");

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == sizeof(encryptionKeyId) + kMAX_Point_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.BeginVerifier(nullptr, 0, nullptr, 0, mPASEVerifier.mW0, kSpake2p_WS_Length, mPoint, sizeof(mPoint));
    SuccessOrExit(err);

    encryptionKeyId = chip::Encoding::LittleEndian::Read16(buf);
    msg->ConsumeHead(sizeof(encryptionKeyId));

    // Pass Pa to check abort condition.
    err = mSpake2p.ComputeRoundOne(msg->Start(), msg->DataLength(), Y, &Y_len);
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    SetPeerKeyId(encryptionKeyId);

    err = mSpake2p.ComputeRoundTwo(msg->Start(), msg->DataLength(), verifier, &verifier_len);
    SuccessOrExit(err);

    // Make sure our addition doesn't overflow.
    VerifyOrExit(UINTMAX_MAX - verifier_len >= Y_len, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(CanCastTo<uint16_t>(Y_len + verifier_len), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    data_len = static_cast<uint16_t>(sizeof(encryptionKeyId) + Y_len + verifier_len);

    {
        Encoding::LittleEndian::PacketBufferWriter bbuf(System::PacketBufferHandle::New(data_len));
        VerifyOrExit(!bbuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
        bbuf.Put16(GetLocalKeyId());
        bbuf.Put(&Y[0], Y_len);
        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);

        mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2p3;

        // Call delegate to send the Msg2 to peer
        err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PASE_Spake2p2, bbuf.Finalize(),
                                         SendFlags(SendMessageFlags::kExpectResponse));
        SuccessOrExit(err);
    }

    ChipLogDetail(SecureChannel, "Sent spake2p msg2");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(Spake2pErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR PASESession::HandleMsg2_and_SendMsg3(const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len_raw = kMAX_Hash_Length;
    uint16_t verifier_len; // To be inited one we check length is small enough

    uint8_t * buf  = msg->Start();
    size_t buf_len = msg->DataLength();

    System::PacketBufferHandle resp;

    Spake2pErrorType spake2pErr = Spake2pErrorType::kUnexpected;

    uint16_t encryptionKeyId = 0;

    ChipLogDetail(SecureChannel, "Received spake2p msg2");

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buf_len == sizeof(encryptionKeyId) + kMAX_Point_Length + kMAX_Hash_Length,
                 err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    encryptionKeyId = chip::Encoding::LittleEndian::Read16(buf);
    msg->ConsumeHead(sizeof(encryptionKeyId));
    buf     = msg->Start();
    buf_len = msg->DataLength();

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    SetPeerKeyId(encryptionKeyId);

    err = mSpake2p.ComputeRoundTwo(buf, kMAX_Point_Length, verifier, &verifier_len_raw);
    SuccessOrExit(err);
    VerifyOrExit(CanCastTo<uint16_t>(verifier_len_raw), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    verifier_len = static_cast<uint16_t>(verifier_len_raw);

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

    {
        Encoding::PacketBufferWriter bbuf(System::PacketBufferHandle::New(verifier_len));
        VerifyOrExit(!bbuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        bbuf.Put(verifier, verifier_len);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);

        // Call delegate to send the Msg3 to peer
        err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PASE_Spake2p3, bbuf.Finalize());
        SuccessOrExit(err);
    }

    ChipLogDetail(SecureChannel, "Sent spake2p msg3");

    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(spake2pErr);
    }
    return err;
}

CHIP_ERROR PASESession::HandleMsg3(const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    const uint8_t * hash        = msg->Start();
    Spake2pErrorType spake2pErr = Spake2pErrorType::kUnexpected;

    ChipLogDetail(SecureChannel, "Received spake2p msg3");

    // We will set NextExpectedMsg to PASE_Spake2pError in all cases
    // However, when we are using IP rendezvous, we might set it to PASE_Spake2p1.
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::PASE_Spake2pError;

    VerifyOrExit(hash != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(msg->DataLength() == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = mSpake2p.KeyConfirm(hash, kMAX_Hash_Length);
    if (err != CHIP_NO_ERROR)
    {
        spake2pErr = Spake2pErrorType::kInvalidKeyConfirmation;
        SuccessOrExit(err);
    }

    err = mSpake2p.GetKeys(mKe, &mKeLen);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(spake2pErr);
    }
    return err;
}

void PASESession::SendErrorMsg(Spake2pErrorType errorCode)
{
    System::PacketBufferHandle msg;
    uint16_t msglen        = sizeof(Spake2pErrorMsg);
    Spake2pErrorMsg * pMsg = nullptr;

    msg = System::PacketBufferHandle::New(msglen);
    VerifyOrReturn(!msg.IsNull(), ChipLogError(SecureChannel, "Failed to allocate error message"));

    pMsg        = reinterpret_cast<Spake2pErrorMsg *>(msg->Start());
    pMsg->error = errorCode;

    msg->SetDataLength(msglen);

    VerifyOrReturn(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::PASE_Spake2pError, std::move(msg)) ==
                       CHIP_NO_ERROR,
                   ChipLogError(SecureChannel, "Failed to send error message"));
}

CHIP_ERROR PASESession::HandleErrorMsg(const System::PacketBufferHandle & msg)
{
    ReturnErrorCodeIf(msg->Start() == nullptr || msg->DataLength() != sizeof(Spake2pErrorMsg), CHIP_ERROR_MESSAGE_INCOMPLETE);

    static_assert(
        sizeof(Spake2pErrorMsg) == sizeof(uint8_t),
        "Assuming size of Spake2pErrorMsg message is 1 octet, so that endian-ness conversion and memory alignment is not needed");

    Spake2pErrorMsg * pMsg = reinterpret_cast<Spake2pErrorMsg *>(msg->Start());

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (pMsg->error)
    {
    case Spake2pErrorType::kInvalidKeyConfirmation:
        err = CHIP_ERROR_KEY_CONFIRMATION_FAILED;
        break;

    case Spake2pErrorType::kUnexpected:
        err = CHIP_ERROR_INVALID_PASE_PARAMETER;
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };
    ChipLogError(SecureChannel, "Received error during pairing process. %s", ErrorStr(err));

    return err;
}

CHIP_ERROR PASESession::ValidateReceivedMessage(ExchangeContext * exchange, const PacketHeader & packetHeader,
                                                const PayloadHeader & payloadHeader, System::PacketBufferHandle && msg)
{
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (PBKDFParamRequest) received by PASESession
    // via UnsolicitedMessageHandler. The exchange context is allocated by exchange manager and provided
    // to the handler (PASESession object).
    if (mExchangeCtxt != nullptr)
    {
        if (mExchangeCtxt != exchange)
        {
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        mExchangeCtxt = exchange;
        mExchangeCtxt->SetResponseTimeout(kSpake2p_Response_Timeout);
    }

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payloadHeader.HasMessageType(mNextExpectedMsg) ||
                            payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::PASE_Spake2pError),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::OnMessageReceived(ExchangeContext * exchange, const PacketHeader & packetHeader,
                                          const PayloadHeader & payloadHeader, System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = ValidateReceivedMessage(exchange, packetHeader, payloadHeader, std::move(msg));
    SuccessOrExit(err);

    SetPeerAddress(mMessageDispatch.GetPeerAddress());

    switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
    {
    case Protocols::SecureChannel::MsgType::PBKDFParamRequest:
        err = HandlePBKDFParamRequest(msg);
        break;

    case Protocols::SecureChannel::MsgType::PBKDFParamResponse:
        err = HandlePBKDFParamResponse(msg);
        break;

    case Protocols::SecureChannel::MsgType::PASE_Spake2p1:
        err = HandleMsg1_and_SendMsg2(msg);
        break;

    case Protocols::SecureChannel::MsgType::PASE_Spake2p2:
        err = HandleMsg2_and_SendMsg3(msg);
        break;

    case Protocols::SecureChannel::MsgType::PASE_Spake2p3:
        err = HandleMsg3(msg);
        break;

    case Protocols::SecureChannel::MsgType::PASE_Spake2pError:
        err = HandleErrorMsg(msg);
        break;

    default:
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    };

exit:

    // Call delegate to indicate pairing failure
    if (err != CHIP_NO_ERROR)
    {
        // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        mExchangeCtxt = nullptr;
        Clear();
        ChipLogError(SecureChannel, "Failed during PASE session setup. %s", ErrorStr(err));
        mDelegate->OnSessionEstablishmentError(err);
    }
    return err;
}

} // namespace chip
