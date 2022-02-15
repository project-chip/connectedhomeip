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
 *      This file implements the the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 *
 */
#include <protocols/secure_channel/CASESession.h>

#include <inttypes.h>
#include <string.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <trace/trace.h>
#include <transport/PairingSession.h>
#include <transport/SessionManager.h>

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;
using namespace Encoding;
using namespace Protocols::SecureChannel;

constexpr uint8_t kKDFSR2Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32 };
constexpr uint8_t kKDFSR3Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33 };
constexpr size_t kKDFInfoLength   = sizeof(kKDFSR2Info);
constexpr uint8_t kKDFSEInfo[]    = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };
constexpr size_t kKDFSEInfoLength = sizeof(kKDFSEInfo);

constexpr uint8_t kKDFS1RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x31, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };
constexpr uint8_t kKDFS2RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };

constexpr uint8_t kResume1MIC_Nonce[] =
    /* "NCASE_SigmaR1" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x31 };
constexpr uint8_t kResume2MIC_Nonce[] =
    /* "NCASE_SigmaR2" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x32 };
constexpr uint8_t kTBEData2_Nonce[] =
    /* "NCASE_Sigma2N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x4e };
constexpr uint8_t kTBEData3_Nonce[] =
    /* "NCASE_Sigma3N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33, 0x4e };
constexpr size_t kTBEDataNonceLength = sizeof(kTBEData2_Nonce);
static_assert(sizeof(kTBEData2_Nonce) == sizeof(kTBEData3_Nonce), "TBEData2_Nonce and TBEData3_Nonce must be same size");

enum
{
    kTag_TBEData_SenderNOC    = 1,
    kTag_TBEData_SenderICAC   = 2,
    kTag_TBEData_Signature    = 3,
    kTag_TBEData_ResumptionID = 4,
};

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

// Wait at most 30 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received within timeout window.
static constexpr ExchangeContext::Timeout kSigma_Response_Timeout = System::Clock::Seconds16(30);

CASESession::CASESession() : PairingSession(Transport::SecureSession::Type::kCASE)
{
    mTrustedRootId = CertificateKeyId();
}

CASESession::~CASESession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void CASESession::Clear()
{
    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    mCommissioningHash.Clear();
    mCASESessionEstablished = false;
    PairingSession::Clear();

    mState = kInitialized;

    CloseExchange();
}

void CASESession::CloseExchange()
{
    if (mExchangeCtxt != nullptr)
    {
        mExchangeCtxt->Close();
        mExchangeCtxt = nullptr;
    }
}

void CASESession::DiscardExchange()
{
    if (mExchangeCtxt != nullptr)
    {
        // Make sure the exchange doesn't try to notify us when it closes,
        // since we might be dead by then.
        mExchangeCtxt->SetDelegate(nullptr);
        // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        mExchangeCtxt = nullptr;
    }
}

CHIP_ERROR CASESession::ToCachable(CASESessionCachable & cachableSession)
{
    const NodeId peerNodeId = GetPeerNodeId();
    VerifyOrReturnError(CanCastTo<uint16_t>(mSharedSecret.Length()), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(CanCastTo<uint64_t>(peerNodeId), CHIP_ERROR_INTERNAL);

    cachableSession.mSharedSecretLen = LittleEndian::HostSwap16(static_cast<uint16_t>(mSharedSecret.Length()));
    cachableSession.mPeerNodeId      = LittleEndian::HostSwap64(peerNodeId);
    for (size_t i = 0; i < cachableSession.mPeerCATs.size(); i++)
    {
        cachableSession.mPeerCATs.values[i] = LittleEndian::HostSwap32(GetPeerCATs().values[i]);
    }
    // TODO: Get the fabric index
    cachableSession.mLocalFabricIndex      = 0;
    cachableSession.mSessionSetupTimeStamp = LittleEndian::HostSwap64(mSessionSetupTimeStamp);

    memcpy(cachableSession.mResumptionId, mResumptionId, sizeof(mResumptionId));
    memcpy(cachableSession.mSharedSecret, mSharedSecret, mSharedSecret.Length());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FromCachable(const CASESessionCachable & cachableSession)
{
    uint16_t length = LittleEndian::HostSwap16(cachableSession.mSharedSecretLen);
    ReturnErrorOnFailure(mSharedSecret.SetLength(static_cast<size_t>(length)));
    memset(mSharedSecret, 0, sizeof(mSharedSecret.Capacity()));
    memcpy(mSharedSecret, cachableSession.mSharedSecret, length);

    SetPeerNodeId(LittleEndian::HostSwap64(cachableSession.mPeerNodeId));
    CATValues peerCATs;
    for (size_t i = 0; i < cachableSession.mPeerCATs.size(); i++)
    {
        peerCATs.values[i] = LittleEndian::HostSwap32(cachableSession.mPeerCATs.values[i]);
    }
    SetPeerCATs(peerCATs);
    SetSessionTimeStamp(LittleEndian::HostSwap64(cachableSession.mSessionSetupTimeStamp));
    // TODO: Set the fabric index correctly
    mLocalFabricIndex = cachableSession.mLocalFabricIndex;

    memcpy(mResumptionId, cachableSession.mResumptionId, sizeof(mResumptionId));

    const ByteSpan * ipkListSpan = GetIPKList();
    VerifyOrReturnError(ipkListSpan->size() == sizeof(mIPK), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mIPK, ipkListSpan->data(), sizeof(mIPK));

    mCASESessionEstablished = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Init(uint16_t localSessionId, SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());

    mDelegate = delegate;
    SetLocalSessionId(localSessionId);

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CASESession::ListenForSessionEstablishment(uint16_t localSessionId, FabricTable * fabrics, SessionEstablishmentDelegate * delegate,
                                           Optional<ReliableMessageProtocolConfig> mrpConfig)
{
    VerifyOrReturnError(fabrics != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Init(localSessionId, delegate));

    mFabricsTable   = fabrics;
    mLocalMRPConfig = mrpConfig;

    mCASESessionEstablished = false;

    ChipLogDetail(SecureChannel, "Waiting for Sigma1 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::EstablishSession(const Transport::PeerAddress peerAddress, FabricInfo * fabric, NodeId peerNodeId,
                                         uint16_t localSessionId, ExchangeContext * exchangeCtxt,
                                         SessionEstablishmentDelegate * delegate, Optional<ReliableMessageProtocolConfig> mrpConfig)
{
    TRACE_EVENT_SCOPE("EstablishSession", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Return early on error here, as we have not initialized any state yet
    ReturnErrorCodeIf(exchangeCtxt == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(fabric == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    err = Init(localSessionId, delegate);

    // We are setting the exchange context specifically before checking for error.
    // This is to make sure the exchange will get closed if Init() returned an error.
    mExchangeCtxt = exchangeCtxt;

    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExit(err);

    mFabricInfo     = fabric;
    mLocalMRPConfig = mrpConfig;

    mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout + mExchangeCtxt->GetSessionHandle()->GetAckTimeout());
    SetPeerAddress(peerAddress);
    SetPeerNodeId(peerNodeId);

    err = SendSigma1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

void CASESession::OnResponseTimeout(ExchangeContext * ec)
{
    VerifyOrReturn(ec != nullptr, ChipLogError(SecureChannel, "CASESession::OnResponseTimeout was called by null exchange"));
    VerifyOrReturn(mExchangeCtxt == ec, ChipLogError(SecureChannel, "CASESession::OnResponseTimeout exchange doesn't match"));
    ChipLogError(SecureChannel, "CASESession timed out while waiting for a response from the peer. Current state was %u", mState);
    // Discard the exchange so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    DiscardExchange();
    Clear();
    // Do this last in case the delegate frees us.
    mDelegate->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR CASESession::DeriveSecureSession(CryptoContext & session, CryptoContext::SessionRole role)
{
    size_t saltlen;

    (void) kKDFSEInfo;
    (void) kKDFSEInfoLength;

    VerifyOrReturnError(mCASESessionEstablished, CHIP_ERROR_INCORRECT_STATE);

    // Generate Salt for Encryption keys
    saltlen = sizeof(mIPK) + kSHA256_Hash_Length;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    ReturnErrorCodeIf(!msg_salt.Alloc(saltlen), CHIP_ERROR_NO_MEMORY);
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt.Get(), saltlen);
        bbuf.Put(mIPK, sizeof(mIPK));
        bbuf.Put(mMessageDigest, sizeof(mMessageDigest));

        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    ReturnErrorOnFailure(session.InitFromSecret(ByteSpan(mSharedSecret, mSharedSecret.Length()), ByteSpan(msg_salt.Get(), saltlen),
                                                CryptoContext::SessionInfoType::kSessionEstablishment, role));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma1()
{
    TRACE_EVENT_SCOPE("SendSigma1", "CASESession");
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t data_len            = TLV::EstimateStructOverhead(kSigmaParamRandomNumberSize, // initiatorRandom
                                                  sizeof(uint16_t),            // initiatorSessionId,
                                                  kSHA256_Hash_Length,         // destinationId
                                                  kP256_PublicKey_Length,      // InitiatorEphPubKey,
                                                  mrpParamsSize,               // initiatorMRPParams
                                                  kCASEResumptionIDSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R1;
    TLV::TLVType outerContainerType                    = TLV::kTLVType_NotSpecified;
    uint8_t destinationIdentifier[kSHA256_Hash_Length] = { 0 };

    // Generate an ephemeral keypair
    ReturnErrorOnFailure(mEphemeralKey.Initialize());

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(mInitiatorRandom, sizeof(mInitiatorRandom)));

    // Construct Sigma1 Msg
    msg_R1 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R1));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), ByteSpan(mInitiatorRandom)));
    // Retrieve Session Identifier
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), GetLocalSessionId()));
    // Generate a Destination Identifier
    {
        MutableByteSpan destinationIdSpan(destinationIdentifier);
        ReturnErrorCodeIf(mFabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);
        memcpy(mIPK, GetIPKList()->data(), sizeof(mIPK));
        ReturnErrorOnFailure(
            mFabricInfo->GenerateDestinationID(ByteSpan(mIPK), ByteSpan(mInitiatorRandom), GetPeerNodeId(), destinationIdSpan));
    }
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), destinationIdentifier, sizeof(destinationIdentifier)));

    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(4), mEphemeralKey.Pubkey(), static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));

    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(5), mLocalMRPConfig.Value(), tlvWriter));
    }

    // If CASE session was previously established using the current state information, let's fill in the session resumption
    // information in the the Sigma1 request. It'll speed up the session establishment process if the peer can resume the old
    // session, since no certificate chains will have to be verified.
    if (mCASESessionEstablished)
    {
        ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(6), mResumptionId, kCASEResumptionIDSize));

        uint8_t initiatorResume1MIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
        MutableByteSpan resumeMICSpan(initiatorResume1MIC);
        ReturnErrorOnFailure(GenerateSigmaResumeMIC(ByteSpan(mInitiatorRandom), ByteSpan(mResumptionId), ByteSpan(kKDFS1RKeyInfo),
                                                    ByteSpan(kResume1MIC_Nonce), resumeMICSpan));

        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(7), resumeMICSpan));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R1));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msg_R1->Start(), msg_R1->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma1, std::move(msg_R1),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = kSentSigma1;

    ChipLogDetail(SecureChannel, "Sent Sigma1 msg");

    mDelegate->OnSessionEstablishmentStarted();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma1_and_SendSigma2(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma1_and_SendSigma2", "CASESession");
    ReturnErrorOnFailure(HandleSigma1(std::move(msg)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma1(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma1", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;

    uint16_t initiatorSessionId;
    ByteSpan destinationIdentifier;
    ByteSpan initiatorRandom;

    ChipLogDetail(SecureChannel, "Received Sigma1 msg");

    bool sessionResumptionRequested = false;
    ByteSpan resumptionId;
    ByteSpan resume1MIC;
    ByteSpan initiatorPubKey;

    const ByteSpan * ipkListSpan = GetIPKList();
    FabricIndex fabricIndex      = kUndefinedFabricIndex;

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = ParseSigma1(tlvReader, initiatorRandom, initiatorSessionId, destinationIdentifier, initiatorPubKey,
                                    sessionResumptionRequested, resumptionId, resume1MIC));

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", initiatorSessionId);
    SetPeerSessionId(initiatorSessionId);

    if (sessionResumptionRequested && resumptionId.data_equal(ByteSpan(mResumptionId)))
    {
        // Cross check resume1MIC with the shared secret
        if (ValidateSigmaResumeMIC(resume1MIC, initiatorRandom, resumptionId, ByteSpan(kKDFS1RKeyInfo),
                                   ByteSpan(kResume1MIC_Nonce)) == CHIP_NO_ERROR)
        {
            // Send Sigma2Resume message to the initiator
            SuccessOrExit(err = SendSigma2Resume(initiatorRandom));

            mDelegate->OnSessionEstablishmentStarted();

            // Early returning here, since we have sent Sigma2Resume, and no further processing is needed for the Sigma1 message
            return CHIP_NO_ERROR;
        }
    }

    memcpy(mIPK, ipkListSpan->data(), sizeof(mIPK));

    VerifyOrExit(mFabricsTable != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    fabricIndex =
        mFabricsTable->FindDestinationIDCandidate(destinationIdentifier, initiatorRandom, ipkListSpan, GetIPKListEntries());
    VerifyOrExit(fabricIndex != kUndefinedFabricIndex, err = CHIP_ERROR_KEY_NOT_FOUND);

    mFabricInfo = mFabricsTable->FindFabricWithIndex(fabricIndex);
    VerifyOrExit(mFabricInfo != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // ParseSigma1 ensures that:
    // mRemotePubKey.Length() == initiatorPubKey.size() == kP256_PublicKey_Length.
    memcpy(mRemotePubKey.Bytes(), initiatorPubKey.data(), mRemotePubKey.Length());

    SuccessOrExit(err = SendSigma2());

    mDelegate->OnSessionEstablishmentStarted();

exit:

    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeNoSharedRoot);
        mState = kInitialized;
    }
    else if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::SendSigma2Resume(const ByteSpan & initiatorRandom)
{
    TRACE_EVENT_SCOPE("SendSigma2Resume", "CASESession");
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t max_sigma2_resume_data_len =
        TLV::EstimateStructOverhead(kCASEResumptionIDSize, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sizeof(uint16_t), mrpParamsSize);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R2_resume;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    msg_R2_resume = System::PacketBufferHandle::New(max_sigma2_resume_data_len);
    VerifyOrReturnError(!msg_R2_resume.IsNull(), CHIP_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R2_resume));

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mResumptionId, sizeof(mResumptionId)));

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(1), ByteSpan(mResumptionId)));

    uint8_t sigma2ResumeMIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];
    MutableByteSpan resumeMICSpan(sigma2ResumeMIC);
    ReturnErrorOnFailure(GenerateSigmaResumeMIC(initiatorRandom, ByteSpan(mResumptionId), ByteSpan(kKDFS2RKeyInfo),
                                                ByteSpan(kResume2MIC_Nonce), resumeMICSpan));

    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), resumeMICSpan));

    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), GetLocalSessionId()));

    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(4), mLocalMRPConfig.Value(), tlvWriter));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R2_resume));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2Resume, std::move(msg_R2_resume),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = kSentSigma2Resume;

    ChipLogDetail(SecureChannel, "Sent Sigma2Resume msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma2()
{
    TRACE_EVENT_SCOPE("SendSigma2", "CASESession");
    VerifyOrReturnError(mFabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ByteSpan icaCert;
    ReturnErrorOnFailure(mFabricInfo->GetICACert(icaCert));

    ByteSpan nocCert;
    ReturnErrorOnFailure(mFabricInfo->GetNOCCert(nocCert));

    ReturnErrorOnFailure(mFabricInfo->GetTrustedRootId(mTrustedRootId));
    VerifyOrReturnError(!mTrustedRootId.empty(), CHIP_ERROR_INTERNAL);

    // Fill in the random value
    uint8_t msg_rand[kSigmaParamRandomNumberSize];
    ReturnErrorOnFailure(DRBG_get_bytes(&msg_rand[0], sizeof(msg_rand)));

    // Generate an ephemeral keypair
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    mEphemeralKey.SetKeyId(CASE_EPHEMERAL_KEY);
#endif
    ReturnErrorOnFailure(mEphemeralKey.Initialize());

    // Generate a Shared Secret
    ReturnErrorOnFailure(mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    uint8_t msg_salt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];

    MutableByteSpan saltSpan(msg_salt);
    ReturnErrorOnFailure(ConstructSaltSigma2(ByteSpan(msg_rand), mEphemeralKey.Pubkey(), ByteSpan(mIPK), saltSpan));

    HKDF_sha_crypto mHKDF;
    uint8_t sr2k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR2Info,
                                           kKDFInfoLength, sr2k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES));

    // Construct Sigma2 TBS Data
    size_t msg_r2_signed_len =
        TLV::EstimateStructOverhead(nocCert.size(), icaCert.size(), kP256_PublicKey_Length, kP256_PublicKey_Length);

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    VerifyOrReturnError(msg_R2_Signed.Alloc(msg_r2_signed_len), CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(ConstructTBSData(nocCert, icaCert, ByteSpan(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length()),
                                          ByteSpan(mRemotePubKey, mRemotePubKey.Length()), msg_R2_Signed.Get(), msg_r2_signed_len));

    // Generate a Signature
    VerifyOrReturnError(mFabricInfo->GetOperationalKey() != nullptr, CHIP_ERROR_INCORRECT_STATE);

    P256ECDSASignature tbsData2Signature;
    ReturnErrorOnFailure(
        mFabricInfo->GetOperationalKey()->ECDSA_sign_msg(msg_R2_Signed.Get(), msg_r2_signed_len, tbsData2Signature));

    msg_R2_Signed.Free();

    // Construct Sigma2 TBE Data
    size_t msg_r2_signed_enc_len =
        TLV::EstimateStructOverhead(nocCert.size(), icaCert.size(), tbsData2Signature.Length(), kCASEResumptionIDSize);

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    VerifyOrReturnError(msg_R2_Encrypted.Alloc(msg_r2_signed_enc_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderNOC), nocCert));
    if (!icaCert.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderICAC), icaCert));
    }
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(kTag_TBEData_Signature), tbsData2Signature,
                                            static_cast<uint32_t>(tbsData2Signature.Length())));

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mResumptionId, sizeof(mResumptionId)));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(kTag_TBEData_ResumptionID), mResumptionId,
                                            static_cast<uint32_t>(sizeof(mResumptionId))));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    msg_r2_signed_enc_len = static_cast<size_t>(tlvWriter.GetLengthWritten());

    // Generate the encrypted data blob
    ReturnErrorOnFailure(AES_CCM_encrypt(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len, nullptr, 0, sr2k,
                                         CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData2_Nonce, kTBEDataNonceLength,
                                         msg_R2_Encrypted.Get(), msg_R2_Encrypted.Get() + msg_r2_signed_enc_len,
                                         CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    // Construct Sigma2 Msg
    const size_t mrpParamsSize = mLocalMRPConfig.HasValue() ? TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t)) : 0;
    size_t data_len            = TLV::EstimateStructOverhead(kSigmaParamRandomNumberSize, sizeof(uint16_t), kP256_PublicKey_Length,
                                                  msg_r2_signed_enc_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, mrpParamsSize);

    System::PacketBufferHandle msg_R2 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R2.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriterMsg2;
    outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriterMsg2.Init(std::move(msg_R2));
    ReturnErrorOnFailure(tlvWriterMsg2.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(TLV::ContextTag(1), &msg_rand[0], sizeof(msg_rand)));
    ReturnErrorOnFailure(tlvWriterMsg2.Put(TLV::ContextTag(2), GetLocalSessionId()));
    ReturnErrorOnFailure(
        tlvWriterMsg2.PutBytes(TLV::ContextTag(3), mEphemeralKey.Pubkey(), static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(TLV::ContextTag(4), msg_R2_Encrypted.Get(),
                                                static_cast<uint32_t>(msg_r2_signed_enc_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES)));
    if (mLocalMRPConfig.HasValue())
    {
        ChipLogDetail(SecureChannel, "Including MRP parameters");
        ReturnErrorOnFailure(EncodeMRPParameters(TLV::ContextTag(5), mLocalMRPConfig.Value(), tlvWriterMsg2));
    }
    ReturnErrorOnFailure(tlvWriterMsg2.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriterMsg2.Finalize(&msg_R2));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msg_R2->Start(), msg_R2->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2, std::move(msg_R2),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    mState = kSentSigma2;

    ChipLogDetail(SecureChannel, "Sent Sigma2 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2Resume(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma2Resume", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t responderSessionId;

    uint32_t decodeTagIdSeq = 0;

    ChipLogDetail(SecureChannel, "Received Sigma2Resume msg");

    uint8_t sigma2ResumeMIC[CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES];

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tlvReader.GetLength() == kCASEResumptionIDSize, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(mResumptionId, kCASEResumptionIDSize));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tlvReader.GetLength() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(sigma2ResumeMIC, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    SuccessOrExit(err = ValidateSigmaResumeMIC(ByteSpan(sigma2ResumeMIC), ByteSpan(mInitiatorRandom), ByteSpan(mResumptionId),
                                               ByteSpan(kKDFS2RKeyInfo), ByteSpan(kResume2MIC_Nonce)));

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    if (tlvReader.Next() != CHIP_END_OF_TLV)
    {
        SuccessOrExit(err = DecodeMRPParametersIfPresent(TLV::ContextTag(4), tlvReader));
    }

    ChipLogDetail(SecureChannel, "Peer assigned session session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    // TODO: Set timestamp on the new session, to allow selecting a least-recently-used session for eviction
    // on running out of session contexts.

    mCASESessionEstablished = true;

    // Discard the exchange so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    DiscardExchange();

    // Call delegate to indicate session establishment is successful
    // Do this last in case the delegate frees us.
    mDelegate->OnSessionEstablished();

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigma2_and_SendSigma3(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma2_and_SendSigma3", "CASESession");
    ReturnErrorOnFailure(HandleSigma2(std::move(msg)));
    ReturnErrorOnFailure(SendSigma3());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma2", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf = msg->Start();
    size_t buflen       = msg->DataLength();

    uint8_t msg_salt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    size_t msg_r2_encrypted_len          = 0;
    size_t msg_r2_encrypted_len_with_tag = 0;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    size_t msg_r2_signed_len;

    uint8_t sr2k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    P256ECDSASignature tbsData2Signature;

    P256PublicKey remoteCredential;

    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    ByteSpan responderNOC;
    ByteSpan responderICAC;

    uint16_t responderSessionId;

    uint32_t decodeTagIdSeq = 0;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    ChipLogDetail(SecureChannel, "Received Sigma2 msg");

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Retrieve Responder's Random value
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(responderRandom, sizeof(responderRandom)));

    // Assign Session ID
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session session ID %d", responderSessionId);
    SetPeerSessionId(responderSessionId);

    // Retrieve Responder's Ephemeral Pubkey
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));

    // Generate a Shared Secret
    SuccessOrExit(err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    // Generate the S2K key
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma2(ByteSpan(responderRandom), mRemotePubKey, ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR2Info,
                                kKDFInfoLength, sr2k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, buflen }));

    // Generate decrypted data
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(msg_R2_Encrypted.Alloc(tlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r2_encrypted_len_with_tag = tlvReader.GetLength();
    VerifyOrExit(msg_r2_encrypted_len_with_tag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(msg_R2_Encrypted.Get(), static_cast<uint32_t>(msg_r2_encrypted_len_with_tag)));
    msg_r2_encrypted_len = msg_r2_encrypted_len_with_tag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

    SuccessOrExit(err = AES_CCM_decrypt(msg_R2_Encrypted.Get(), msg_r2_encrypted_len, nullptr, 0,
                                        msg_R2_Encrypted.Get() + msg_r2_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sr2k,
                                        CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData2_Nonce, kTBEDataNonceLength,
                                        msg_R2_Encrypted.Get()));

    decryptedDataTlvReader.Init(msg_R2_Encrypted.Get(), msg_r2_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_SenderNOC)));
    SuccessOrExit(err = decryptedDataTlvReader.Get(responderNOC));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    if (TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_SenderICAC)
    {
        VerifyOrExit(decryptedDataTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
        SuccessOrExit(err = decryptedDataTlvReader.Get(responderICAC));
        SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_Signature)));
    }

    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    SuccessOrExit(err = Validate_and_RetrieveResponderID(responderNOC, responderICAC, remoteCredential));

    // Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_r2_signed_len = TLV::EstimateStructOverhead(sizeof(uint16_t), responderNOC.size(), responderICAC.size(),
                                                    kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(responderNOC, responderICAC, ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                         ByteSpan(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length()), msg_R2_Signed.Get(),
                                         msg_r2_signed_len));

    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_Signature, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData2Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData2Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData2Signature, tbsData2Signature.Length()));

    // Validate signature
    SuccessOrExit(err = remoteCredential.ECDSA_validate_msg_signature(msg_R2_Signed.Get(), msg_r2_signed_len, tbsData2Signature));

    // Retrieve session resumption ID
    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_ResumptionID)));
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(mResumptionId, static_cast<uint32_t>(sizeof(mResumptionId))));

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    {
        CATValues peerCATs;
        SuccessOrExit(err = ExtractCATsFromOpCert(responderNOC, peerCATs));
        SetPeerCATs(peerCATs);
    }

    // Retrieve responderMRPParams if present
    if (tlvReader.Next() != CHIP_END_OF_TLV)
    {
        SuccessOrExit(err = DecodeMRPParametersIfPresent(TLV::ContextTag(5), tlvReader));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::SendSigma3()
{
    TRACE_EVENT_SCOPE("SendSigma3", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    MutableByteSpan messageDigestSpan(mMessageDigest);
    System::PacketBufferHandle msg_R3;
    size_t data_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    size_t msg_r3_encrypted_len;

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    uint8_t sr3k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    size_t msg_r3_signed_len;

    P256ECDSASignature tbsData3Signature;

    ChipLogDetail(SecureChannel, "Sending Sigma3");

    ByteSpan icaCert;
    ByteSpan nocCert;

    VerifyOrExit(mFabricInfo != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = mFabricInfo->GetICACert(icaCert));
    SuccessOrExit(err = mFabricInfo->GetNOCCert(nocCert));

    SuccessOrExit(err = mFabricInfo->GetTrustedRootId(mTrustedRootId));
    VerifyOrExit(!mTrustedRootId.empty(), err = CHIP_ERROR_INTERNAL);

    // Prepare Sigma3 TBS Data Blob
    msg_r3_signed_len = TLV::EstimateStructOverhead(icaCert.size(), nocCert.size(), kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(nocCert, icaCert, ByteSpan(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length()),
                                         ByteSpan(mRemotePubKey, mRemotePubKey.Length()), msg_R3_Signed.Get(), msg_r3_signed_len));

    // Generate a signature
    VerifyOrExit(mFabricInfo->GetOperationalKey() != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    err = mFabricInfo->GetOperationalKey()->ECDSA_sign_msg(msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature);
    SuccessOrExit(err);

    // Prepare Sigma3 TBE Data Blob
    msg_r3_encrypted_len = TLV::EstimateStructOverhead(nocCert.size(), icaCert.size(), tbsData3Signature.Length());

    VerifyOrExit(msg_R3_Encrypted.Alloc(msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderNOC), nocCert));
        if (!icaCert.empty())
        {
            SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(kTag_TBEData_SenderICAC), icaCert));
        }
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(kTag_TBEData_Signature), tbsData3Signature,
                                               static_cast<uint32_t>(tbsData3Signature.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r3_encrypted_len = static_cast<size_t>(tlvWriter.GetLengthWritten());
    }

    // Generate S3K key
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR3Info,
                                kKDFInfoLength, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    // Generated Encrypted data blob
    err = AES_CCM_encrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES,
                          kTBEData3_Nonce, kTBEDataNonceLength, msg_R3_Encrypted.Get(),
                          msg_R3_Encrypted.Get() + msg_r3_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);
    SuccessOrExit(err);

    // Generate Sigma3 Msg
    data_len = TLV::EstimateStructOverhead(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, msg_r3_encrypted_len);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(TLV::ContextTag(1), msg_R3_Encrypted.Get(),
                                 static_cast<uint32_t>(msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3);
        SuccessOrExit(err);
    }

    err = mCommissioningHash.AddData(ByteSpan{ msg_R3->Start(), msg_R3->DataLength() });
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma3, std::move(msg_R3),
                                     SendFlags(SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Sent Sigma3 msg");

    err = mCommissioningHash.Finish(messageDigestSpan);
    SuccessOrExit(err);

    mState = kSentSigma3;

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigma3(System::PacketBufferHandle && msg)
{
    TRACE_EVENT_SCOPE("HandleSigma3", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    MutableByteSpan messageDigestSpan(mMessageDigest);
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf   = msg->Start();
    const uint16_t bufLen = msg->DataLength();

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    size_t msg_r3_encrypted_len          = 0;
    size_t msg_r3_encrypted_len_with_tag = 0;
    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    size_t msg_r3_signed_len;

    uint8_t sr3k[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    P256ECDSASignature tbsData3Signature;

    P256PublicKey remoteCredential;

    ByteSpan initiatorNOC;
    ByteSpan initiatorICAC;

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    uint32_t decodeTagIdSeq = 0;

    ChipLogDetail(SecureChannel, "Received Sigma3 msg");

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Fetch encrypted data
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(msg_R3_Encrypted.Alloc(tlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r3_encrypted_len_with_tag = tlvReader.GetLength();
    VerifyOrExit(msg_r3_encrypted_len_with_tag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(msg_R3_Encrypted.Get(), static_cast<uint32_t>(msg_r3_encrypted_len_with_tag)));
    msg_r3_encrypted_len = msg_r3_encrypted_len_with_tag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;

    // Step 1
    {
        MutableByteSpan saltSpan(msg_salt);
        err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);

        HKDF_sha_crypto mHKDF;
        err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), saltSpan.data(), saltSpan.size(), kKDFSR3Info,
                                kKDFInfoLength, sr3k, CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        SuccessOrExit(err);
    }

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, bufLen }));

    // Step 2 - Decrypt data blob
    SuccessOrExit(err = AES_CCM_decrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0,
                                        msg_R3_Encrypted.Get() + msg_r3_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, sr3k,
                                        CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, kTBEData3_Nonce, kTBEDataNonceLength,
                                        msg_R3_Encrypted.Get()));

    decryptedDataTlvReader.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag()));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_SenderNOC)));
    SuccessOrExit(err = decryptedDataTlvReader.Get(initiatorNOC));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    if (TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_SenderICAC)
    {
        VerifyOrExit(decryptedDataTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
        SuccessOrExit(err = decryptedDataTlvReader.Get(initiatorICAC));
        SuccessOrExit(err = decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, TLV::ContextTag(kTag_TBEData_Signature)));
    }

    // Step 5/6
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    SuccessOrExit(err = Validate_and_RetrieveResponderID(initiatorNOC, initiatorICAC, remoteCredential));

    // Step 4 - Construct Sigma3 TBS Data
    msg_r3_signed_len = TLV::EstimateStructOverhead(sizeof(uint16_t), initiatorNOC.size(), initiatorICAC.size(),
                                                    kP256_PublicKey_Length, kP256_PublicKey_Length);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBSData(initiatorNOC, initiatorICAC, ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                         ByteSpan(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length()), msg_R3_Signed.Get(),
                                         msg_r3_signed_len));

    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == kTag_TBEData_Signature, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData3Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData3Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData3Signature, tbsData3Signature.Length()));

    // TODO - Validate message signature prior to validating the received operational credentials.
    //        The op cert check requires traversal of cert chain, that is a more expensive operation.
    //        If message signature check fails, the cert chain check will be unnecessary, but with the
    //        current flow of code, a malicious node can trigger a DoS style attack on the device.
    //        The same change should be made in Sigma2 processing.
    // Step 7 - Validate Signature
    SuccessOrExit(err = remoteCredential.ECDSA_validate_msg_signature(msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature));

    SuccessOrExit(err = mCommissioningHash.Finish(messageDigestSpan));

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    {
        CATValues peerCATs;
        SuccessOrExit(err = ExtractCATsFromOpCert(initiatorNOC, peerCATs));
        SetPeerCATs(peerCATs);
    }

    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    // TODO: Set timestamp on the new session, to allow selecting a least-recently-used session for eviction
    // on running out of session contexts.

    mCASESessionEstablished = true;

    // Discard the exchange so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    DiscardExchange();

    // Call delegate to indicate session establishment is successful
    // Do this last in case the delegate frees us.
    mDelegate->OnSessionEstablished();

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::ConstructSaltSigma2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                            MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    bbuf.Put(rand.data(), kSigmaParamRandomNumberSize);
    bbuf.Put(pubkey, pubkey.Length());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigma3(const ByteSpan & ipk, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSigmaResumeKey(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                                const ByteSpan & skInfo, const ByteSpan & nonce, MutableByteSpan & resumeKey)
{
    VerifyOrReturnError(resumeKey.size() >= CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    constexpr size_t saltSize = kSigmaParamRandomNumberSize + kCASEResumptionIDSize;
    uint8_t salt[saltSize];

    memset(salt, 0, saltSize);
    Encoding::LittleEndian::BufferWriter bbuf(salt, saltSize);

    bbuf.Put(initiatorRandom.data(), initiatorRandom.size());
    bbuf.Put(resumptionID.data(), resumptionID.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);

    HKDF_sha_crypto mHKDF;
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), salt, saltWritten, skInfo.data(), skInfo.size(),
                                           resumeKey.data(), CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES));
    resumeKey.reduce_size(CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::GenerateSigmaResumeMIC(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                               const ByteSpan & skInfo, const ByteSpan & nonce, MutableByteSpan & resumeMIC)
{
    VerifyOrReturnError(resumeMIC.size() >= CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t srk[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    MutableByteSpan resumeKey(srk);

    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, resumeKey));

    ReturnErrorOnFailure(AES_CCM_encrypt(nullptr, 0, nullptr, 0, resumeKey.data(), resumeKey.size(), nonce.data(), nonce.size(),
                                         nullptr, resumeMIC.data(), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
    resumeMIC.reduce_size(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateSigmaResumeMIC(const ByteSpan & resumeMIC, const ByteSpan & initiatorRandom,
                                               const ByteSpan & resumptionID, const ByteSpan & skInfo, const ByteSpan & nonce)
{
    VerifyOrReturnError(resumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t srk[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
    MutableByteSpan resumeKey(srk);

    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, resumeKey));

    ReturnErrorOnFailure(AES_CCM_decrypt(nullptr, 0, nullptr, 0, resumeMIC.data(), resumeMIC.size(), resumeKey.data(),
                                         resumeKey.size(), nonce.data(), nonce.size(), nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Validate_and_RetrieveResponderID(const ByteSpan & responderNOC, const ByteSpan & responderICAC,
                                                         Crypto::P256PublicKey & responderID)
{
    ReturnErrorCodeIf(mFabricInfo == nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(SetEffectiveTime());

    PeerId peerId;
    FabricId rawFabricId;
    ReturnErrorOnFailure(
        mFabricInfo->VerifyCredentials(responderNOC, responderICAC, mValidContext, peerId, rawFabricId, responderID));

    SetPeerNodeId(peerId.GetNodeId());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBSData(const ByteSpan & senderNOC, const ByteSpan & senderICAC, const ByteSpan & senderPubKey,
                                         const ByteSpan & receiverPubKey, uint8_t * tbsData, size_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    enum
    {
        kTag_TBSData_SenderNOC      = 1,
        kTag_TBSData_SenderICAC     = 2,
        kTag_TBSData_SenderPubKey   = 3,
        kTag_TBSData_ReceiverPubKey = 4,
    };

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderNOC), senderNOC));
    if (!senderICAC.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderICAC), senderICAC));
    }
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_SenderPubKey), senderPubKey));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kTag_TBSData_ReceiverPubKey), receiverPubKey));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<size_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::RetrieveIPK(FabricId fabricId, MutableByteSpan & ipk)
{
    memset(ipk.data(), static_cast<int>(fabricId), ipk.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::GetHardcodedTime()
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;

    effectiveTime.Year   = 2022;
    effectiveTime.Month  = 1;
    effectiveTime.Day    = 1;
    effectiveTime.Hour   = 10;
    effectiveTime.Minute = 10;
    effectiveTime.Second = 10;

    return ASN1ToChipEpochTime(effectiveTime, mValidContext.mEffectiveTime);
}

CHIP_ERROR CASESession::SetEffectiveTime()
{
    System::Clock::Milliseconds64 currentTimeMS;
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(currentTimeMS);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(
            SecureChannel,
            "The device does not support GetClock_RealTimeMS() API. This will eventually result in CASE session setup failures.");
        // TODO: Remove use of hardcoded time during CASE setup
        return GetHardcodedTime();
    }

    System::Clock::Seconds32 currentTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentTimeMS);
    VerifyOrReturnError(UnixEpochToChipEpochTime(currentTime.count(), mValidContext.mEffectiveTime), CHIP_ERROR_INVALID_TIME);

    return CHIP_NO_ERROR;
}

void CASESession::OnSuccessStatusReport()
{
    ChipLogProgress(SecureChannel, "Success status report received. Session was established");
    mCASESessionEstablished = true;

    // Discard the exchange so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    DiscardExchange();

    mState = kInitialized;

    // TODO: Set timestamp on the new session, to allow selecting a least-recently-used session for eviction
    // on running out of session contexts.

    // Call delegate to indicate pairing completion.
    // Do this last in case the delegate frees us.
    mDelegate->OnSessionEstablished();
}

CHIP_ERROR CASESession::OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (protocolCode)
    {
    case kProtocolCodeInvalidParam:
        err = CHIP_ERROR_INVALID_CASE_PARAMETER;
        break;

    case kProtocolCodeNoSharedRoot:
        err = CHIP_ERROR_NO_SHARED_TRUSTED_ROOT;
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };
    mState = kInitialized;
    ChipLogError(SecureChannel, "Received error (protocol code %d) during pairing process. %s", protocolCode, ErrorStr(err));
    return err;
}

CHIP_ERROR CASESession::ParseSigma1(TLV::ContiguousBufferTLVReader & tlvReader, ByteSpan & initiatorRandom,
                                    uint16_t & initiatorSessionId, ByteSpan & destinationId, ByteSpan & initiatorEphPubKey,
                                    bool & resumptionRequested, ByteSpan & resumptionId, ByteSpan & initiatorResumeMIC)
{
    using namespace TLV;

    constexpr uint8_t kInitiatorRandomTag    = 1;
    constexpr uint8_t kInitiatorSessionIdTag = 2;
    constexpr uint8_t kDestinationIdTag      = 3;
    constexpr uint8_t kInitiatorPubKeyTag    = 4;
    constexpr uint8_t kInitiatorMRPParamsTag = 5;
    constexpr uint8_t kResumptionIDTag       = 6;
    constexpr uint8_t kResume1MICTag         = 7;

    TLVType containerType = kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorRandomTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(initiatorRandom));
    VerifyOrReturnError(initiatorRandom.size() == kSigmaParamRandomNumberSize, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorSessionIdTag)));
    ReturnErrorOnFailure(tlvReader.Get(initiatorSessionId));

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kDestinationIdTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(destinationId));
    VerifyOrReturnError(destinationId.size() == kSHA256_Hash_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(ContextTag(kInitiatorPubKeyTag)));
    ReturnErrorOnFailure(tlvReader.GetByteView(initiatorEphPubKey));
    VerifyOrReturnError(initiatorEphPubKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Optional members start here.
    CHIP_ERROR err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kInitiatorMRPParamsTag))
    {
        ReturnErrorOnFailure(DecodeMRPParametersIfPresent(TLV::ContextTag(kInitiatorMRPParamsTag), tlvReader));
        err = tlvReader.Next();
    }

    bool resumptionIDTagFound = false;
    bool resume1MICTagFound   = false;

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kResumptionIDTag))
    {
        resumptionIDTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(resumptionId));
        VerifyOrReturnError(resumptionId.size() == kCASEResumptionIDSize, CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == ContextTag(kResume1MICTag))
    {
        resume1MICTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(initiatorResumeMIC));
        VerifyOrReturnError(initiatorResumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        // We ran out of struct members, but that's OK, because they were optional.
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    if (resumptionIDTagFound && resume1MICTagFound)
    {
        resumptionRequested = true;
    }
    else if (!resumptionIDTagFound && !resume1MICTagFound)
    {
        resumptionRequested = false;
    }
    else
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateReceivedMessage(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                System::PacketBufferHandle & msg)
{
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (CASE_Sigma1) received by CASESession
    // via UnsolicitedMessageHandler. The exchange context is allocated by exchange manager and provided
    // to the handler (CASESession object).
    if (mExchangeCtxt != nullptr)
    {
        if (mExchangeCtxt != ec)
        {
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        mExchangeCtxt = ec;
        mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout + mExchangeCtxt->GetSessionHandle()->GetAckTimeout());
    }

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && msg)
{
    CHIP_ERROR err                            = ValidateReceivedMessage(ec, payloadHeader, msg);
    Protocols::SecureChannel::MsgType msgType = static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType());
    SuccessOrExit(err);

    // By default, CHIP_ERROR_INVALID_MESSAGE_TYPE is returned if in the current state
    // a message handler is not defined for the received message type.
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;

    switch (mState)
    {
    case kInitialized:
        if (msgType == Protocols::SecureChannel::MsgType::CASE_Sigma1)
        {
            err = HandleSigma1_and_SendSigma2(std::move(msg));
        }
        break;
    case kSentSigma1:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma2:
            err = HandleSigma2_and_SendSigma3(std::move(msg));
            break;

        case Protocols::SecureChannel::MsgType::CASE_Sigma2Resume:
            err = HandleSigma2Resume(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case kSentSigma2:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma3:
            err = HandleSigma3(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case kSentSigma3:
    case kSentSigma2Resume:
        if (msgType == Protocols::SecureChannel::MsgType::StatusReport)
        {
            err = HandleStatusReport(std::move(msg), /* successExpected*/ true);
        }
        break;
    default:
        // Return the default error that was set above
        break;
    };

exit:

    if (err == CHIP_ERROR_INVALID_MESSAGE_TYPE)
    {
        ChipLogError(SecureChannel, "Received message (type %d) cannot be handled in %d state.", to_underlying(msgType), mState);
    }

    // Call delegate to indicate session establishment failure.
    if (err != CHIP_NO_ERROR)
    {
        // Discard the exchange so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        DiscardExchange();
        Clear();
        // Do this last in case the delegate frees us.
        mDelegate->OnSessionEstablishmentError(err);
    }
    return err;
}

} // namespace chip
