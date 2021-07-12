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
#include <transport/SecureSessionMgr.h>

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;

constexpr uint8_t kIPKInfo[] = { 0x49, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x74, 0x79, 0x50, 0x72, 0x6f,
                                 0x74, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79 };

constexpr uint8_t kKDFSR2Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x52, 0x32 };
constexpr uint8_t kKDFSR3Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x52, 0x33 };
constexpr size_t kKDFInfoLength   = sizeof(kKDFSR2Info);
constexpr uint8_t kKDFSEInfo[]    = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };
constexpr size_t kKDFSEInfoLength = sizeof(kKDFSEInfo);

constexpr uint8_t kIVSR2[] = { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x52, 0x32 };
constexpr uint8_t kIVSR3[] = { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x52, 0x33 };
constexpr size_t kIVLength = sizeof(kIVSR2);

constexpr size_t kTAGSize = 16;

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

// Wait at most 10 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received within timeout window.
static constexpr ExchangeContext::Timeout kSigma_Response_Timeout = 10000;

/**
 * \brief
 * A list of registered packet types a.k.a. TLV context-specific tags to be used during CASE protocol.
 */
enum CASETLVTag : uint8_t
{
    /*! \brief Tag 0. Default stub and end of transmission signal. */
    kUnknown = 0,
    /*! \brief Tag 1. The packet contains a random number. */
    kRandom = 1,
    /*! \brief Tag 2. The packet contains the Session ID. */
    kSessionID = 2,
    /*! \brief Tag 3. The packet contains the Destination ID. */
    kDestinationID = 3,
    /*! \brief Tag 4. The packet contains the Initiator's Ephemeral Public Key. */
    kInitiatorEphPubKey = 4,
    /*! \brief Tag 5. The packet contains the Initiator's Ephemeral Public Key. */
    kResponderEphPubKey = 5,
    /*! \brief Tag 6. The packet contains a Node Operational Certificate. */
    kNOC = 6,
    /*! \brief Tag 7. The packet contains a signature. */
    kSignature = 7,
    /*! \brief Tag 8. The packet contains a Trusted Root ID. */
    kTrustedRootID = 8,
    /*! \brief Tag 9. The packet contains an Encrypted data blob. */
    kEncryptedData = 9,
    /*! \brief Tag 10. The packet contains an AEAD Tag. */
    kTag = 10,
    // TODO: Remove tag 11
    /*! \brief Tag 11. The packet contains the total number of Trusted Root IDs. */
    kNumberofTrustedRootIDs = 11,
};

CASESession::CASESession()
{
    mTrustedRootId = CertificateKeyId();
    // dummy initialization REMOVE LATER
    for (size_t i = 0; i < mFabricSecret.Capacity(); i++)
    {
        mFabricSecret[i] = static_cast<uint8_t>(i);
    }
    mFabricSecret.SetLength(mFabricSecret.Capacity());
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
    if (!mTrustedRootId.empty())
    {
        chip::Platform::MemoryFree(const_cast<uint8_t *>(mTrustedRootId.data()));
        mTrustedRootId = CertificateKeyId();
    }

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
    VerifyOrReturnError(CanCastTo<uint64_t>(peerNodeId), CHIP_ERROR_INTERNAL);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mSharedSecretLen  = static_cast<uint16_t>(mSharedSecret.Length());
    serializable.mMessageDigestLen = static_cast<uint16_t>(sizeof(mMessageDigest));
    serializable.mPairingComplete  = (mPairingComplete) ? 1 : 0;
    serializable.mPeerNodeId       = peerNodeId;
    serializable.mLocalKeyId       = mConnectionState.GetLocalKeyID();
    serializable.mPeerKeyId        = mConnectionState.GetPeerKeyID();

    memcpy(serializable.mSharedSecret, mSharedSecret, mSharedSecret.Length());
    memcpy(serializable.mMessageDigest, mMessageDigest, sizeof(mMessageDigest));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FromSerializable(const CASESessionSerializable & serializable)
{
    mPairingComplete = (serializable.mPairingComplete == 1);
    ReturnErrorOnFailure(mSharedSecret.SetLength(static_cast<size_t>(serializable.mSharedSecretLen)));

    VerifyOrReturnError(serializable.mMessageDigestLen <= sizeof(mMessageDigest), CHIP_ERROR_INVALID_ARGUMENT);

    memset(mSharedSecret, 0, sizeof(mSharedSecret.Capacity()));
    memcpy(mSharedSecret, serializable.mSharedSecret, mSharedSecret.Length());
    memcpy(mMessageDigest, serializable.mMessageDigest, serializable.mMessageDigestLen);

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
                                         OperationalCredentialSet * operationalCredentialSet, NodeId peerNodeId, uint16_t myKeyId,
                                         ExchangeContext * exchangeCtxt, SessionEstablishmentDelegate * delegate)
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
                 static_cast<std::underlying_type_t<decltype(mNextExpectedMsg)>>(mNextExpectedMsg));
    mDelegate->OnSessionEstablishmentError(CHIP_ERROR_TIMEOUT);
    Clear();
}

CHIP_ERROR CASESession::DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role)
{
    uint16_t saltlen;

    (void) kKDFSEInfo;
    (void) kKDFSEInfoLength;

    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);

    // Generate Salt for Encryption keys
    saltlen = kSHA256_Hash_Length;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    ReturnErrorCodeIf(!msg_salt.Alloc(saltlen), CHIP_ERROR_NO_MEMORY);
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt.Get(), saltlen);
        // TODO: Add IPK to Salt
        bbuf.Put(mMessageDigest, sizeof(mMessageDigest));

        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);
    }

    ReturnErrorOnFailure(session.InitFromSecret(ByteSpan(mSharedSecret, mSharedSecret.Length()), ByteSpan(msg_salt.Get(), saltlen),
                                                SecureSession::SessionInfoType::kSessionEstablishment, role));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigmaR1()
{
    uint16_t data_len =
        static_cast<uint16_t>(kSigmaParamRandomNumberSize + sizeof(uint16_t) + sizeof(uint16_t) +
                              mOpCredSet->GetCertCount() * kTrustedRootIdSize + kP256_PublicKey_Length + sizeof(uint64_t) * 4);

    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R1;
    TLV::TLVType outerContainerType                      = TLV::kTLVType_NotSpecified;
    uint8_t initiatorRandom[kSigmaParamRandomNumberSize] = { 0 };

    msg_R1 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_ERROR_NO_MEMORY);

    // Step 1
    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(initiatorRandom, kSigmaParamRandomNumberSize));

// Step 4
#ifdef ENABLE_HSM_CASE_EPHERMAL_KEY
    mEphemeralKey.SetKeyId(CASE_EPHEMERAL_KEY);
#endif
    ReturnErrorOnFailure(mEphemeralKey.Initialize());

    // Start writing TLV
    tlvWriter.Init(std::move(msg_R1));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kRandom, initiatorRandom, sizeof(initiatorRandom)));

    // Step 5
    uint16_t n_trusted_roots = mOpCredSet->GetCertCount();
    // Initiator's session ID
    ReturnErrorOnFailure(tlvWriter.Put(CASETLVTag::kSessionID, mConnectionState.GetLocalKeyID(), true));
    // Step 2/3
    ReturnErrorOnFailure(tlvWriter.Put(CASETLVTag::kNumberofTrustedRootIDs, n_trusted_roots, true));
    for (uint16_t i = 0; i < n_trusted_roots; ++i)
    {
        CertificateKeyId trustedRootId = mOpCredSet->GetTrustedRootId(i);
        if (!trustedRootId.empty())
        {
            ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kTrustedRootID, trustedRootId.data(), trustedRootId.size()));
        }
    }
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R1));

    ReturnErrorOnFailure(mCommissioningHash.AddData(msg_R1->Start(), msg_R1->DataLength()));

    ReturnErrorOnFailure(ComputeIPK(mConnectionState.GetLocalKeyID(), mIPK, sizeof(mIPK)));

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
    System::PacketBufferTLVReader suppTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    uint16_t encryptionKeyId = 0;
    uint32_t n_trusted_roots;

    ChipLogDetail(SecureChannel, "Received SigmaR1 msg");

    err = mCommissioningHash.AddData(msg->Start(), msg->DataLength());
    SuccessOrExit(err);

    tlvReader.Init(std::move(msg));
    err = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kSessionID, suppTlvReader);
    SuccessOrExit(err);
    err = suppTlvReader.Get(encryptionKeyId);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kNumberofTrustedRootIDs, suppTlvReader);
    SuccessOrExit(err);
    err = suppTlvReader.Get(n_trusted_roots);
    SuccessOrExit(err);

    // Step 1/2
    err = FindValidTrustedRoot(tlvReader, n_trusted_roots);
    SuccessOrExit(err);

    // write public key from message
    err = tlvReader.FindElementWithTag(CASETLVTag::kInitiatorEphPubKey, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(mRemotePubKey.Length() == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length()));
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    mConnectionState.SetPeerKeyID(encryptionKeyId);

exit:

    if (err == CHIP_ERROR_CERT_NOT_TRUSTED)
    {
        SendErrorMsg(SigmaErrorType::kNoSharedTrustRoots);
    }
    else if (err != CHIP_NO_ERROR)
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
    P256ECDSASignature sigmaR2Signature;

    uint8_t tag[kTAGSize];

    HKDF_sha_crypto mHKDF;

    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(msg_rand.Alloc(kSigmaParamRandomNumberSize), err = CHIP_ERROR_NO_MEMORY);

    // Step 1
    // Fill in the random value
    err = DRBG_get_bytes(msg_rand.Get(), kSigmaParamRandomNumberSize);
    SuccessOrExit(err);

    // Step 3
    // hardcoded to use a p256keypair
#ifdef ENABLE_HSM_CASE_EPHERMAL_KEY
    mEphemeralKey.SetKeyId(CASE_EPHEMERAL_KEY);
#endif
    err = mEphemeralKey.Initialize();
    SuccessOrExit(err);

    // Step 4
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    err = ComputeIPK(mConnectionState.GetLocalKeyID(), mIPK, sizeof(mIPK));
    SuccessOrExit(err);

    // Step 5
    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR2(ByteSpan(msg_rand.Get(), kSigmaParamRandomNumberSize), mEphemeralKey.Pubkey(), mIPK,
                                   sizeof(mIPK), saltSpan);
        SuccessOrExit(err);
    }

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Step 6
    msg_r2_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + mOpCredSet->GetDevOpCredLen(mTrustedRootId) +
                                              kP256_PublicKey_Length * 2 + sizeof(uint64_t) * 3);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    // Generate Sigma2 TBS Data
    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R2_Signed.Get(), msg_r2_signed_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mRemotePubKey,
                                               static_cast<uint32_t>(mRemotePubKey.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r2_signed_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Step 7
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R2_Signed.Get(), msg_r2_signed_len, sigmaR2Signature);
    SuccessOrExit(err);

    // Step 8
    msg_r2_signed_enc_len = static_cast<uint16_t>(sizeof(uint16_t) + mOpCredSet->GetDevOpCredLen(mTrustedRootId) +
                                                  sigmaR2Signature.Length() + sizeof(uint64_t) * 2);

    VerifyOrExit(msg_R2_Encrypted.Alloc(msg_r2_signed_enc_len), err = CHIP_ERROR_NO_MEMORY);

    // Generate Sigma2 TBE Data
    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len);
        SuccessOrExit(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(
            err = tlvWriter.PutBytes(CASETLVTag::kSignature, sigmaR2Signature, static_cast<uint32_t>(sigmaR2Signature.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
    }

    // Step 9
    err = AES_CCM_encrypt(msg_R2_Encrypted.Get(), msg_r2_signed_enc_len, nullptr, 0, sr2k, kAEADKeySize, kIVSR2, kIVLength,
                          msg_R2_Encrypted.Get(), tag, sizeof(tag));
    SuccessOrExit(err);

    data_len = static_cast<uint16_t>(kSigmaParamRandomNumberSize + sizeof(uint16_t) + kTrustedRootIdSize + kP256_PublicKey_Length +
                                     msg_r2_signed_enc_len + sizeof(tag) + sizeof(uint64_t) * 6);

    msg_R2 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R2.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Step 10
    // now construct sigmaR2
    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R2));
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kRandom, msg_rand.Get(), kSigmaParamRandomNumberSize));
        SuccessOrExit(err = tlvWriter.Put(CASETLVTag::kSessionID, mConnectionState.GetLocalKeyID(), true));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kTrustedRootID, mTrustedRootId.data(),
                                               static_cast<uint32_t>(mTrustedRootId.size())));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kEncryptedData, msg_R2_Encrypted.Get(), msg_r2_signed_enc_len));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kTag, tag, sizeof(tag)));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize(&msg_R2));
    }

    err = mCommissioningHash.AddData(msg_R2->Start(), msg_R2->DataLength());
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
    System::PacketBufferTLVReader suppTlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf = msg->Start();
    size_t buflen       = msg->DataLength();

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Encrypted;
    uint16_t msg_r2_encrypted_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R2_Signed;
    uint16_t msg_r2_signed_len;

    uint8_t sr2k[kAEADKeySize];

    P256ECDSASignature sigmaR2SignedData;

    P256PublicKey remoteCredential;

    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;

    uint8_t tag[kTAGSize];

    uint16_t encryptionKeyId = 0;

    HKDF_sha_crypto mHKDF;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    ChipLogDetail(SecureChannel, "Received SigmaR2 msg");

    tlvReader.Init(std::move(msg));
    SuccessOrExit(err = tlvReader.Next(containerType, TLV::AnonymousTag));
    SuccessOrExit(err = tlvReader.EnterContainer(containerType));

    // Assign Session Key ID
    SuccessOrExit(err = tlvReader.FindElementWithTag(CASETLVTag::kSessionID, suppTlvReader));
    SuccessOrExit(err = suppTlvReader.Get(encryptionKeyId));

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    mConnectionState.SetPeerKeyID(encryptionKeyId);

    // Retrieve Responder's Random value
    err = tlvReader.FindElementWithTag(CASETLVTag::kRandom, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(kSigmaParamRandomNumberSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(responderRandom, sizeof(responderRandom));
    SuccessOrExit(err);

    SuccessOrExit(err = FindValidTrustedRoot(tlvReader, 1));

    // Retrieve Responder's Ephemeral Pubkey
    SuccessOrExit(err = tlvReader.FindElementWithTag(CASETLVTag::kResponderEphPubKey, suppTlvReader));
    VerifyOrExit(mRemotePubKey.Length() == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    SuccessOrExit(err = suppTlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));

    // Step 2
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    // Step 3
    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    err = ComputeIPK(mConnectionState.GetPeerKeyID(), mRemoteIPK, sizeof(mRemoteIPK));
    SuccessOrExit(err);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR2(ByteSpan(responderRandom, sizeof(responderRandom)), mRemotePubKey, mRemoteIPK,
                                   sizeof(mRemoteIPK), saltSpan);
        SuccessOrExit(err);
    }

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                            kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(buf, buflen);
    SuccessOrExit(err);

    // Step 4
    err = tlvReader.FindElementWithTag(CASETLVTag::kEncryptedData, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(msg_R2_Encrypted.Alloc(suppTlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r2_encrypted_len = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                  = suppTlvReader.GetBytes(msg_R2_Encrypted.Get(), msg_r2_encrypted_len);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kTag, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(kTAGSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    err = suppTlvReader.GetBytes(tag, sizeof(tag));
    SuccessOrExit(err);

    err = AES_CCM_decrypt(msg_R2_Encrypted.Get(), msg_r2_encrypted_len, nullptr, 0, tag, kTAGSize, sr2k, kAEADKeySize, kIVSR2,
                          kIVLength, msg_R2_Encrypted.Get());
    SuccessOrExit(err);

    decryptedDataTlvReader.Init(msg_R2_Encrypted.Get(), msg_r2_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    err           = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = decryptedDataTlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = decryptedDataTlvReader.FindElementWithTag(CASETLVTag::kNOC, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    responderOpCertLen = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                = suppTlvReader.GetBytes(responderOpCert, responderOpCertLen);
    SuccessOrExit(err);

    // Step 5
    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    err = Validate_and_RetrieveResponderID(responderOpCert, responderOpCertLen, remoteCredential);
    SuccessOrExit(err);

    // Step 6 - Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_r2_signed_len =
        static_cast<uint16_t>(sizeof(uint16_t) + responderOpCertLen + kP256_PublicKey_Length * 2 + sizeof(uint64_t) * 3);

    VerifyOrExit(msg_R2_Signed.Alloc(msg_r2_signed_len), err = CHIP_ERROR_NO_MEMORY);

    err = ConstructTBS2Data(responderOpCert, responderOpCertLen, msg_R2_Signed.Get(), msg_r2_signed_len);
    SuccessOrExit(err);

    err = decryptedDataTlvReader.FindElementWithTag(CASETLVTag::kSignature, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(sigmaR2SignedData.Capacity() >= suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    sigmaR2SignedData.SetLength(suppTlvReader.GetLength());
    err = suppTlvReader.GetBytes(sigmaR2SignedData, static_cast<uint32_t>(sigmaR2SignedData.Length()));
    SuccessOrExit(err);

    err = remoteCredential.ECDSA_validate_msg_signature(msg_R2_Signed.Get(), msg_r2_signed_len, sigmaR2SignedData);
    SuccessOrExit(err);

exit:
    if (err == CHIP_ERROR_INVALID_SIGNATURE)
    {
        SendErrorMsg(SigmaErrorType::kInvalidSignature);
    }
    else if (err == CHIP_ERROR_CERT_NOT_TRUSTED)
    {
        SendErrorMsg(SigmaErrorType::kNoSharedTrustRoots);
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

    System::PacketBufferHandle msg_R3;
    uint16_t data_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    uint16_t msg_r3_encrypted_len;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    uint8_t sr3k[kAEADKeySize];

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    uint16_t msg_r3_signed_len;

    P256ECDSASignature sigmaR3Signature;

    uint8_t tag[kTAGSize];

    HKDF_sha_crypto mHKDF;

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    ChipLogDetail(SecureChannel, "Sending SigmaR3");
    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR3(mIPK, sizeof(mIPK), saltSpan);
        SuccessOrExit(err);
    }
    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Step 2
    msg_r3_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + mOpCredSet->GetDevOpCredLen(mTrustedRootId) +
                                              kP256_PublicKey_Length * 2 + sizeof(uint64_t) * 3);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Signed.Get(), msg_r3_signed_len);
        SuccessOrExit(err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                               static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                               mOpCredSet->GetDevOpCredLen(mTrustedRootId)));
        SuccessOrExit(err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mRemotePubKey,
                                               static_cast<uint32_t>(mRemotePubKey.Length())));
        SuccessOrExit(err = tlvWriter.EndContainer(outerContainerType));
        SuccessOrExit(err = tlvWriter.Finalize());
        msg_r3_signed_len = static_cast<uint16_t>(tlvWriter.GetLengthWritten());
    }

    // Step 3
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R3_Signed.Get(), msg_r3_signed_len, sigmaR3Signature);
    SuccessOrExit(err);

    // Step 4
    msg_r3_encrypted_len = static_cast<uint16_t>(sizeof(uint16_t) + mOpCredSet->GetDevOpCredLen(mTrustedRootId) +
                                                 static_cast<uint16_t>(sigmaR3Signature.Length()) + sizeof(uint64_t) * 2);

    VerifyOrExit(msg_R3_Encrypted.Alloc(msg_r3_encrypted_len), err = CHIP_ERROR_NO_MEMORY);

    {
        TLV::TLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                 mOpCredSet->GetDevOpCredLen(mTrustedRootId));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kSignature, sigmaR3Signature, static_cast<uint32_t>(sigmaR3Signature.Length()));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize();
        SuccessOrExit(err);
    }

    // Step 5
    err = AES_CCM_encrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0, sr3k, kAEADKeySize, kIVSR3, kIVLength,
                          msg_R3_Encrypted.Get(), tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 6
    data_len = static_cast<uint16_t>(sizeof(tag) + msg_r3_encrypted_len + sizeof(uint64_t) * 2);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kEncryptedData, msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kTag, tag, sizeof(tag));
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3);
        SuccessOrExit(err);
    }

    err = mCommissioningHash.AddData(msg_R3->Start(), msg_R3->DataLength());
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = mExchangeCtxt->SendMessage(Protocols::SecureChannel::MsgType::CASE_SigmaR3, std::move(msg_R3));
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Sent SigmaR3 msg");

    err = mCommissioningHash.Finish(mMessageDigest);
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
    System::PacketBufferTLVReader tlvReader;
    System::PacketBufferTLVReader suppTlvReader;
    TLV::TLVReader decryptedDataTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * buf   = msg->Start();
    const uint16_t bufLen = msg->DataLength();

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Encrypted;
    uint16_t msg_r3_encrypted_len;
    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_R3_Signed;
    uint16_t msg_r3_signed_len;

    uint8_t sr3k[kAEADKeySize];

    P256ECDSASignature sigmaR3SignedData;

    P256PublicKey remoteCredential;

    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;

    chip::Platform::ScopedMemoryBuffer<uint8_t> msg_salt;
    uint16_t saltlen;

    uint8_t tag[kTAGSize];

    HKDF_sha_crypto mHKDF;

    ChipLogDetail(SecureChannel, "Received SigmaR3 msg");

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    tlvReader.Init(std::move(msg));
    err = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kEncryptedData, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);

    VerifyOrExit(msg_R3_Encrypted.Alloc(suppTlvReader.GetLength()), err = CHIP_ERROR_NO_MEMORY);
    msg_r3_encrypted_len = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                  = suppTlvReader.GetBytes(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kTag, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(kTAGSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    err = suppTlvReader.GetBytes(tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    VerifyOrExit(msg_salt.Alloc(saltlen), err = CHIP_ERROR_NO_MEMORY);

    err = ComputeIPK(mConnectionState.GetPeerKeyID(), mRemoteIPK, sizeof(mRemoteIPK));
    SuccessOrExit(err);

    {
        MutableByteSpan saltSpan(msg_salt.Get(), saltlen);
        err = ConstructSaltSigmaR3(mRemoteIPK, sizeof(mRemoteIPK), saltSpan);
        SuccessOrExit(err);
    }

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt.Get(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                            kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(buf, bufLen);
    SuccessOrExit(err);

    // Step 2
    err = AES_CCM_decrypt(msg_R3_Encrypted.Get(), msg_r3_encrypted_len, nullptr, 0, tag, kTAGSize, sr3k, kAEADKeySize, kIVSR3,
                          kIVLength, msg_R3_Encrypted.Get());
    SuccessOrExit(err);

    decryptedDataTlvReader.Init(msg_R3_Encrypted.Get(), msg_r3_encrypted_len);
    containerType = TLV::kTLVType_Structure;
    err           = decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = decryptedDataTlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = decryptedDataTlvReader.FindElementWithTag(CASETLVTag::kNOC, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    responderOpCertLen = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                = suppTlvReader.GetBytes(responderOpCert, responderOpCertLen);
    SuccessOrExit(err);

    // Step 3
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    err = Validate_and_RetrieveResponderID(responderOpCert, responderOpCertLen, remoteCredential);
    SuccessOrExit(err);

    // Step 4
    msg_r3_signed_len =
        static_cast<uint16_t>(sizeof(uint16_t) + responderOpCertLen + kP256_PublicKey_Length * 2 + sizeof(uint64_t) * 3);

    VerifyOrExit(msg_R3_Signed.Alloc(msg_r3_signed_len), err = CHIP_ERROR_NO_MEMORY);

    err = ConstructTBS3Data(responderOpCert, responderOpCertLen, msg_R3_Signed.Get(), msg_r3_signed_len);
    SuccessOrExit(err);

    err = decryptedDataTlvReader.FindElementWithTag(CASETLVTag::kSignature, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_ByteString, err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(sigmaR3SignedData.Capacity() >= suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    sigmaR3SignedData.SetLength(suppTlvReader.GetLength());
    err = suppTlvReader.GetBytes(sigmaR3SignedData, static_cast<uint32_t>(sigmaR3SignedData.Length()));
    SuccessOrExit(err);

    err = remoteCredential.ECDSA_validate_msg_signature(msg_R3_Signed.Get(), msg_r3_signed_len, sigmaR3SignedData);
    SuccessOrExit(err);

    err = mCommissioningHash.Finish(mMessageDigest);
    SuccessOrExit(err);

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

CHIP_ERROR CASESession::FindValidTrustedRoot(const System::PacketBufferTLVReader & tlvReader, uint32_t nTrustedRoots)
{
    CertificateKeyId trustedRoot;
    System::PacketBufferTLVReader suppTlvReader;
    uint8_t trustedRootId[kTrustedRootIdSize];

    trustedRoot = CertificateKeyId(trustedRootId);

    for (uint32_t i = 0; i < nTrustedRoots; ++i)
    {
        ReturnErrorOnFailure(tlvReader.FindElementWithTag(CASETLVTag::kTrustedRootID, suppTlvReader));

        VerifyOrReturnError(kTrustedRootIdSize == suppTlvReader.GetLength(), CHIP_ERROR_INVALID_TLV_ELEMENT);
        VerifyOrReturnError(suppTlvReader.GetType() == TLV::kTLVType_ByteString, CHIP_ERROR_WRONG_TLV_TYPE);
        ReturnErrorOnFailure(suppTlvReader.GetBytes(trustedRootId, kTrustedRootIdSize));

        if (mOpCredSet->IsTrustedRootIn(trustedRoot))
        {
            if (!mTrustedRootId.empty())
            {
                chip::Platform::MemoryFree(const_cast<uint8_t *>(mTrustedRootId.data()));
                mTrustedRootId = CertificateKeyId();
            }
            mTrustedRootId = CertificateKeyId(reinterpret_cast<const uint8_t *>(chip::Platform::MemoryAlloc(kTrustedRootIdSize)));
            VerifyOrReturnError(!mTrustedRootId.empty(), CHIP_ERROR_NO_MEMORY);

            memcpy(const_cast<uint8_t *>(mTrustedRootId.data()), trustedRoot.data(), trustedRoot.size());

            break;
        }
    }
    VerifyOrReturnError(!mTrustedRootId.empty(), CHIP_ERROR_CERT_NOT_TRUSTED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigmaR2(const ByteSpan & rand, const P256PublicKey & pubkey, const uint8_t * ipk,
                                             size_t ipkLen, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk, ipkLen);
    bbuf.Put(rand.data(), kSigmaParamRandomNumberSize);
    bbuf.Put(pubkey, pubkey.Length());
    ReturnErrorOnFailure(mCommissioningHash.Finish(md));
    bbuf.Put(md, kSHA256_Hash_Length);
    ReturnErrorOnFailure(mCommissioningHash.Begin());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigmaR3(const uint8_t * ipk, size_t ipkLen, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk, ipkLen);
    ReturnErrorOnFailure(mCommissioningHash.Finish(md));
    bbuf.Put(md, kSHA256_Hash_Length);
    ReturnErrorOnFailure(mCommissioningHash.Begin());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::Validate_and_RetrieveResponderID(const uint8_t * responderOpCert, uint16_t responderOpCertLen,
                                                         P256PublicKey & responderID)
{
    ChipCertificateData * resultCert = nullptr;

    ChipCertificateSet certSet;
    // Certificate set can contain up to 3 certs (NOC, ICA cert, and Root CA cert)
    ReturnErrorOnFailure(certSet.Init(3, kMaxCHIPCertDecodeBufLength));

    Encoding::LittleEndian::BufferWriter bbuf(responderID, responderID.Length());
    ReturnErrorOnFailure(
        certSet.LoadCerts(responderOpCert, responderOpCertLen, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    bbuf.Put(certSet.GetCertSet()[0].mPublicKey.data(), certSet.GetCertSet()[0].mPublicKey.size());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    // Validate responder identity located in msg_r2_encrypted
    ReturnErrorOnFailure(
        mOpCredSet->FindCertSet(mTrustedRootId)
            ->LoadCerts(responderOpCert, responderOpCertLen, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    ReturnErrorOnFailure(SetEffectiveTime());
    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    {
        const ChipDN & subjectDN              = certSet.GetCertSet()[0].mSubjectDN;
        const CertificateKeyId & subjectKeyId = certSet.GetCertSet()[0].mSubjectKeyId;

        ReturnErrorOnFailure(mOpCredSet->FindValidCert(mTrustedRootId, subjectDN, subjectKeyId, mValidContext, resultCert));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS2Data(const uint8_t * responderOpCert, uint32_t responderOpCertLen, uint8_t * tbsData,
                                          uint16_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kNOC, responderOpCert, responderOpCertLen));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<uint16_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS3Data(const uint8_t * responderOpCert, uint32_t responderOpCertLen, uint8_t * tbsData,
                                          uint16_t & tbsDataLen)
{
    TLV::TLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(tbsData, tbsDataLen);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kNOC, responderOpCert, responderOpCertLen));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    tbsDataLen = static_cast<uint16_t>(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ComputeIPK(const uint16_t sessionID, uint8_t * ipk, size_t ipkLen)
{
    HKDF_sha_crypto mHKDF;
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(mFabricSecret, mFabricSecret.Length(), reinterpret_cast<const uint8_t *>(&sessionID),
                                           sizeof(sessionID), kIPKInfo, sizeof(kIPKInfo), ipk, ipkLen));

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
    case SigmaErrorType::kNoSharedTrustRoots:
        err = CHIP_ERROR_CERT_NOT_TRUSTED;
        break;

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

    if (packetHeader.GetSourceNodeId().HasValue())
    {
        if (mConnectionState.GetPeerNodeId() == kUndefinedNodeId)
        {
            mConnectionState.SetPeerNodeId(packetHeader.GetSourceNodeId().Value());
        }
        else
        {
            VerifyOrReturnError(packetHeader.GetSourceNodeId().Value() == mConnectionState.GetPeerNodeId(),
                                CHIP_ERROR_WRONG_NODE_ID);
        }
    }

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
