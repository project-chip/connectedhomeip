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
#include <protocols/secure_channel/StatusReport.h>
#include <setup_payload/SetupPayload.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

namespace chip {

using namespace Crypto;
using namespace Messaging;
using namespace Protocols::SecureChannel;

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
    mNextExpectedMsg = MsgType::PASE_PakeError;

    mSpake2p.Clear();
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
    serializable.mLocalSessionId  = GetLocalSessionId();
    serializable.mPeerSessionId   = GetPeerSessionId();

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

    SetLocalSessionId(serializable.mLocalSessionId);
    SetPeerSessionId(serializable.mPeerSessionId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::Init(uint16_t mySessionId, uint32_t setupCode, SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Reset any state maintained by PASESession object (in case it's being reused for pairing)
    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ Uint8::from_const_char(kSpake2pContext), strlen(kSpake2pContext) }));

    mDelegate = delegate;

    ChipLogDetail(SecureChannel, "Assigned local session key ID %d", mySessionId);
    SetLocalSessionId(mySessionId);
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

        // Passcodes shall be restricted to the values 00000001 to 99999998 in decimal, see 5.1.1.6
        setupPIN = (setupPIN % 99999998) + 1;
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

CHIP_ERROR PASESession::WaitForPairing(uint32_t mySetUpPINCode, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                       uint16_t mySessionId, SessionEstablishmentDelegate * delegate)
{
    // Return early on error here, as we have not initalized any state yet
    ReturnErrorCodeIf(salt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(salt.data() == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = Init(mySessionId, mySetUpPINCode, delegate);
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

    mNextExpectedMsg = MsgType::PBKDFParamRequest;
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
                                       uint16_t passcodeID, uint16_t mySessionId, SessionEstablishmentDelegate * delegate)
{
    ReturnErrorOnFailure(WaitForPairing(0, pbkdf2IterCount, salt, mySessionId, delegate));

    memmove(&mPASEVerifier, &verifier, sizeof(verifier));
    mComputeVerifier = false;
    mPasscodeID      = passcodeID;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::Pair(const Transport::PeerAddress peerAddress, uint32_t peerSetUpPINCode, uint16_t mySessionId,
                             Messaging::ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate)
{
    ReturnErrorCodeIf(exchangeCtxt == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CHIP_ERROR err = Init(mySessionId, peerSetUpPINCode, delegate);
    SuccessOrExit(err);

    mExchangeCtxt = exchangeCtxt;
    mExchangeCtxt->SetResponseTimeout(kSpake2p_Response_Timeout);

    SetPeerAddress(peerAddress);

    err = SendPBKDFParamRequest();
    SuccessOrExit(err);

    mDelegate->OnSessionEstablishmentStarted();

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

CHIP_ERROR PASESession::DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role)
{
    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);
    return session.InitFromSecret(ByteSpan(mKe, mKeLen), ByteSpan(nullptr, 0),
                                  CryptoContext::SessionInfoType::kSessionEstablishment, role);
}

CHIP_ERROR PASESession::SendPBKDFParamRequest()
{
    ReturnErrorOnFailure(DRBG_get_bytes(mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));

    const size_t max_msg_len =
        EstimateTLVStructOverhead(kPBKDFParamRandomNumberSize + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t), 4);
    System::PacketBufferHandle req = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!req.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(req));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), GetLocalSessionId()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), mPasscodeID));
    ReturnErrorOnFailure(tlvWriter.PutBoolean(TLV::ContextTag(4), mHavePBKDFParameters));
    // TODO - Add optional MRP parameter support to PASE
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&req));

    // Update commissioning hash with the pbkdf2 param request that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ req->Start(), req->DataLength() }));

    mNextExpectedMsg = MsgType::PBKDFParamResponse;

    ReturnErrorOnFailure(
        mExchangeCtxt->SendMessage(MsgType::PBKDFParamRequest, std::move(req), SendFlags(SendMessageFlags::kExpectResponse)));

    ChipLogDetail(SecureChannel, "Sent PBKDF param request");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamRequest(System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t initiatorSessionId;
    uint8_t initiatorRandom[kPBKDFParamRandomNumberSize];

    uint32_t decodeTagIdSeq = 0;
    bool hasPBKDFParameters = false;

    ChipLogDetail(SecureChannel, "Received PBKDF param request");

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(initiatorRandom, sizeof(initiatorRandom)));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(initiatorSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session ID %d", initiatorSessionId);
    SetPeerSessionId(initiatorSessionId);

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(mPasscodeID));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(hasPBKDFParameters));

    // TODO - Check if optional MRP parameters were sent. If so, cache them.

    err = SendPBKDFParamResponse(ByteSpan(initiatorRandom), hasPBKDFParameters);
    SuccessOrExit(err);

    mDelegate->OnSessionEstablishmentStarted();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR PASESession::SendPBKDFParamResponse(ByteSpan initiatorRandom, bool initiatorHasPBKDFParams)
{
    ReturnErrorOnFailure(DRBG_get_bytes(mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));

    const size_t max_msg_len = EstimateTLVStructOverhead(
        kPBKDFParamRandomNumberSize + kPBKDFParamRandomNumberSize + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t), 5);
    System::PacketBufferHandle resp = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!resp.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(resp));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    // The initiator random value is being sent back in the response as required by the specifications
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), initiatorRandom));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), GetLocalSessionId()));

    if (!initiatorHasPBKDFParams)
    {
        TLV::TLVType pbkdfParamContainer;
        ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Structure, pbkdfParamContainer));
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), mIterationCount));
        ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), mSalt, mSaltLength));
        ReturnErrorOnFailure(tlvWriter.EndContainer(pbkdfParamContainer));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&resp));

    // Update commissioning hash with the pbkdf2 param response that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ resp->Start(), resp->DataLength() }));
    ReturnErrorOnFailure(SetupSpake2p(mIterationCount, ByteSpan(mSalt, mSaltLength)));

    size_t sizeof_point = sizeof(mPoint);
    ReturnErrorOnFailure(mSpake2p.ComputeL(mPoint, &sizeof_point, mPASEVerifier.mL, kSpake2p_WS_Length));

    mNextExpectedMsg = MsgType::PASE_Pake1;

    ReturnErrorOnFailure(
        mExchangeCtxt->SendMessage(MsgType::PBKDFParamResponse, std::move(resp), SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent PBKDF param response");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamResponse(System::PacketBufferHandle && msg)
{

    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t responderSessionId;
    uint8_t random[kPBKDFParamRandomNumberSize];

    uint32_t decodeTagIdSeq = 0;
    uint32_t iterCount      = 0;
    uint32_t saltLength     = 0;
    const uint8_t * salt;

    ChipLogDetail(SecureChannel, "Received PBKDF param response");

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    // Initiator's random value
    SuccessOrExit(err = tlvReader.GetBytes(random, sizeof(random)));
    VerifyOrExit(ByteSpan(random).data_equal(ByteSpan(mPBKDFLocalRandomData)), err = CHIP_ERROR_INVALID_PASE_PARAMETER);

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    // Responder's random value
    SuccessOrExit(err = tlvReader.GetBytes(random, sizeof(random)));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    if (mHavePBKDFParameters)
    {
        // TODO - Add a unit test that exercises mHavePBKDFParameters path
        err = SetupSpake2p(mIterationCount, ByteSpan(mSalt, mSaltLength));
        SuccessOrExit(err);
    }
    else
    {
        SuccessOrExit(err = tlvReader.Next());
        SuccessOrExit(err = tlvReader.EnterContainer(containerType));
        decodeTagIdSeq = 0;

        SuccessOrExit(err = tlvReader.Next());
        VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
        SuccessOrExit(err = tlvReader.Get(iterCount));

        SuccessOrExit(err = tlvReader.Next());
        VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
        saltLength = tlvReader.GetLength();
        SuccessOrExit(err = tlvReader.GetDataPtr(salt));

        err = SetupSpake2p(iterCount, ByteSpan(salt, saltLength));
        SuccessOrExit(err);
    }

    err = SendMsg1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR PASESession::SendMsg1()
{
    const size_t max_msg_len       = EstimateTLVStructOverhead(kMAX_Point_Length, 1);
    System::PacketBufferHandle msg = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));

    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);

    constexpr uint8_t kPake1_pA = 1;

    ReturnErrorOnFailure(
        mSpake2p.BeginProver(nullptr, 0, nullptr, 0, mPASEVerifier.mW0, kSpake2p_WS_Length, mPASEVerifier.mL, kSpake2p_WS_Length));
    ReturnErrorOnFailure(mSpake2p.ComputeRoundOne(NULL, 0, X, &X_len));
    VerifyOrReturnError(X_len == sizeof(X), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kPake1_pA), ByteSpan(X)));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg));

    mNextExpectedMsg = MsgType::PASE_Pake2;

    ReturnErrorOnFailure(
        mExchangeCtxt->SendMessage(MsgType::PASE_Pake1, std::move(msg), SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent spake2p msg1");

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandleMsg1_and_SendMsg2(System::PacketBufferHandle && msg1)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    ChipLogDetail(SecureChannel, "Received spake2p msg1");

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * X;
    size_t X_len = 0;

    tlvReader.Init(std::move(msg1));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == 1, err = CHIP_ERROR_INVALID_TLV_TAG);
    X_len = tlvReader.GetLength();
    SuccessOrExit(err = tlvReader.GetDataPtr(X));
    SuccessOrExit(
        err = mSpake2p.BeginVerifier(nullptr, 0, nullptr, 0, mPASEVerifier.mW0, kSpake2p_WS_Length, mPoint, sizeof(mPoint)));

    SuccessOrExit(err = mSpake2p.ComputeRoundOne(X, X_len, Y, &Y_len));
    VerifyOrReturnError(Y_len == sizeof(Y), CHIP_ERROR_INTERNAL);
    SuccessOrExit(err = mSpake2p.ComputeRoundTwo(X, X_len, verifier, &verifier_len));
    msg1 = nullptr;

    {
        const size_t max_msg_len    = EstimateTLVStructOverhead(Y_len + verifier_len, 2);
        constexpr uint8_t kPake2_pB = 1;
        constexpr uint8_t kPake2_cB = 2;

        System::PacketBufferHandle msg2 = System::PacketBufferHandle::New(max_msg_len);
        VerifyOrExit(!msg2.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        System::PacketBufferTLVWriter tlvWriter;
        tlvWriter.Init(std::move(msg2));

        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kPake2_pB), ByteSpan(Y)));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kPake2_cB), ByteSpan(verifier, verifier_len)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg2));

        mNextExpectedMsg = MsgType::PASE_Pake3;

        err = mExchangeCtxt->SendMessage(MsgType::PASE_Pake2, std::move(msg2), SendFlags(SendMessageFlags::kExpectResponse));
        SuccessOrExit(err);
    }

    ChipLogDetail(SecureChannel, "Sent spake2p msg2");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR PASESession::HandleMsg2_and_SendMsg3(System::PacketBufferHandle && msg2)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    System::PacketBufferHandle resp;

    ChipLogDetail(SecureChannel, "Received spake2p msg2");

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * Y;
    size_t Y_len = 0;

    const uint8_t * peer_verifier;
    size_t peer_verifier_len = 0;

    uint32_t decodeTagIdSeq = 0;

    tlvReader.Init(std::move(msg2));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    Y_len = tlvReader.GetLength();
    SuccessOrExit(err = tlvReader.GetDataPtr(Y));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    peer_verifier_len = tlvReader.GetLength();
    SuccessOrExit(err = tlvReader.GetDataPtr(peer_verifier));

    SuccessOrExit(err = mSpake2p.ComputeRoundTwo(Y, Y_len, verifier, &verifier_len));

    SuccessOrExit(err = mSpake2p.KeyConfirm(peer_verifier, peer_verifier_len));
    SuccessOrExit(err = mSpake2p.GetKeys(mKe, &mKeLen));
    msg2 = nullptr;

    {
        const size_t max_msg_len    = EstimateTLVStructOverhead(verifier_len, 1);
        constexpr uint8_t kPake3_cB = 1;

        System::PacketBufferHandle msg3 = System::PacketBufferHandle::New(max_msg_len);
        VerifyOrExit(!msg3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        System::PacketBufferTLVWriter tlvWriter;
        tlvWriter.Init(std::move(msg3));

        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kPake3_cB), ByteSpan(verifier, verifier_len)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg3));

        mNextExpectedMsg = MsgType::StatusReport;

        err = mExchangeCtxt->SendMessage(MsgType::PASE_Pake3, std::move(msg3), SendFlags(SendMessageFlags::kExpectResponse));
        SuccessOrExit(err);
    }

    ChipLogDetail(SecureChannel, "Sent spake2p msg3");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR PASESession::HandleMsg3(System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(SecureChannel, "Received spake2p msg3");

    // We will set NextExpectedMsg to PASE_PakeError in all cases
    mNextExpectedMsg = MsgType::PASE_PakeError;

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * peer_verifier;
    size_t peer_verifier_len = 0;

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == 1, err = CHIP_ERROR_INVALID_TLV_TAG);
    peer_verifier_len = tlvReader.GetLength();
    SuccessOrExit(err = tlvReader.GetDataPtr(peer_verifier));

    VerifyOrExit(peer_verifier_len == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    SuccessOrExit(err = mSpake2p.KeyConfirm(peer_verifier, peer_verifier_len));
    SuccessOrExit(err = mSpake2p.GetKeys(mKe, &mKeLen));

    // Send confirmation to peer that we succeeded so they can start using the session.
    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

void PASESession::OnSuccessStatusReport()
{
    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();
}

CHIP_ERROR PASESession::OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (protocolCode)
    {
    case kProtocolCodeInvalidParam:
        err = CHIP_ERROR_INVALID_PASE_PARAMETER;
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };
    ChipLogError(SecureChannel, "Received error (protocol code %d) during PASE process. %s", protocolCode, ErrorStr(err));
    return err;
}

CHIP_ERROR PASESession::ValidateReceivedMessage(ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                                System::PacketBufferHandle && msg)
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
    VerifyOrReturnError(payloadHeader.HasMessageType(mNextExpectedMsg) || payloadHeader.HasMessageType(MsgType::StatusReport),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::OnMessageReceived(ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = ValidateReceivedMessage(exchange, payloadHeader, std::move(msg));
    SuccessOrExit(err);

    switch (static_cast<MsgType>(payloadHeader.GetMessageType()))
    {
    case MsgType::PBKDFParamRequest:
        err = HandlePBKDFParamRequest(std::move(msg));
        break;

    case MsgType::PBKDFParamResponse:
        err = HandlePBKDFParamResponse(std::move(msg));
        break;

    case MsgType::PASE_Pake1:
        err = HandleMsg1_and_SendMsg2(std::move(msg));
        break;

    case MsgType::PASE_Pake2:
        err = HandleMsg2_and_SendMsg3(std::move(msg));
        break;

    case MsgType::PASE_Pake3:
        err = HandleMsg3(std::move(msg));
        break;

    case MsgType::StatusReport:
        err = HandleStatusReport(std::move(msg), mNextExpectedMsg == MsgType::StatusReport);
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
