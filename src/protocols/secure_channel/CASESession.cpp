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
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 *
 */
#include <protocols/secure_channel/CASESession.h>

#include <inttypes.h>
#include <string.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <protocols/Protocols.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>
#include <support/TypeTraits.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;

constexpr uint8_t kKDFSR2Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32 };
constexpr uint8_t kKDFSR3Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33 };
constexpr size_t kKDFInfoLength   = sizeof(kKDFSR2Info);
constexpr uint8_t kKDFSEInfo[]    = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };
constexpr size_t kKDFSEInfoLength = sizeof(kKDFSEInfo);

constexpr uint8_t kTBEData2_Nonce[] =
    /* "NCASE_Sigma2N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x4e };
constexpr uint8_t kTBEData3_Nonce[] =
    /* "NCASE_Sigma3N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33, 0x4e };
constexpr size_t kTBEDataNonceLength = sizeof(kTBEData2_Nonce);
static_assert(sizeof(kTBEData2_Nonce) == sizeof(kTBEData3_Nonce), "TBEData2_Nonce and TBEData3_Nonce must be same size");

// TODO: move this constant over to src/crypto/CHIPCryptoPAL.h - name it CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES
constexpr size_t kTAGSize = 16;

constexpr size_t kDestinationMessageLen = kSigmaParamRandomNumberSize + kP256_PublicKey_Length + sizeof(FabricId) + sizeof(NodeId);

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

// Wait at most 10 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received within timeout window.
static constexpr ExchangeContext::Timeout kSigma_Response_Timeout = 10000;

CASESession::CASESession()
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
    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;
    mCommissioningHash.Clear();
    mPairingComplete = false;
    mConnectionState.Reset();

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

CHIP_ERROR CASESession::Serialize(CASESessionSerialized & output)
{
    uint16_t serializedLen = 0;
    CASESessionSerializable serializable;

    VerifyOrReturnError(BASE64_ENCODED_LEN(sizeof(serializable)) <= sizeof(output.inner), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(ToSerializable(serializable));

    serializedLen = chip::Base64Encode(Uint8::to_const_uchar(reinterpret_cast<uint8_t *>(&serializable)),
                                       static_cast<uint16_t>(sizeof(serializable)), Uint8::to_char(output.inner));
    VerifyOrReturnError(serializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(serializedLen < sizeof(output.inner), CHIP_ERROR_INVALID_ARGUMENT);
    output.inner[serializedLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Deserialize(CASESessionSerialized & input)
{
    CASESessionSerializable serializable;
    size_t maxlen            = BASE64_ENCODED_LEN(sizeof(serializable));
    size_t len               = strnlen(Uint8::to_char(input.inner), maxlen);
    uint16_t deserializedLen = 0;

    VerifyOrReturnError(len < sizeof(CASESessionSerialized), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_INVALID_ARGUMENT);

    memset(&serializable, 0, sizeof(serializable));
    deserializedLen =
        Base64Decode(Uint8::to_const_char(input.inner), static_cast<uint16_t>(len), Uint8::to_uchar((uint8_t *) &serializable));

    VerifyOrReturnError(deserializedLen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(deserializedLen <= sizeof(serializable), CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(FromSerializable(serializable));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ToSerializable(CASESessionSerializable & serializable)
{
    const NodeId peerNodeId = mConnectionState.GetPeerNodeId();
    VerifyOrReturnError(CanCastTo<uint16_t>(mSharedSecret.Length()), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(CanCastTo<uint16_t>(sizeof(mMessageDigest)), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(CanCastTo<uint16_t>(sizeof(mIPK)), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(CanCastTo<uint64_t>(peerNodeId), CHIP_ERROR_INTERNAL);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mSharedSecretLen  = static_cast<uint16_t>(mSharedSecret.Length());
    serializable.mMessageDigestLen = static_cast<uint16_t>(sizeof(mMessageDigest));
    serializable.mIPKLen           = static_cast<uint16_t>(sizeof(mIPK));
    serializable.mPairingComplete  = (mPairingComplete) ? 1 : 0;
    serializable.mPeerNodeId       = peerNodeId;
    serializable.mLocalKeyId       = mConnectionState.GetLocalKeyID();
    serializable.mPeerKeyId        = mConnectionState.GetPeerKeyID();

    memcpy(serializable.mSharedSecret, mSharedSecret, mSharedSecret.Length());
    memcpy(serializable.mMessageDigest, mMessageDigest, sizeof(mMessageDigest));
    memcpy(serializable.mIPK, mIPK, sizeof(mIPK));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FromSerializable(const CASESessionSerializable & serializable)
{
    mPairingComplete = (serializable.mPairingComplete == 1);
    ReturnErrorOnFailure(mSharedSecret.SetLength(static_cast<size_t>(serializable.mSharedSecretLen)));

    VerifyOrReturnError(serializable.mMessageDigestLen <= sizeof(mMessageDigest), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(serializable.mIPKLen <= sizeof(mIPK), CHIP_ERROR_INVALID_ARGUMENT);

    memset(mSharedSecret, 0, sizeof(mSharedSecret.Capacity()));
    memcpy(mSharedSecret, serializable.mSharedSecret, mSharedSecret.Length());
    memcpy(mMessageDigest, serializable.mMessageDigest, serializable.mMessageDigestLen);
    memcpy(mIPK, serializable.mIPK, serializable.mIPKLen);

    mConnectionState.SetPeerNodeId(serializable.mPeerNodeId);
    mConnectionState.SetLocalKeyID(serializable.mLocalKeyId);
    mConnectionState.SetPeerKeyID(serializable.mPeerKeyId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Init(OperationalCredentialSet * operationalCredentialSet, uint16_t myKeyId,
                             SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(operationalCredentialSet != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(operationalCredentialSet->GetCertCount() > 0, CHIP_ERROR_CERT_NOT_FOUND);

    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());

    mDelegate = delegate;
    mConnectionState.SetLocalKeyID(myKeyId);
    mOpCredSet = operationalCredentialSet;

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
CASESession::ListenForSessionEstablishment(OperationalCredentialSet * operationalCredentialSet, uint16_t myKeyId,
                                           SessionEstablishmentDelegate * delegate)
{
    ReturnErrorOnFailure(Init(operationalCredentialSet, myKeyId, delegate));

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR1;
    mPairingComplete = false;

    ChipLogDetail(SecureChannel, "Waiting for SigmaR1 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::EstablishSession(const Transport::PeerAddress peerAddress,
                                         OperationalCredentialSet * operationalCredentialSet, uint8_t opCredSetIndex,
                                         NodeId peerNodeId, uint16_t myKeyId, ExchangeContext * exchangeCtxt,
                                         SessionEstablishmentDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Return early on error here, as we have not initalized any state yet
    ReturnErrorCodeIf(exchangeCtxt == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    err = Init(operationalCredentialSet, myKeyId, delegate);

    // We are setting the exchange context specifically before checking for error.
    // This is to make sure the exchange will get closed if Init() returned an error.
    mExchangeCtxt = exchangeCtxt;

    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExit(err);

    mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout);
    mConnectionState.SetPeerAddress(peerAddress);
    mConnectionState.SetPeerNodeId(peerNodeId);
    mTrustedRootId = operationalCredentialSet->GetTrustedRootId(opCredSetIndex);
    VerifyOrExit(!mTrustedRootId.empty(), err = CHIP_ERROR_INTERNAL);

    err = SendSigmaR1();
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
    ChipLogError(SecureChannel,
                 "CASESession timed out while waiting for a response from the peer. Expected message type was %" PRIu8,
                 to_underlying(mNextExpectedMsg));
    mDelegate->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
    // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
    // exchange will handle that.
    mExchangeCtxt = nullptr;
    Clear();
}

CHIP_ERROR CASESession::DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role)
{
    size_t saltlen;

    (void) kKDFSEInfo;
    (void) kKDFSEInfoLength;

    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);

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
                                                SecureSession::SessionInfoType::kSessionEstablishment, role));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigmaR1()
{
    uint16_t data_len = EstimateTLVStructOverhead(
        static_cast<uint16_t>(kSigmaParamRandomNumberSize + sizeof(uint16_t) + kSHA256_Hash_Length + kP256_PublicKey_Length), 4);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R1;
    TLV::TLVType outerContainerType                      = TLV::kTLVType_NotSpecified;
    uint8_t initiatorRandom[kSigmaParamRandomNumberSize] = { 0 };
    uint8_t destinationIdentifier[kSHA256_Hash_Length]   = { 0 };

    // Generate an ephemeral keypair
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    mEphemeralKey.SetKeyId(CASE_EPHEMERAL_KEY);
#endif
    ReturnErrorOnFailure(mEphemeralKey.Initialize());

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(initiatorRandom, kSigmaParamRandomNumberSize));

    // Construct Sigma1 Msg
    msg_R1 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R1));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), initiatorRandom, sizeof(initiatorRandom)));
    // Retrieve Session Identifier
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), mConnectionState.GetLocalKeyID(), true));
    // Generate a Destination Identifier
    {
        const ChipCertificateData * rootCertificate = mOpCredSet->GetRootCertificate(mTrustedRootId);
        VerifyOrReturnError(rootCertificate != nullptr, CHIP_ERROR_CERT_NOT_FOUND);
        VerifyOrReturnError(!rootCertificate->mPublicKey.empty(), CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(rootCertificate->mPublicKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);
        ChipCertificateData nodeOperationalCertificate;
        FabricId fabricId;
        MutableByteSpan destinationIdSpan(destinationIdentifier);

        ReturnErrorOnFailure(DecodeChipCert(mOpCredSet->GetDevOpCred(mTrustedRootId), mOpCredSet->GetDevOpCredLen(mTrustedRootId),
                                            nodeOperationalCertificate));
        ReturnErrorOnFailure(nodeOperationalCertificate.mSubjectDN.GetCertFabricId(fabricId));
        // retrieve Fabric IPK
        MutableByteSpan ipkSpan(mIPK);
        ReturnErrorOnFailure(RetrieveIPK(fabricId, ipkSpan));
        ReturnErrorOnFailure(GenerateDestinationID(ByteSpan(initiatorRandom), rootCertificate->mPublicKey,
                                                   mConnectionState.GetPeerNodeId(), fabricId, ByteSpan(mIPK), destinationIdSpan));
    }
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), destinationIdentifier, sizeof(destinationIdentifier)));

    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(4), mEphemeralKey.Pubkey(), static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R1));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msg_R1->Start(), msg_R1->DataLength() }));

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR2;

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_SigmaR1, std::move(msg_R1),
                                                    SendFlags(SendMessageFlags::kExpectResponse)));

    ChipLogDetail(SecureChannel, "Sent SigmaR1 msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigmaR1_and_SendSigmaR2(System::PacketBufferHandle & msg)
{
    ReturnErrorOnFailure(HandleSigmaR1(msg));
    ReturnErrorOnFailure(SendSigmaR2());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigmaR1(System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t initiatorSessionId = 0;
    uint8_t destinationIdentifier[kSHA256_Hash_Length];
    uint8_t initiatorRandom[kSigmaParamRandomNumberSize];

    uint32_t decodeTagIdSeq = 0;

    ChipLogDetail(SecureChannel, "Received SigmaR1 msg");

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

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", initiatorSessionId);
    mConnectionState.SetPeerKeyID(initiatorSessionId);

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(destinationIdentifier, sizeof(destinationIdentifier)));

    {
        const ByteSpan * ipkListSpan = GetIPKList();
        SuccessOrExit(err = FindDestinationIdCandidate(ByteSpan(destinationIdentifier), ByteSpan(initiatorRandom), ipkListSpan,
                                                       GetIPKListEntries()));
    }

    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::SendSigmaR2()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg_R2;
    uint16_t data_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_rand;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    uint16_t msg_r2_signed_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    uint16_t msg_r2_signed_enc_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    uint8_t sr2k[kAEADKeySize];
    P256ECDSASignature tbsData2Signature;

    HKDF_sha_crypto mHKDF;

    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(msg_rand.Alloc(kSigmaParamRandomNumberSize), err = CHIP_ERROR_NO_MEMORY);

    // Fill in the random value
    err = DRBG_get_bytes(msg_rand.Get(), kSigmaParamRandomNumberSize);
    SuccessOrExit(err);

    // Generate an ephemeral keypair
#ifdef ENABLE_HSM_CASE_EPHEMERAL_KEY
    mEphemeralKey.SetKeyId(CASE_EPHEMERAL_KEY);
#endif
    err = mEphemeralKey.Initialize();
    SuccessOrExit(err);

    // Generate a Shared Secret
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR2(ByteSpan(msg_rand.Get(), kSigmaParamRandomNumberSize), mEphemeralKey.Pubkey(), ByteSpan(mIPK),
                                   saltSpan);
        SuccessOrExit(err);
    }

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Construct Sigma2 TBS Data
    msg_r2_signed_len = EstimateTLVStructOverhead(
        static_cast<uint16_t>(mOpCredSet->GetDevOpCredLen(mTrustedRootId) + kP256_PublicKey_Length * 2), 3);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R2_Signed.Get(), msg_r2_signed_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(1), mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(2), mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(3), mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r2_signed_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Generate a Signature
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R2_Signed.Get(), msg_r2_signed_len, tbsData2Signature);
    SuccessOrExit(err);

    // Construct Sigma2 TBE Data
    msg_r2_signed_enc_len = EstimateTLVStructOverhead(
        static_cast<uint16_t>(mOpCredSet->GetDevOpCredLen(mTrustedRootId) + tbsData2Signature.Length()), 2);

    VerifyOrExit(msg_R2_Encrypted.Alloc(msg_r2_signed_enc_len + kTAGSize), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(1), mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(
            err = tlvWriter.PutBytes(TLV::ContextTag(2), tbsData2Signature, static_cast<uint32_t>(tbsData2Signature.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r2_signed_enc_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Generate the encrypted data blob
    err = AES_CCM_encrypt(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len, nullptr, 0, sr2k, kAEADKeySize, kTBEData2_Nonce,
                          kTBEDataNonceLength, msg_R2_Encrypted.Get(), msg_R2_Encrypted.Get() + msg_r2_signed_enc_len, kTAGSize);
    SuccessOrExit(err);

    // Construct Sigma2 Msg
    data_len = EstimateTLVStructOverhead(static_cast<uint16_t>(kSigmaParamRandomNumberSize + sizeof(uint16_t) +
                                                               kP256_PublicKey_Length + msg_r2_signed_enc_len + kTAGSize),
                                         4);

    msg_R2 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R2.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R2));
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(1), msg_rand.Get(), kSigmaParamRandomNumberSize));
        SuccessOrExit(err = tlvWriter.Put(TLV::ContextTag(2), mConnectionState.GetLocalKeyID(), true));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(3), mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(4), msg_R2_Encrypted.Get(),
                                               static_cast<uint32_t>(msg_r2_signed_enc_len + kTAGSize)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg_R2));
    }

    err = mCommissioningHash.AddData(ByteSpan{ msg_R2->Start(), msg_R2->DataLength() });
    SuccessOrExit(err);

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR3;

    // Call delegate to send the msg to peer
    err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_SigmaR2, std::move(msg_R2),
                                     SendFlags(SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Sent SigmaR2 msg");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR2_and_SendSigmaR3(System::PacketBufferHandle & msg)
{
    ReturnErrorOnFailure(HandleSigmaR2(msg));
    ReturnErrorOnFailure(SendSigmaR3());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigmaR2(System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader tlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf = msg->Start();
    size_t buflen       = msg->DataLength();

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    size_t msg_r2_encrypted_len          = 0;
    size_t msg_r2_encrypted_len_with_tag = 0;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    uint16_t msg_r2_signed_len;

    uint8_t sr2k[kAEADKeySize];

    P256ECDSASignature tbsData2Signature;

    P256PublicKey remoteCredential;

    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;

    uint16_t responderSessionId = 0;

    HKDF_sha_crypto mHKDF;

    uint32_t decodeTagIdSeq = 0;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    ChipLogDetail(SecureChannel, "Received SigmaR2 msg");

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Retrieve Responder's Random value
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(responderRandom, sizeof(responderRandom)));

    // Assign Session Key ID
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.Get(responderSessionId));

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", responderSessionId);
    mConnectionState.SetPeerKeyID(responderSessionId);

    // Retrieve Responder's Ephemeral Pubkey
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    SuccessOrExit(err = tlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));

    // Generate a Shared Secret
    SuccessOrExit(err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    // Generate the S2K key
    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        SuccessOrExit(err = ConstructSaltSigmaR2(ByteSpan(responderRandom), mRemotePubKey, ByteSpan(mIPK), saltSpan));
    }

    SuccessOrExit(err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR2Info,
                                          kKDFInfoLength, sr2k, kAEADKeySize));

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, buflen }));

    // Generate decrypted data
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(msg_R2_Encrypted.Alloc(tlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r2_encrypted_len_with_tag = tlvReader.GetLength();
    VerifyOrExit(msg_r2_encrypted_len_with_tag > kTAGSize, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(msg_R2_Encrypted.Get(), static_cast<uint32_t>(msg_r2_encrypted_len_with_tag)));
    msg_r2_encrypted_len = msg_r2_encrypted_len_with_tag - kTAGSize;

    SuccessOrExit(err = AES_CCM_decrypt(msg_R2_Encrypted.Get(), msg_r2_encrypted_len, nullptr, 0,
                                        msg_R2_Encrypted.Get() + msg_r2_encrypted_len, kTAGSize, sr2k, kAEADKeySize,
                                        kTBEData2_Nonce, kTBEDataNonceLength, msg_R2_Encrypted.Get()));

    decodeTagIdSeq = 0;
    decryptedDataTlvReader.Init(msg_R2_Encrypted.Get(), static_cast<uint32_t>(msg_r2_encrypted_len));
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    responderOpCertLen = static_cast<uint16_t>(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(responderOpCert, responderOpCertLen));

    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    SuccessOrExit(err = Validate_and_RetrieveResponderID(ByteSpan(responderOpCert, responderOpCertLen), remoteCredential));

    // Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_r2_signed_len =
        EstimateTLVStructOverhead(static_cast<uint16_t>(sizeof(uint16_t) + responderOpCertLen + kP256_PublicKey_Length * 2), 3);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBS2Data(ByteSpan(responderOpCert, responderOpCertLen), msg_R2_Signed.Get(), msg_r2_signed_len));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData2Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData2Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData2Signature, static_cast<uint32_t>(tbsData2Signature.Length())));

    // Validate signature
    SuccessOrExit(err = remoteCredential.ECDSA_validate_msg_signature(msg_R2_Signed.Get(), msg_r2_signed_len, tbsData2Signature));

exit:
    if (err == CHIP_ERROR_INVALID_SIGNATURE)
    {
        SendErrorMsg(SigmaErrorType::kInvalidSignature);
    }
    else if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::SendSigmaR3()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    MutableByteSpan messageDigestSpan(mMessageDigest);
    System::PacketBufferHandle msg_R3;
    uint16_t data_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    uint16_t msg_r3_encrypted_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    uint8_t sr3k[kAEADKeySize];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    uint16_t msg_r3_signed_len;

    P256ECDSASignature tbsData3Signature;

    HKDF_sha_crypto mHKDF;

    ChipLogDetail(SecureChannel, "Sending SigmaR3");

    saltlen = kIPKSize + kSHA256_Hash_Length;
    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR3(ByteSpan(mIPK), saltSpan);
        SuccessOrExit(err);
    }

    // Prepare SigmaR3 TBS Data Blob
    msg_r3_signed_len = EstimateTLVStructOverhead(
        static_cast<uint16_t>(mOpCredSet->GetDevOpCredLen(mTrustedRootId) + kP256_PublicKey_Length * 2), 3);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Signed.Get(), msg_r3_signed_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(1), mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(2), mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(3), mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r3_signed_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Generate a signature
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature);
    SuccessOrExit(err);

    // Prepare SigmaR3 TBE Data Blob
    msg_r3_encrypted_len = EstimateTLVStructOverhead(
        static_cast<uint16_t>(mOpCredSet->GetDevOpCredLen(mTrustedRootId) + static_cast<uint16_t>(tbsData3Signature.Length())), 2);

    VerifyOrExit(msg_R3_Encrypted.Alloc(msg_r3_encrypted_len + kTAGSize), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(TLV::ContextTag(1), mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(
            err = tlvWriter.PutBytes(TLV::ContextTag(2), tbsData3Signature, static_cast<uint32_t>(tbsData3Signature.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r3_encrypted_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Generate S3K key
    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Generated Encrypted data blob
    err = AES_CCM_encrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0, sr3k, kAEADKeySize, kTBEData3_Nonce,
                          kTBEDataNonceLength, msg_R3_Encrypted.Get(), msg_R3_Encrypted.Get() + msg_r3_encrypted_len, kTAGSize);
    SuccessOrExit(err);

    // Generate Sigma3 Msg
    data_len = EstimateTLVStructOverhead(static_cast<uint16_t>(kTAGSize + msg_r3_encrypted_len), 1);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err =
            tlvWriter.PutBytes(TLV::ContextTag(1), msg_R3_Encrypted.Get(), static_cast<uint32_t>(msg_r3_encrypted_len + kTAGSize));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3);
        SuccessOrExit(err);
    }

    err = mCommissioningHash.AddData(ByteSpan{ msg_R3->Start(), msg_R3->DataLength() });
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_SigmaR3, std::move(msg_R3));
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Sent SigmaR3 msg");

    err = mCommissioningHash.Finish(messageDigestSpan);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR3(System::PacketBufferHandle & msg)
{
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
    uint16_t msg_r3_signed_len;

    uint8_t sr3k[kAEADKeySize];

    P256ECDSASignature tbsData3Signature;

    P256PublicKey remoteCredential;

    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    HKDF_sha_crypto mHKDF;

    uint32_t decodeTagIdSeq = 0;

    ChipLogDetail(SecureChannel, "Received SigmaR3 msg");

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Fetch encrypted data
    SuccessOrExit(err = tlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(tlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(msg_R3_Encrypted.Alloc(tlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r3_encrypted_len_with_tag = tlvReader.GetLength();
    VerifyOrExit(msg_r3_encrypted_len_with_tag > kTAGSize, err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = tlvReader.GetBytes(msg_R3_Encrypted.Get(), static_cast<uint32_t>(msg_r3_encrypted_len_with_tag)));
    msg_r3_encrypted_len = msg_r3_encrypted_len_with_tag - kTAGSize;

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        SuccessOrExit(err = ConstructSaltSigmaR3(ByteSpan(mIPK), saltSpan));
    }

    SuccessOrExit(err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR3Info,
                                          kKDFInfoLength, sr3k, kAEADKeySize));

    SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, bufLen }));

    // Step 2 - Decrypt data blob
    SuccessOrExit(err = AES_CCM_decrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0,
                                        msg_R3_Encrypted.Get() + msg_r3_encrypted_len, kTAGSize, sr3k, kAEADKeySize,
                                        kTBEData3_Nonce, kTBEDataNonceLength, msg_R3_Encrypted.Get()));

    decodeTagIdSeq = 0;
    decryptedDataTlvReader.Init(msg_R3_Encrypted.Get(), static_cast<uint32_t>(msg_r3_encrypted_len));
    containerType = TLV::kTLVType_Structure;
    SuccessOrExit(err = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = decryptedDataTlvReader.EnterContainer(containerType));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    responderOpCertLen = static_cast<uint16_t>(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(responderOpCert, responderOpCertLen));

    // Step 5/6
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    SuccessOrExit(err = Validate_and_RetrieveResponderID(ByteSpan(responderOpCert, responderOpCertLen), remoteCredential));

    // Step 4 - Construct SigmaR3 TBS Data
    msg_r3_signed_len =
        EstimateTLVStructOverhead(static_cast<uint16_t>(sizeof(uint16_t) + responderOpCertLen + kP256_PublicKey_Length * 2), 3);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = ConstructTBS3Data(ByteSpan(responderOpCert, responderOpCertLen), msg_R3_Signed.Get(), msg_r3_signed_len));

    SuccessOrExit(err = decryptedDataTlvReader.Next());
    VerifyOrExit(TLV::TagNumFromTag(decryptedDataTlvReader.GetTag()) == ++decodeTagIdSeq, err = CHIP_ERROR_INVALID_TLV_TAG);
    VerifyOrExit(tbsData3Signature.Capacity() >= decryptedDataTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    tbsData3Signature.SetLength(decryptedDataTlvReader.GetLength());
    SuccessOrExit(err = decryptedDataTlvReader.GetBytes(tbsData3Signature, static_cast<uint32_t>(tbsData3Signature.Length())));

    // Step 7 - Validate Signature
    SuccessOrExit(err = remoteCredential.ECDSA_validate_msg_signature(msg_R3_Signed.Get(), msg_r3_signed_len, tbsData3Signature));

    SuccessOrExit(err = mCommissioningHash.Finish(messageDigestSpan));

    mPairingComplete = true;

    // Forget our exchange, as no additional messages are expected from the peer
    mExchangeCtxt = nullptr;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:
    if (err == CHIP_ERROR_INVALID_SIGNATURE)
    {
        SendErrorMsg(SigmaErrorType::kInvalidSignature);
    }
    else if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

void CASESession::SendErrorMsg(SigmaErrorType errorCode)
{
    System::PacketBufferHandle msg;
    uint16_t msglen      = sizeof(SigmaErrorMsg);
    SigmaErrorMsg * pMsg = nullptr;

    msg = System::PacketBufferHandle::New(msglen);
    VerifyOrReturn(!msg.IsNull(), ChipLogError(SecureChannel, "Failed to allocate error message"));

    pMsg        = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    pMsg->error = errorCode;

    msg->SetDataLength(msglen);

    VerifyOrReturn(mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_SigmaErr, std::move(msg)) != CHIP_NO_ERROR,
                   ChipLogError(SecureChannel, "Failed to send error message"));
}

CHIP_ERROR CASESession::GenerateDestinationID(const ByteSpan & random, const P256PublicKeySpan & rootPubkey, NodeId nodeId,
                                              FabricId fabricId, const ByteSpan & ipk, MutableByteSpan & destinationId)
{
    HMAC_sha hmac;
    uint8_t destinationMessage[kDestinationMessageLen];

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage, sizeof(destinationMessage));

    bbuf.Put(random.data(), random.size());
    bbuf.Put(rootPubkey.data(), rootPubkey.size());
    bbuf.Put64(fabricId);
    bbuf.Put64(nodeId);

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(hmac.HMAC_SHA256(ipk.data(), ipk.size(), destinationMessage, sizeof(destinationMessage),
                                          destinationId.data(), destinationId.size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FindDestinationIdCandidate(const ByteSpan & destinationId, const ByteSpan & initiatorRandom,
                                                   const ByteSpan * ipkList, size_t ipkListEntries)
{
    uint8_t nCertificateSets = mOpCredSet->GetCertCount();

    for (size_t certChainIdx = 0; certChainIdx < nCertificateSets; ++certChainIdx)
    {
        uint8_t candidate[kSHA256_Hash_Length] = { 0 };
        CertificateKeyId trustedRootId;
        ChipCertificateData nodeOperationalCertificate;
        NodeId nodeId;
        FabricId fabricId;

        trustedRootId = mOpCredSet->GetTrustedRootId(static_cast<uint16_t>(certChainIdx));

        ReturnErrorOnFailure(DecodeChipCert(mOpCredSet->GetDevOpCred(trustedRootId), mOpCredSet->GetDevOpCredLen(trustedRootId),
                                            nodeOperationalCertificate));

        ReturnErrorOnFailure(nodeOperationalCertificate.mSubjectDN.GetCertChipId(nodeId));
        ReturnErrorOnFailure(nodeOperationalCertificate.mSubjectDN.GetCertFabricId(fabricId));

        const ChipCertificateData * rootCertificate = mOpCredSet->GetRootCertificate(trustedRootId);
        VerifyOrReturnError(rootCertificate != nullptr, CHIP_ERROR_CERT_NOT_FOUND);
        VerifyOrReturnError(!rootCertificate->mPublicKey.empty(), CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(rootCertificate->mPublicKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);

        for (size_t ipkIdx = 0; ipkIdx < ipkListEntries; ++ipkIdx)
        {
            MutableByteSpan candidateSpan(candidate);
            ReturnErrorOnFailure(GenerateDestinationID(initiatorRandom, rootCertificate->mPublicKey, nodeId, fabricId,
                                                       ipkList[ipkIdx], candidateSpan));

            if (destinationId.data_equal(candidateSpan))
            {
                VerifyOrReturnError(sizeof(mIPK) == ipkList[ipkIdx].size(), CHIP_ERROR_INTERNAL);
                memcpy(mIPK, ipkList[ipkIdx].data(), ipkList[ipkIdx].size());
                mTrustedRootId = trustedRootId;
                break;
            }
        }
    }

    VerifyOrReturnError(!mTrustedRootId.empty(), CHIP_ERROR_CERT_NOT_TRUSTED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigmaR2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
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

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigmaR3(const ByteSpan & ipk, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Validate_and_RetrieveResponderID(const ByteSpan & responderOpCert, Crypto::P256PublicKey & responderID)
{
    const ChipCertificateData * resultCert = nullptr;

    ChipCertificateSet certSet;
    // Certificate set can contain up to 3 certs (NOC, ICA cert, and Root CA cert)
    ReturnErrorOnFailure(certSet.Init(3));

    Encoding::LittleEndian::BufferWriter bbuf(responderID, responderID.Length());
    ReturnErrorOnFailure(certSet.LoadCert(responderOpCert.data(), static_cast<uint32_t>(responderOpCert.size()),
                                          BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    bbuf.Put(certSet.GetCertSet()[0].mPublicKey.data(), certSet.GetCertSet()[0].mPublicKey.size());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Validate responder identity located in msg_r2_encrypted
    ReturnErrorOnFailure(mOpCredSet->FindCertSet(mTrustedRootId)
                             ->LoadCert(responderOpCert.data(), static_cast<uint32_t>(responderOpCert.size()),
                                        BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    ReturnErrorOnFailure(SetEffectiveTime());
    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    {
        const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

        ReturnErrorOnFailure(mOpCredSet->FindValidCert(mTrustedRootId, subjectDN, subjectKeyId, mValidContext, &resultCert));

        // Now that we have verified that this is a valid cert, try to get the
        // peer's operational identity from it.
        PeerId peerId;
        ReturnErrorOnFailure(ExtractPeerIdFromOpCert(certSet.GetCertSet()[0], &peerId));
        mConnectionState.SetPeerNodeId(peerId.GetNodeId());
    }

    // Release the previously loaded NOC Certificate
    ReturnErrorOnFailure(mOpCredSet->FindCertSet(mTrustedRootId)->ReleaseLastCert());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS2Data(const ByteSpan & responderOpCert, uint8_t * tbsData, uint16_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(1), responderOpCert.data(), static_cast<uint32_t>(responderOpCert.size())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(3), mEphemeralKey.Pubkey(), static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<uint16_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS3Data(const ByteSpan & responderOpCert, uint8_t * tbsData, uint16_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(1), responderOpCert.data(), static_cast<uint32_t>(responderOpCert.size())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(3), mEphemeralKey.Pubkey(), static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<uint16_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::RetrieveIPK(FabricId fabricId, MutableByteSpan & ipk)
{
    memset(ipk.data(), static_cast<int>(fabricId), ipk.size());
    return CHIP_NO_ERROR;
}

// TODO: Remove this and replace with system method to retrieve current time
CHIP_ERROR CASESession::SetEffectiveTime(void)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;

    effectiveTime.Year   = 2021;
    effectiveTime.Month  = 2;
    effectiveTime.Day    = 12;
    effectiveTime.Hour   = 10;
    effectiveTime.Minute = 10;
    effectiveTime.Second = 10;

    return ASN1ToChipEpochTime(effectiveTime, mValidContext.mEffectiveTime);
}

CHIP_ERROR CASESession::HandleErrorMsg(const System::PacketBufferHandle & msg)
{
    ReturnErrorCodeIf(msg->Start() == nullptr || msg->DataLength() != sizeof(SigmaErrorMsg), CHIP_ERROR_MESSAGE_INCOMPLETE);

    static_assert(sizeof(SigmaErrorMsg) == sizeof(uint8_t),
                  "Assuming size of SigmaErrorMsg message is 1 octet, so that endian-ness conversion is not needed");

    SigmaErrorMsg * pMsg = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    ChipLogError(SecureChannel, "Received error (%d) during CASE pairing process", pMsg->error);

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (pMsg->error)
    {
    case SigmaErrorType::kUnsupportedVersion:
        err = CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION;
        break;

    case SigmaErrorType::kInvalidSignature:
    case SigmaErrorType::kInvalidResumptionTag:
    case SigmaErrorType::kUnexpected:
        err = CHIP_ERROR_INVALID_CASE_PARAMETER;
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };

    return err;
}

CHIP_ERROR CASESession::ValidateReceivedMessage(ExchangeContext * ec, const PacketHeader & packetHeader,
                                                const PayloadHeader & payloadHeader, System::PacketBufferHandle & msg)
{
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (CASE_SigmaR1) received by CASESession
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
        mExchangeCtxt->SetResponseTimeout(kSigma_Response_Timeout);
    }

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(payloadHeader.HasMessageType(mNextExpectedMsg) ||
                            payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::CASE_SigmaErr),
                        CHIP_ERROR_INVALID_MESSAGE_TYPE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader,
                                          const PayloadHeader & payloadHeader, System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = ValidateReceivedMessage(ec, packetHeader, payloadHeader, msg);
    SuccessOrExit(err);

    mConnectionState.SetPeerAddress(mMessageDispatch.GetPeerAddress());

    switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
    {
    case Protocols::SecureChannel::MsgType::CASE_SigmaR1:
        err = HandleSigmaR1_and_SendSigmaR2(msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR2:
        err = HandleSigmaR2_and_SendSigmaR3(msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR3:
        err = HandleSigmaR3(msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaErr:
        err = HandleErrorMsg(msg);
        break;

    default:
        SendErrorMsg(SigmaErrorType::kUnexpected);
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        break;
    };

exit:

    // Call delegate to indicate session establishment failure.
    if (err != CHIP_NO_ERROR)
    {
        // Null out mExchangeCtxt so that Clear() doesn't try closing it.  The
        // exchange will handle that.
        mExchangeCtxt = nullptr;
        Clear();
        mDelegate->OnSessionEstablishmentError(err);
    }
    return err;
}

} // namespace chip
