/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <messaging/SessionParameters.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <setup_payload/SetupPayload.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <tracing/macros.h>
#include <transport/SessionManager.h>

namespace {

enum class PBKDFParamRequestTags : uint8_t
{
    kInitiatorRandom        = 1,
    kInitiatorSessionId     = 2,
    kPasscodeId             = 3,
    kHasPBKDFParameters     = 4,
    kInitiatorSessionParams = 5,
};

enum class PBKDFParamResponseTags : uint8_t
{
    kInitiatorRandom        = 1,
    kResponderRandom        = 2,
    kResponderSessionId     = 3,
    kPbkdfParameters        = 4,
    kResponderSessionParams = 5,
};

enum class PBKDFParameterSetTags : uint8_t
{
    kIterations = 1,
    kSalt       = 2,
};

enum class Pake1Tags : uint8_t
{
    kPa = 1,
};

enum class Pake2Tags : uint8_t
{
    kPb = 1,
    kCb = 2,
};
enum class Pake3Tags : uint8_t
{
    kCa = 1,
};

// Utility to extract the underlying value of TLV Tag enum classes, used in TLV encoding and parsing.
template <typename Enum>
constexpr chip::TLV::Tag AsTlvContextTag(Enum e)
{
    return chip::TLV::ContextTag(chip::to_underlying(e));
}

} // namespace
namespace chip {

using namespace Crypto;
using namespace Messaging;
using namespace Protocols::SecureChannel;

const char kSpake2pContext[] = "CHIP PAKE V1 Commissioning";

// Amounts of time to allow for server-side processing of messages.
//
// These timeout values only allow for the server-side processing and assume that any transport-specific
// latency will be added to them.
//
// The session establishment fails if the response is not received within the resulting timeout window,
// which accounts for both transport latency and the server-side latency.
static constexpr ExchangeContext::Timeout kExpectedLowProcessingTime  = System::Clock::Seconds16(2);
static constexpr ExchangeContext::Timeout kExpectedHighProcessingTime = System::Clock::Seconds16(30);

PASESession::~PASESession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void PASESession::OnSessionReleased()
{
    // Call into our super-class before we clear our state.
    PairingSession::OnSessionReleased();
    Clear();
}

void PASESession::Finish()
{
    mPairingComplete = true;
    PairingSession::Finish();
}

void PASESession::Clear()
{
    MATTER_TRACE_SCOPE("Clear", "PASESession");
    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    memset(&mPASEVerifier, 0, sizeof(mPASEVerifier));
    mNextExpectedMsg.ClearValue();

    mSpake2p.Clear();
    mCommissioningHash.Clear();

    mIterationCount = 0;
    mSaltLength     = 0;
    if (mSalt != nullptr)
    {
        chip::Platform::MemoryFree(mSalt);
        mSalt = nullptr;
    }
    mPairingComplete = false;
    PairingSession::Clear();
}

CHIP_ERROR PASESession::Init(SessionManager & sessionManager, uint32_t setupCode, SessionEstablishmentDelegate * delegate)
{
    MATTER_TRACE_SCOPE("Init", "PASESession");
    VerifyOrReturnError(sessionManager.GetSessionKeystore() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Reset any state maintained by PASESession object (in case it's being reused for pairing)
    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ Uint8::from_const_char(kSpake2pContext), strlen(kSpake2pContext) }));

    mDelegate = delegate;
    ReturnErrorOnFailure(AllocateSecureSession(sessionManager));
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ChipLogDetail(SecureChannel, "Assigned local session key ID %u", GetLocalSessionId().Value());

    VerifyOrReturnError(setupCode < (1 << kSetupPINCodeFieldLengthInBits), CHIP_ERROR_INVALID_ARGUMENT);
    mSetupPINCode = setupCode;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::GeneratePASEVerifier(Spake2pVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                             bool useRandomPIN, uint32_t & setupPINCode)
{
    MATTER_TRACE_SCOPE("GeneratePASEVerifier", "PASESession");

    if (useRandomPIN)
    {
        ReturnErrorOnFailure(SetupPayload::generateRandomSetupPin(setupPINCode));
    }

    return verifier.Generate(pbkdf2IterCount, salt, setupPINCode);
}

CHIP_ERROR PASESession::SetupSpake2p()
{
    MATTER_TRACE_SCOPE("SetupSpake2p", "PASESession");
    uint8_t context[kSHA256_Hash_Length] = { 0 };
    MutableByteSpan contextSpan{ context };

    ReturnErrorOnFailure(mCommissioningHash.Finish(contextSpan));
    ReturnErrorOnFailure(mSpake2p.Init(contextSpan.data(), contextSpan.size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::WaitForPairing(SessionManager & sessionManager, const Spake2pVerifier & verifier, uint32_t pbkdf2IterCount,
                                       const ByteSpan & salt, Optional<ReliableMessageProtocolConfig> mrpLocalConfig,
                                       SessionEstablishmentDelegate * delegate)
{
    // Return early on error here, as we have not initialized any state yet
    VerifyOrReturnError(!salt.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt.data() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length && salt.size() <= kSpake2p_Max_PBKDF_Salt_Length,
                        CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = Init(sessionManager, kSetupPINCodeUndefinedValue, delegate);
    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExit(err);

    mRole = CryptoContext::SessionRole::kResponder;

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
    memmove(&mPASEVerifier, &verifier, sizeof(verifier));

    mIterationCount = pbkdf2IterCount;
    mNextExpectedMsg.SetValue(MsgType::PBKDFParamRequest);
    mPairingComplete = false;
    mLocalMRPConfig  = MakeOptional(mrpLocalConfig.ValueOr(GetDefaultMRPConfig()));

    ChipLogDetail(SecureChannel, "Waiting for PBKDF param request");

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR PASESession::Pair(SessionManager & sessionManager, uint32_t peerSetUpPINCode,
                             Optional<ReliableMessageProtocolConfig> mrpLocalConfig, Messaging::ExchangeContext * exchangeCtxt,
                             SessionEstablishmentDelegate * delegate)
{
    MATTER_TRACE_SCOPE("Pair", "PASESession");
    VerifyOrReturnError(exchangeCtxt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CHIP_ERROR err = Init(sessionManager, peerSetUpPINCode, delegate);
    SuccessOrExit(err);

    mRole = CryptoContext::SessionRole::kInitiator;

    mExchangeCtxt.Emplace(*exchangeCtxt);

    // When commissioning starts, the peer is assumed to be active.
    mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->MarkActiveRx();

    mExchangeCtxt.Value()->UseSuggestedResponseTimeout(kExpectedLowProcessingTime);

    mLocalMRPConfig = MakeOptional(mrpLocalConfig.ValueOr(GetDefaultMRPConfig()));

    err = SendPBKDFParamRequest();
    SuccessOrExit(err);

    mDelegate->OnSessionEstablishmentStarted();

exit:
    if (err != CHIP_NO_ERROR)
    {
        // If a failure happens before we have placed the incoming exchange into `mExchangeCtxt`, we need to make
        // sure to close the exchange to fulfill our API contract.
        if (!mExchangeCtxt.HasValue())
        {
            exchangeCtxt->Close();
        }
        Clear();
        ChipLogError(SecureChannel, "Failed during PASE session pairing request: %" CHIP_ERROR_FORMAT, err.Format());
        MATTER_TRACE_COUNTER("PASEFail");
    }
    return err;
}

void PASESession::OnResponseTimeout(ExchangeContext * ec)
{
    MATTER_TRACE_SCOPE("OnResponseTimeout", "PASESession");
    VerifyOrReturn(ec != nullptr, ChipLogError(SecureChannel, "PASESession::OnResponseTimeout was called by null exchange"));
    VerifyOrReturn(!mExchangeCtxt.HasValue() || &mExchangeCtxt.Value().Get() == ec,
                   ChipLogError(SecureChannel, "PASESession::OnResponseTimeout exchange doesn't match"));
    // If we were waiting for something, mNextExpectedMsg had better have a value.
    ChipLogError(SecureChannel, "PASESession timed out while waiting for a response from the peer. Expected message type was %u",
                 to_underlying(mNextExpectedMsg.Value()));
    MATTER_TRACE_COUNTER("PASETimeout");
    // Discard the exchange so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    DiscardExchange();
    Clear();
    // Do this last in case the delegate frees us.
    NotifySessionEstablishmentError(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR PASESession::DeriveSecureSession(CryptoContext & session)
{
    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);

    SessionKeystore & keystore = *mSessionManager->GetSessionKeystore();
    AutoReleaseSymmetricKey<HkdfKeyHandle> hkdfKey(keystore);

    ReturnErrorOnFailure(mSpake2p.GetKeys(keystore, hkdfKey.KeyHandle()));
    ReturnErrorOnFailure(session.InitFromSecret(keystore, hkdfKey.KeyHandle(), ByteSpan{} /* salt */,
                                                CryptoContext::SessionInfoType::kSessionEstablishment, mRole));

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::ReadSessionParamsIfPresent(const TLV::Tag & expectedSessionParamsTag,
                                                   System::PacketBufferTLVReader & tlvReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == expectedSessionParamsTag)
    {
        ReturnErrorOnFailure(DecodeSessionParametersIfPresent(expectedSessionParamsTag, tlvReader, mRemoteSessionParams));
        mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            GetRemoteSessionParameters());

        err = tlvReader.Next();
    }
    return err;
}

CHIP_ERROR PASESession::SendPBKDFParamRequest()
{
    MATTER_TRACE_SCOPE("SendPBKDFParamRequest", "PASESession");

    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(DRBG_get_bytes(mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));

    const size_t max_msg_len = TLV::EstimateStructOverhead(kPBKDFParamRandomNumberSize,         // initiatorRandom,
                                                           sizeof(uint16_t),                    // initiatorSessionId
                                                           sizeof(PasscodeId),                  // passcodeId,
                                                           sizeof(uint8_t),                     // hasPBKDFParameters
                                                           SessionParameters::kEstimatedTLVSize // Session Parameters
    );

    System::PacketBufferHandle req = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!req.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(req));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorRandom), mPBKDFLocalRandomData,
                                            sizeof(mPBKDFLocalRandomData)));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorSessionId), GetLocalSessionId().Value()));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(PBKDFParamRequestTags::kPasscodeId), kDefaultCommissioningPasscodeId));
    ReturnErrorOnFailure(tlvWriter.PutBoolean(AsTlvContextTag(PBKDFParamRequestTags::kHasPBKDFParameters), mHavePBKDFParameters));

    VerifyOrReturnError(mLocalMRPConfig.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(EncodeSessionParameters(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorSessionParams),
                                                 mLocalMRPConfig.Value(), tlvWriter));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&req));

    // Update commissioning hash with the pbkdf2 param request that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ req->Start(), req->DataLength() }));

    ReturnErrorOnFailure(mExchangeCtxt.Value()->SendMessage(MsgType::PBKDFParamRequest, std::move(req),
                                                            SendFlags(SendMessageFlags::kExpectResponse)));

    mNextExpectedMsg.SetValue(MsgType::PBKDFParamResponse);

#if CHIP_PROGRESS_LOGGING
    const auto localMRPConfig = mLocalMRPConfig.Value();
#endif // CHIP_PROGRESS_LOGGING
    ChipLogProgress(SecureChannel, "Sent PBKDF param request [II:%" PRIu32 "ms AI:%" PRIu32 "ms AT:%ums)",
                    localMRPConfig.mIdleRetransTimeout.count(), localMRPConfig.mActiveRetransTimeout.count(),
                    localMRPConfig.mActiveThresholdTime.count());

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamRequest(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandlePBKDFParamRequest", "PASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t initiatorSessionId;
    uint8_t initiatorRandom[kPBKDFParamRandomNumberSize];

    PasscodeId passcodeId   = kDefaultCommissioningPasscodeId;
    bool hasPBKDFParameters = false;

    ChipLogDetail(SecureChannel, "Received PBKDF param request");

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorRandom)));
    VerifyOrExit(tlvReader.GetLength() == kPBKDFParamRandomNumberSize, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(initiatorRandom, sizeof(initiatorRandom)));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorSessionId)));
    SuccessOrExit(err = tlvReader.Get(initiatorSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session ID %d", initiatorSessionId);
    SetPeerSessionId(initiatorSessionId);

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamRequestTags::kPasscodeId)));
    SuccessOrExit(err = tlvReader.Get(passcodeId));
    VerifyOrExit(passcodeId == kDefaultCommissioningPasscodeId, err = CHIP_ERROR_INVALID_PASE_PARAMETER);

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamRequestTags::kHasPBKDFParameters)));
    SuccessOrExit(err = tlvReader.Get(hasPBKDFParameters));

    err = ReadSessionParamsIfPresent(AsTlvContextTag(PBKDFParamRequestTags::kInitiatorSessionParams), tlvReader);

    // Future-proofing: CHIP_NO_ERROR will be returned by Next() within ReadSessionParamsIfPresent() if we have additional
    // non-parsed TLV Elements, which could happen in the future if additional elements are added to the specification.
    VerifyOrExit(err == CHIP_END_OF_TLV || err == CHIP_NO_ERROR, /* No Action */);

    // ExitContainer() acts as a safeguard to ensure that the received encoded message is properly terminated with an EndOfContainer
    // TLV element. It is called as an extra validation step to enforce input data structure integrity. Without it, the message may
    // still parse correctly, but malformed or incomplete data might go undetected.
    // ExitContainer() will return CHIP_END_OF_TLV if the EndOfContainer TLV element terminator is missing.
    SuccessOrExit(err = tlvReader.ExitContainer(containerType));

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
    MATTER_TRACE_SCOPE("SendPBKDFParamResponse", "PASESession");

    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(DRBG_get_bytes(mPBKDFLocalRandomData, sizeof(mPBKDFLocalRandomData)));

    const size_t max_msg_len =
        TLV::EstimateStructOverhead(kPBKDFParamRandomNumberSize,                                // initiatorRandom
                                    kPBKDFParamRandomNumberSize,                                // responderRandom
                                    sizeof(uint16_t),                                           // responderSessionId
                                    TLV::EstimateStructOverhead(sizeof(uint32_t), mSaltLength), // pbkdf_parameters
                                    SessionParameters::kEstimatedTLVSize                        // Session Parameters
        );

    System::PacketBufferHandle resp = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!resp.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(resp));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    // The initiator random value is being sent back in the response as required by the specifications
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(PBKDFParamResponseTags::kInitiatorRandom), initiatorRandom));
    ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(PBKDFParamResponseTags::kResponderRandom), mPBKDFLocalRandomData,
                                            sizeof(mPBKDFLocalRandomData)));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(PBKDFParamResponseTags::kResponderSessionId), GetLocalSessionId().Value()));

    if (!initiatorHasPBKDFParams)
    {
        TLV::TLVType pbkdfParamContainer;
        ReturnErrorOnFailure(tlvWriter.StartContainer(AsTlvContextTag(PBKDFParamResponseTags::kPbkdfParameters),
                                                      TLV::kTLVType_Structure, pbkdfParamContainer));
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(PBKDFParameterSetTags::kIterations), mIterationCount));
        ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(PBKDFParameterSetTags::kSalt), mSalt, mSaltLength));
        ReturnErrorOnFailure(tlvWriter.EndContainer(pbkdfParamContainer));
    }

    VerifyOrReturnError(mLocalMRPConfig.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(EncodeSessionParameters(AsTlvContextTag(PBKDFParamResponseTags::kResponderSessionParams),
                                                 mLocalMRPConfig.Value(), tlvWriter));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&resp));

    // Update commissioning hash with the pbkdf2 param response that's being sent.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ resp->Start(), resp->DataLength() }));
    ReturnErrorOnFailure(SetupSpake2p());

    ReturnErrorOnFailure(mExchangeCtxt.Value()->SendMessage(MsgType::PBKDFParamResponse, std::move(resp),
                                                            SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent PBKDF param response");

    mNextExpectedMsg.SetValue(MsgType::PASE_Pake1);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandlePBKDFParamResponse(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandlePBKDFParamResponse", "PASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t responderSessionId;
    uint8_t random[kPBKDFParamRandomNumberSize];

    ByteSpan salt;
    uint8_t serializedWS[kSpake2p_WS_Length * 2] = { 0 };

    ChipLogDetail(SecureChannel, "Received PBKDF param response");

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Initiator's random value
    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamResponseTags::kInitiatorRandom)));
    SuccessOrExit(err = tlvReader.GetBytes(random, sizeof(random)));
    VerifyOrExit(ByteSpan(random).data_equal(ByteSpan(mPBKDFLocalRandomData)), err = CHIP_ERROR_INVALID_PASE_PARAMETER);

    // Responder's random value
    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamResponseTags::kResponderRandom)));
    VerifyOrExit(tlvReader.GetLength() == kPBKDFParamRandomNumberSize, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(random, sizeof(random)));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamResponseTags::kResponderSessionId)));
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    if (mHavePBKDFParameters)
    {
        err = ReadSessionParamsIfPresent(AsTlvContextTag(PBKDFParamResponseTags::kResponderSessionParams), tlvReader);

        // Future-proofing: CHIP_NO_ERROR will be returned by Next() within ReadSessionParamsIfPresent() if we have additional
        // non-parsed TLV Elements, which could happen in the future if additional elements are added to the specification.
        VerifyOrExit(err == CHIP_END_OF_TLV || err == CHIP_NO_ERROR, /* No Action */);

        // TODO - Add a unit test that exercises mHavePBKDFParameters path
        salt = ByteSpan(mSalt, mSaltLength);
    }
    else
    {
        SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParamResponseTags::kPbkdfParameters)));
        SuccessOrExit(err = tlvReader.EnterContainer(containerType));

        SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParameterSetTags::kIterations)));
        SuccessOrExit(err = tlvReader.Get(mIterationCount));

        SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(PBKDFParameterSetTags::kSalt)));
        VerifyOrExit(tlvReader.GetLength() >= kSpake2p_Min_PBKDF_Salt_Length &&
                         tlvReader.GetLength() <= kSpake2p_Max_PBKDF_Salt_Length,
                     err = CHIP_ERROR_INVALID_TLV_ELEMENT);
        SuccessOrExit(err = tlvReader.Get(salt));

        SuccessOrExit(err = tlvReader.ExitContainer(containerType));

        err = ReadSessionParamsIfPresent(AsTlvContextTag(PBKDFParamResponseTags::kResponderSessionParams), tlvReader);

        // Future-proofing: CHIP_NO_ERROR will be returned by Next() within ReadSessionParamsIfPresent() if we have additional
        // non-parsed TLV Elements, which could happen in the future if additional elements are added to the specification.
        VerifyOrExit(err == CHIP_END_OF_TLV || err == CHIP_NO_ERROR, /* No Action */);
    }

    // ExitContainer() acts as a safeguard to ensure that the received encoded message is properly terminated with an EndOfContainer
    // TLV element. It is called as an extra validation step to enforce input data structure integrity. Without it, the message may
    // still parse correctly, but malformed or incomplete data might go undetected.
    // ExitContainer() will return CHIP_END_OF_TLV if the EndOfContainer TLV element terminator is missing.
    SuccessOrExit(err = tlvReader.ExitContainer(containerType));

    err = SetupSpake2p();
    SuccessOrExit(err);

    err = Spake2pVerifier::ComputeWS(mIterationCount, salt, mSetupPINCode, serializedWS, sizeof(serializedWS));
    SuccessOrExit(err);

    err = mSpake2p.BeginProver(nullptr, 0, nullptr, 0, &serializedWS[0], kSpake2p_WS_Length, &serializedWS[kSpake2p_WS_Length],
                               kSpake2p_WS_Length);
    SuccessOrExit(err);

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
    MATTER_TRACE_SCOPE("SendMsg1", "PASESession");
    const size_t max_msg_len       = TLV::EstimateStructOverhead(kMAX_Point_Length);
    System::PacketBufferHandle msg = System::PacketBufferHandle::New(max_msg_len);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));

    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);

    ReturnErrorOnFailure(mSpake2p.ComputeRoundOne(nullptr, 0, X, &X_len));
    VerifyOrReturnError(X_len == sizeof(X), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Pake1Tags::kPa), ByteSpan(X)));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg));

    ReturnErrorOnFailure(
        mExchangeCtxt.Value()->SendMessage(MsgType::PASE_Pake1, std::move(msg), SendFlags(SendMessageFlags::kExpectResponse)));
    ChipLogDetail(SecureChannel, "Sent spake2p msg1");

    mNextExpectedMsg.SetValue(MsgType::PASE_Pake2);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::HandleMsg1_and_SendMsg2(System::PacketBufferHandle && msg1)
{
    MATTER_TRACE_SCOPE("HandleMsg1_and_SendMsg2", "PASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    ChipLogDetail(SecureChannel, "Received spake2p msg1");
    MATTER_TRACE_SCOPE("Pake1", "PASESession");

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * X;
    size_t X_len = 0;

    tlvReader.Init(std::move(msg1));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(Pake1Tags::kPa)));
    X_len = tlvReader.GetLength();
    VerifyOrExit(X_len == kMAX_Point_Length, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetDataPtr(X));

    SuccessOrExit(err = tlvReader.ExitContainer(containerType));

    SuccessOrExit(err = mSpake2p.BeginVerifier(nullptr, 0, nullptr, 0, mPASEVerifier.mW0, kP256_FE_Length, mPASEVerifier.mL,
                                               kP256_Point_Length));

    SuccessOrExit(err = mSpake2p.ComputeRoundOne(X, X_len, Y, &Y_len));
    VerifyOrReturnError(Y_len == sizeof(Y), CHIP_ERROR_INTERNAL);
    SuccessOrExit(err = mSpake2p.ComputeRoundTwo(X, X_len, verifier, &verifier_len));
    msg1 = nullptr;

    {
        const size_t max_msg_len = TLV::EstimateStructOverhead(Y_len, verifier_len);

        System::PacketBufferHandle msg2 = System::PacketBufferHandle::New(max_msg_len);
        VerifyOrExit(!msg2.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        System::PacketBufferTLVWriter tlvWriter;
        tlvWriter.Init(std::move(msg2));

        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(AsTlvContextTag(Pake2Tags::kPb), ByteSpan(Y)));
        SuccessOrExit(err = tlvWriter.Put(AsTlvContextTag(Pake2Tags::kCb), ByteSpan(verifier, verifier_len)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg2));

        err =
            mExchangeCtxt.Value()->SendMessage(MsgType::PASE_Pake2, std::move(msg2), SendFlags(SendMessageFlags::kExpectResponse));
        SuccessOrExit(err);

        mNextExpectedMsg.SetValue(MsgType::PASE_Pake3);
    }

    ChipLogDetail(SecureChannel, "Sent spake2p msg2");
    MATTER_TRACE_COUNTER("Pake2");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR PASESession::HandleMsg2_and_SendMsg3(System::PacketBufferHandle && msg2)
{
    MATTER_TRACE_SCOPE("HandleMsg2_and_SendMsg3", "PASESession");
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

    tlvReader.Init(std::move(msg2));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(Pake2Tags::kPb)));
    Y_len = tlvReader.GetLength();
    VerifyOrExit(Y_len == kMAX_Point_Length, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetDataPtr(Y));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(Pake2Tags::kCb)));
    peer_verifier_len = tlvReader.GetLength();
    VerifyOrExit(peer_verifier_len == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetDataPtr(peer_verifier));

    // ExitContainer() acts as a safeguard to ensure that the received encoded message is properly terminated with an EndOfContainer
    // TLV element. It is called as an extra validation step to enforce input data structure integrity. Without it, the message may
    // still parse correctly, but malformed or incomplete data might go undetected.
    // ExitContainer() will return CHIP_END_OF_TLV if the EndOfContainer TLV element terminator is missing.
    SuccessOrExit(err = tlvReader.ExitContainer(containerType));

    SuccessOrExit(err = mSpake2p.ComputeRoundTwo(Y, Y_len, verifier, &verifier_len));

    SuccessOrExit(err = mSpake2p.KeyConfirm(peer_verifier, peer_verifier_len));
    msg2 = nullptr;

    {
        const size_t max_msg_len = TLV::EstimateStructOverhead(verifier_len);

        System::PacketBufferHandle msg3 = System::PacketBufferHandle::New(max_msg_len);
        VerifyOrExit(!msg3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        System::PacketBufferTLVWriter tlvWriter;
        tlvWriter.Init(std::move(msg3));

        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(AsTlvContextTag(Pake3Tags::kCa), ByteSpan(verifier, verifier_len)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg3));

        err =
            mExchangeCtxt.Value()->SendMessage(MsgType::PASE_Pake3, std::move(msg3), SendFlags(SendMessageFlags::kExpectResponse));
        SuccessOrExit(err);

        mNextExpectedMsg.SetValue(MsgType::StatusReport);
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
    MATTER_TRACE_SCOPE("HandleMsg3", "PASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(SecureChannel, "Received spake2p msg3");
    MATTER_TRACE_COUNTER("Pake3");

    mNextExpectedMsg.ClearValue();

    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * peer_verifier;
    size_t peer_verifier_len = 0;

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next(AsTlvContextTag(Pake3Tags::kCa)));
    peer_verifier_len = tlvReader.GetLength();
    VerifyOrExit(peer_verifier_len == kMAX_Hash_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    SuccessOrExit(err = tlvReader.GetDataPtr(peer_verifier));

    // ExitContainer() acts as a safeguard to ensure that the received encoded message is properly terminated with an EndOfContainer
    // TLV element. It is called as an extra validation step to enforce input data structure integrity. Without it, the message may
    // still parse correctly, but malformed or incomplete data might go undetected.
    // ExitContainer() will return CHIP_END_OF_TLV if the EndOfContainer TLV element terminator is missing.
    SuccessOrExit(err = tlvReader.ExitContainer(containerType));

    SuccessOrExit(err = mSpake2p.KeyConfirm(peer_verifier, peer_verifier_len));

    // Send confirmation to peer that we succeeded so they can start using the session.
    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    Finish();
exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

void PASESession::OnSuccessStatusReport()
{
    Finish();
}

CHIP_ERROR PASESession::OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode,
                                              Optional<uintptr_t> protocolData)
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
    ChipLogError(SecureChannel, "Received error (protocol code %d) during PASE process: %" CHIP_ERROR_FORMAT, protocolCode,
                 err.Format());
    return err;
}

CHIP_ERROR PASESession::ValidateReceivedMessage(ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                                const System::PacketBufferHandle & msg)
{
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (PBKDFParamRequest) received by PASESession
    // via UnsolicitedMessageHandler. The exchange context is allocated by exchange manager and provided
    // to the handler (PASESession object).
    if (mExchangeCtxt.HasValue())
    {
        if (&mExchangeCtxt.Value().Get() != exchange)
        {
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        mExchangeCtxt.Emplace(*exchange);
    }

    if (!mExchangeCtxt.Value()->GetSessionHandle()->IsUnauthenticatedSession())
    {
        ChipLogError(SecureChannel, "PASESession received PBKDFParamRequest over encrypted session.  Ignoring.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mExchangeCtxt.Value()->UseSuggestedResponseTimeout(kExpectedHighProcessingTime);

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((mNextExpectedMsg.HasValue() && payloadHeader.HasMessageType(mNextExpectedMsg.Value())) ||
                            payloadHeader.HasMessageType(MsgType::StatusReport),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // Handle messages by myself
    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PASESession::OnMessageReceived(ExchangeContext * exchange, const PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("OnMessageReceived", "PASESession");
    CHIP_ERROR err  = ValidateReceivedMessage(exchange, payloadHeader, msg);
    MsgType msgType = static_cast<MsgType>(payloadHeader.GetMessageType());
    SuccessOrExit(err);

#if CHIP_CONFIG_SLOW_CRYPTO
    if (msgType == MsgType::PBKDFParamRequest || msgType == MsgType::PBKDFParamResponse || msgType == MsgType::PASE_Pake1 ||
        msgType == MsgType::PASE_Pake2 || msgType == MsgType::PASE_Pake3)
    {
        SuccessOrExit(err = mExchangeCtxt.Value()->FlushAcks());
    }
#endif // CHIP_CONFIG_SLOW_CRYPTO

    switch (msgType)
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
        err =
            HandleStatusReport(std::move(msg), mNextExpectedMsg.HasValue() && (mNextExpectedMsg.Value() == MsgType::StatusReport));
        break;

    default:
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    };

exit:

    // Call delegate to indicate pairing failure
    if (err != CHIP_NO_ERROR)
    {
        // Discard the exchange so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        DiscardExchange();
        Clear();
        ChipLogError(SecureChannel, "Failed during PASE session setup: %" CHIP_ERROR_FORMAT, err.Format());
        MATTER_TRACE_COUNTER("PASEFail");
        // Do this last in case the delegate frees us.
        NotifySessionEstablishmentError(err);
    }
    return err;
}

} // namespace chip
