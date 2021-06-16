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
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;

// TODO: Remove Later
static P256ECDHDerivedSecret fabricSecret;

constexpr uint8_t kIPKInfo[] = { 0x49, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x74, 0x79, 0x50, 0x72, 0x6f,
                                 0x74, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79 };

constexpr uint8_t kKDFSR2Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32 };
constexpr uint8_t kKDFSR3Info[]   = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33 };
constexpr size_t kKDFInfoLength   = sizeof(kKDFSR2Info);
constexpr uint8_t kKDFSEInfo[]    = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };
constexpr size_t kKDFSEInfoLength = sizeof(kKDFSEInfo);

constexpr uint8_t kIVSR2[] = { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x4e };
constexpr uint8_t kIVSR3[] = { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33, 0x4e };
constexpr size_t kIVLength = sizeof(kIVSR2);

constexpr size_t kTAGSize = 16;

/**
 * \brief
 * A list of registered packet types a.k.a. TLV context-specific tags to be used during CASE protocol.
 */
enum CASETLVTag : uint64_t
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
};

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

// Wait at most 30 seconds for the response from the peer.
// This timeout value assumes the underlying transport is reliable.
// The session establishment fails if the response is not received within timeout window.
static constexpr ExchangeContext::Timeout kSigma_Response_Timeout = 30000;

CASESession::CASESession()
{
    mTrustedRootId.mId  = nullptr;
    mTrustedRootId.mLen = 0;
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
    if (mTrustedRootId.mId != nullptr)
    {
        chip::Platform::MemoryFree(const_cast<uint8_t *>(mTrustedRootId.mId));
        mTrustedRootId.mId  = nullptr;
        mTrustedRootId.mLen = 0;
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

    // TODO: properly initialize fabric secret
    for (size_t i = 0; i < fabricSecret.Capacity(); i++)
    {
        fabricSecret[i] = static_cast<uint8_t>(i);
    }
    fabricSecret.SetLength(fabricSecret.Capacity());

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
                                         OperationalCredentialSet * operationalCredentialSet,
                                         const CertificateKeyId & trustedRootId, NodeId peerNodeId, uint16_t myKeyId,
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
    SuccessOrExit(SetTrustedRootId(trustedRootId));

    err = SendSigmaR1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR CASESession::SetTrustedRootId(const CertificateKeyId & trustedRootId)
{
    VerifyOrReturnError(trustedRootId.mLen == kTrustedRootIdSize, CHIP_ERROR_INVALID_ARGUMENT);

    if (mTrustedRootId.mId == nullptr)
    {
        mTrustedRootId.mId = reinterpret_cast<const uint8_t *>(Platform::MemoryAlloc(kTrustedRootIdSize));
        VerifyOrReturnError(mTrustedRootId.mId != nullptr, CHIP_ERROR_NO_MEMORY);
    }

    memcpy(const_cast<uint8_t *>(mTrustedRootId.mId), trustedRootId.mId, trustedRootId.mLen);
    mTrustedRootId.mLen = trustedRootId.mLen;

    return CHIP_NO_ERROR;
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
    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    (void) kKDFSEInfo;
    (void) kKDFSEInfoLength;

    VerifyOrReturnError(mPairingComplete, CHIP_ERROR_INCORRECT_STATE);

    // Generate Salt for Encryption keys
    saltlen = kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrReturnError(!msg_salt.IsNull(), CHIP_SYSTEM_ERROR_NO_MEMORY);
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);
        // TODO: Add IPK to Salt
        bbuf.Put(mMessageDigest, sizeof(mMessageDigest));

        VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);
    }

    ReturnErrorOnFailure(session.InitFromSecret(ByteSpan(mSharedSecret, mSharedSecret.Length()),
                                                ByteSpan(msg_salt->Start(), saltlen),
                                                SecureSession::SessionInfoType::kSessionEstablishment, role));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigmaR1()
{
    System::PacketBufferTLVWriter tlvWriter;
    System::PacketBufferHandle msg_R1;
    TLV::TLVType outerContainerType                      = TLV::kTLVType_NotSpecified;
    uint8_t initiatorRandom[kSigmaParamRandomNumberSize] = { 0 };
    uint8_t destinationIdentifier[kMAX_Hash_Length]      = { 0 };

    // Step 1
    ReturnErrorOnFailure(mEphemeralKey.Initialize());

    // Step 2
    ReturnErrorOnFailure(ComputeIPK(mConnectionState.GetLocalKeyID(), mIPK, sizeof(mIPK)));

    // Step 3
    ReturnErrorOnFailure(DRBG_get_bytes(initiatorRandom, kSigmaParamRandomNumberSize));

    // Step 6
    msg_R1 = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_SYSTEM_ERROR_NO_MEMORY);

    tlvWriter.Init(std::move(msg_R1));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kRandom, initiatorRandom, sizeof(initiatorRandom)));
    // Step 5 (6)
    ReturnErrorOnFailure(tlvWriter.Put(CASETLVTag::kSessionID, mConnectionState.GetLocalKeyID(), true));

    // Step 4 (6)
    ReturnErrorOnFailure(GenerateDestinationID(initiatorRandom, sizeof(initiatorRandom), &mTrustedRootId,
                                               mConnectionState.GetPeerNodeId(), destinationIdentifier,
                                               sizeof(destinationIdentifier)));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kDestinationID, destinationIdentifier, sizeof(destinationIdentifier)));

    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg_R1));

    ReturnErrorOnFailure(mCommissioningHash.AddData(msg_R1->Start(), msg_R1->DataLength()));

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR2;

    // Send the msg to peer
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
    uint8_t destinationIdentifier[kMAX_Hash_Length];
    uint8_t initiatorRandom[kSigmaParamRandomNumberSize];

    ChipLogDetail(SecureChannel, "Received SigmaR1 msg");

    err = mCommissioningHash.AddData(msg->Start(), msg->DataLength());
    SuccessOrExit(err);

    tlvReader.Init(std::move(msg));
    err = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kRandom, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(kSigmaParamRandomNumberSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(initiatorRandom, sizeof(initiatorRandom));
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kSessionID, suppTlvReader);
    SuccessOrExit(err);
    err = suppTlvReader.Get(encryptionKeyId);
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    mConnectionState.SetPeerKeyID(encryptionKeyId);

    ReturnErrorOnFailure(ComputeIPK(mConnectionState.GetPeerKeyID(), mIPK, sizeof(mIPK)));

    err = tlvReader.FindElementWithTag(CASETLVTag::kDestinationID, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(kMAX_Hash_Length == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(destinationIdentifier, sizeof(destinationIdentifier));
    SuccessOrExit(err);

    err =
        FindDestinationIdCandidate(destinationIdentifier, sizeof(destinationIdentifier), initiatorRandom, sizeof(initiatorRandom));
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kInitiatorEphPubKey, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(mRemotePubKey.Length() == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length()));
    SuccessOrExit(err);

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
    System::PacketBufferHandle msg_R2_tbs;
    System::PacketBufferHandle msg_R2_Encrypted;
    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t sr2k[kAEADKeySize];
    uint8_t tag[kTAGSize];
    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    P256ECDSASignature sigmaR2Signature;

    HKDF_sha_crypto mHKDF;

    // Step 3
    err = mEphemeralKey.Initialize();
    SuccessOrExit(err);

    // Step 4
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    // Step 5 - Generate Sigma2 TBS Data
    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        msg_R2_tbs = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
        VerifyOrExit(!msg_R2_tbs.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

        tlvWriter.Init(std::move(msg_R2_tbs));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                 mOpCredSet->GetDevOpCredLen(mTrustedRootId));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                 static_cast<uint32_t>(mEphemeralKey.Pubkey().Length()));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length()));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R2_tbs);
        SuccessOrExit(err);
    }

    // Step 6 - Generate a signature
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R2_tbs->Start(), msg_R2_tbs->DataLength(), sigmaR2Signature);
    SuccessOrExit(err);

    // Step 7 - Generate Sigma2 TBE Data
    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        msg_R2_Encrypted = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
        VerifyOrExit(!msg_R2_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

        tlvWriter.Init(std::move(msg_R2_Encrypted));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                 mOpCredSet->GetDevOpCredLen(mTrustedRootId));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kSignature, sigmaR2Signature, static_cast<uint32_t>(sigmaR2Signature.Length()));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R2_Encrypted);
        SuccessOrExit(err);
    }

    // Step 8 - Generate sr2k
    saltlen  = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;
    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_salt->SetDataLength(saltlen);

    // Step 10 (8) - Generate a random number
    err = DRBG_get_bytes(responderRandom, sizeof(responderRandom));
    SuccessOrExit(err);

    err = ConstructSaltSigmaR2(responderRandom, sizeof(responderRandom), mEphemeralKey.Pubkey(), mIPK, sizeof(mIPK), msg_salt);
    SuccessOrExit(err);

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Step 9 - Generate the encrypted data blob
    err = AES_CCM_encrypt(msg_R2_Encrypted->Start(), msg_R2_Encrypted->DataLength(), nullptr, 0, sr2k, kAEADKeySize, kIVSR2,
                          kIVLength, msg_R2_Encrypted->Start(), tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 13 - Generate SigmaR2
    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        msg_R2 = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
        VerifyOrExit(!msg_R2.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

        tlvWriter.Init(std::move(msg_R2));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kRandom, responderRandom, sizeof(responderRandom));
        SuccessOrExit(err);
        err = tlvWriter.Put(CASETLVTag::kSessionID, mConnectionState.GetLocalKeyID(), true);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kTrustedRootID, mTrustedRootId.mId, mTrustedRootId.mLen);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                 static_cast<uint32_t>(mEphemeralKey.Pubkey().Length()));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kEncryptedData, msg_R2_Encrypted->Start(), msg_R2_Encrypted->DataLength());
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kTag, tag, sizeof(tag));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R2);
        SuccessOrExit(err);
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
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * raw_msg    = msg->Start();
    const uint16_t raw_msg_len = msg->DataLength();

    System::PacketBufferHandle msg_R2_Encrypted;
    System::PacketBufferHandle msg_R2_Signed;
    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint16_t encryptionKeyId = 0;
    uint8_t tag[kTAGSize];
    uint8_t sr2k[kAEADKeySize];
    uint8_t responderRandom[kSigmaParamRandomNumberSize];
    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;
    P256ECDSASignature sigmaR2SignedData;
    P256PublicKey remoteCredential;

    HKDF_sha_crypto mHKDF;

    ChipLogDetail(SecureChannel, "Received SigmaR2 msg");

    tlvReader.Init(std::move(msg));
    err = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    // Step 1 - Search for matching TrustedRootId
    err = tlvReader.FindElementWithTag(CASETLVTag::kTrustedRootID, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(kTrustedRootIdSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err                 = suppTlvReader.GetBytes(const_cast<uint8_t *>(mTrustedRootId.mId), kTrustedRootIdSize);
    mTrustedRootId.mLen = kTrustedRootIdSize;
    SuccessOrExit(err);

    err = FindValidTrustedRoot(mTrustedRootId);
    SuccessOrExit(err);

    // Assign Session Key ID
    err = tlvReader.FindElementWithTag(CASETLVTag::kSessionID, suppTlvReader);
    SuccessOrExit(err);
    err = suppTlvReader.Get(encryptionKeyId);
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "Peer assigned session key ID %d", encryptionKeyId);
    mConnectionState.SetPeerKeyID(encryptionKeyId);

    // Retrieve Responder's Random value
    err = tlvReader.FindElementWithTag(CASETLVTag::kRandom, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(kSigmaParamRandomNumberSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(responderRandom, sizeof(responderRandom));
    SuccessOrExit(err);

    // Retrieve Responder's Ephemeral Pubkey
    err = tlvReader.FindElementWithTag(CASETLVTag::kResponderEphPubKey, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(mRemotePubKey.Length() == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = suppTlvReader.GetBytes(mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length()));
    SuccessOrExit(err);

    // Step 3 - Generate a Shared Secret
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    // Step 4 - Generate SR2K
    saltlen  = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;
    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_salt->SetDataLength(saltlen);

    err = ConstructSaltSigmaR2(responderRandom, sizeof(responderRandom), mRemotePubKey, mIPK, sizeof(mIPK), msg_salt);
    SuccessOrExit(err);

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                            kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(raw_msg, raw_msg_len);
    SuccessOrExit(err);

    // Step 5 - Generate decrypted data
    err = tlvReader.FindElementWithTag(CASETLVTag::kEncryptedData, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    msg_R2_Encrypted = System::PacketBufferHandle::New(suppTlvReader.GetLength());
    VerifyOrExit(!msg_R2_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_R2_Encrypted->SetDataLength(static_cast<uint16_t>(suppTlvReader.GetLength()));
    err = suppTlvReader.GetBytes(msg_R2_Encrypted->Start(), msg_R2_Encrypted->DataLength());
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kTag, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(kTAGSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    err = suppTlvReader.GetBytes(tag, sizeof(tag));
    SuccessOrExit(err);

    err = AES_CCM_decrypt(msg_R2_Encrypted->Start(), msg_R2_Encrypted->DataLength(), nullptr, 0, tag, sizeof(tag), sr2k,
                          kAEADKeySize, kIVSR2, kIVLength, msg_R2_Encrypted->Start());
    SuccessOrExit(err);

    tlvReader.Init(std::move(msg_R2_Encrypted));
    containerType = TLV::kTLVType_Structure;
    err           = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kNOC, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    responderOpCertLen = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                = suppTlvReader.GetBytes(responderOpCert, responderOpCertLen);
    SuccessOrExit(err);

    // Step 8
    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    err = Validate_and_RetrieveResponderID(responderOpCert, responderOpCertLen, remoteCredential);
    SuccessOrExit(err);

    // Step 7 - Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_R2_Signed = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrExit(!msg_R2_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    err = ConstructTBS2Data(responderOpCert, responderOpCertLen, msg_R2_Signed);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kSignature, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(sigmaR2SignedData.Capacity() >= suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    sigmaR2SignedData.SetLength(suppTlvReader.GetLength());
    err = suppTlvReader.GetBytes(sigmaR2SignedData, static_cast<uint32_t>(sigmaR2SignedData.Length()));
    SuccessOrExit(err);

    // Step 10
    err = remoteCredential.ECDSA_validate_msg_signature(msg_R2_Signed->Start(), msg_R2_Signed->DataLength(), sigmaR2SignedData);
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
    System::PacketBufferHandle msg_R3_Encrypted;
    System::PacketBufferHandle msg_R3_tbs;
    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t tag[kTAGSize];
    uint8_t sr3k[kAEADKeySize];
    P256ECDSASignature sigmaR3Signature;

    HKDF_sha_crypto mHKDF;

    ChipLogDetail(SecureChannel, "Sending SigmaR3");

    // Step 2 - Prepare SigmaR3 TBS Data Blob
    msg_R3_tbs = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrExit(!msg_R3_tbs.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3_tbs));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                 mOpCredSet->GetDevOpCredLen(mTrustedRootId));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                 static_cast<uint32_t>(mEphemeralKey.Pubkey().Length()));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length()));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3_tbs);
        SuccessOrExit(err);
    }

    // Step 3 - Generate a signature
    err = mOpCredSet->SignMsg(mTrustedRootId, msg_R3_tbs->Start(), msg_R3_tbs->DataLength(), sigmaR3Signature);
    SuccessOrExit(err);

    // Step 4 - Prepare SigmaR3 TBE Data Blob
    msg_R3_Encrypted = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrExit(!msg_R3_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3_Encrypted));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kNOC, mOpCredSet->GetDevOpCred(mTrustedRootId),
                                 mOpCredSet->GetDevOpCredLen(mTrustedRootId));
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kSignature, sigmaR3Signature, static_cast<uint32_t>(sigmaR3Signature.Length()));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3_Encrypted);
        SuccessOrExit(err);
    }

    // Step 5 - Generate SR3K
    saltlen = kIPKSize + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_salt->SetDataLength(saltlen);

    err = ConstructSaltSigmaR3(mIPK, sizeof(mIPK), msg_salt);
    SuccessOrExit(err);

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                            kAEADKeySize);
    SuccessOrExit(err);

    // Step 6 - Generated Encrypted data blob
    err = AES_CCM_encrypt(msg_R3_Encrypted->Start(), msg_R3_Encrypted->DataLength(), nullptr, 0, sr3k, kAEADKeySize, kIVSR3,
                          kIVLength, msg_R3_Encrypted->Start(), tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 7 - Generate SigmaR3
    msg_R3 = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(CASETLVTag::kEncryptedData, msg_R3_Encrypted->Start(), msg_R3_Encrypted->DataLength());
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

    // Close the exchange, as no additional messages are expected from the peer
    CloseExchange();

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
    TLV::TLVType containerType = TLV::kTLVType_Structure;

    const uint8_t * raw_msg    = msg->Start();
    const uint16_t raw_msg_len = msg->DataLength();

    System::PacketBufferHandle msg_R3_Encrypted;
    System::PacketBufferHandle msg_R3_Signed;
    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t tag[kTAGSize];
    uint8_t sr3k[kAEADKeySize];
    P256ECDSASignature sigmaR3SignedData;
    P256PublicKey remoteCredential;
    uint8_t responderOpCert[1024];
    uint16_t responderOpCertLen;

    HKDF_sha_crypto mHKDF;

    ChipLogDetail(SecureChannel, "Received SigmaR3 msg");

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    tlvReader.Init(std::move(msg));
    err = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    // Fetch encrypted data
    err = tlvReader.FindElementWithTag(CASETLVTag::kEncryptedData, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    msg_R3_Encrypted = System::PacketBufferHandle::New(suppTlvReader.GetLength());
    VerifyOrExit(!msg_R3_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_R3_Encrypted->SetDataLength(static_cast<uint16_t>(suppTlvReader.GetLength()));
    err = suppTlvReader.GetBytes(msg_R3_Encrypted->Start(), msg_R3_Encrypted->DataLength());
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kTag, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(kTAGSize == suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    err = suppTlvReader.GetBytes(tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);
    msg_salt->SetDataLength(saltlen);

    err = ConstructSaltSigmaR3(mIPK, sizeof(mIPK), msg_salt);
    SuccessOrExit(err);

    err = mHKDF.HKDF_SHA256(mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                            kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(raw_msg, raw_msg_len);
    SuccessOrExit(err);

    // Step 2 - Decrypt data blob
    err = AES_CCM_decrypt(msg_R3_Encrypted->Start(), msg_R3_Encrypted->DataLength(), nullptr, 0, tag, kTAGSize, sr3k, kAEADKeySize,
                          kIVSR3, kIVLength, msg_R3_Encrypted->Start());
    SuccessOrExit(err);

    tlvReader.Init(std::move(msg_R3_Encrypted));
    containerType = TLV::kTLVType_Structure;
    err           = tlvReader.Next(containerType, TLV::AnonymousTag);
    SuccessOrExit(err);
    err = tlvReader.EnterContainer(containerType);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kNOC, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    responderOpCertLen = static_cast<uint16_t>(suppTlvReader.GetLength());
    err                = suppTlvReader.GetBytes(responderOpCert, responderOpCertLen);
    SuccessOrExit(err);

    // Step 5
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    err = Validate_and_RetrieveResponderID(responderOpCert, responderOpCertLen, remoteCredential);
    SuccessOrExit(err);

    // Step 4 - Construct SigmaR3 TBS Data
    msg_R3_Signed = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    VerifyOrExit(!msg_R3_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    err = ConstructTBS3Data(responderOpCert, responderOpCertLen, msg_R3_Signed);
    SuccessOrExit(err);

    err = tlvReader.FindElementWithTag(CASETLVTag::kSignature, suppTlvReader);
    SuccessOrExit(err);
    VerifyOrExit(suppTlvReader.GetType() == TLV::kTLVType_UTF8String || suppTlvReader.GetType() == TLV::kTLVType_ByteString,
                 err = CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrExit(sigmaR3SignedData.Capacity() >= suppTlvReader.GetLength(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    sigmaR3SignedData.SetLength(suppTlvReader.GetLength());
    err = suppTlvReader.GetBytes(sigmaR3SignedData, static_cast<uint32_t>(sigmaR3SignedData.Length()));
    SuccessOrExit(err);

    // Step 7 - Validate Signature
    err = remoteCredential.ECDSA_validate_msg_signature(msg_R3_Signed->Start(), msg_R3_Signed->DataLength(), sigmaR3SignedData);
    SuccessOrExit(err);

    err = mCommissioningHash.Finish(mMessageDigest);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Close the exchange, as no additional messages are expected from the peer
    CloseExchange();

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

CHIP_ERROR CASESession::GenerateDestinationID(const uint8_t * random, uint16_t randomLen, const CertificateKeyId * trustedRootId,
                                              NodeId nodeId, uint8_t * destinationId, uint16_t destinationIdLen)
{
    HKDF_sha_crypto mHKDF;
    // TODO: read fabricId
    uint64_t fabricId              = 0;
    uint16_t destinationMessageLen = kSigmaParamRandomNumberSize + kP256_PublicKey_Length + sizeof(fabricId) + sizeof(nodeId);
    System::PacketBufferHandle destinationMessage = System::PacketBufferHandle::New(destinationMessageLen);
    const ChipCertificateData * rootCertificate   = mOpCredSet->GetRootCertificate(*trustedRootId);
    VerifyOrReturnError(!destinationMessage.IsNull(), CHIP_SYSTEM_ERROR_NO_MEMORY);
    VerifyOrReturnError(rootCertificate != nullptr, CHIP_ERROR_CERT_NOT_FOUND);
    VerifyOrReturnError(rootCertificate->mPublicKey != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(rootCertificate->mPublicKeyLen == kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage->Start(), destinationMessageLen);

    bbuf.Put(random, randomLen);
    bbuf.Put(rootCertificate->mPublicKey, rootCertificate->mPublicKeyLen);
    bbuf.Put64(fabricId);
    bbuf.Put64(nodeId);

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(destinationMessage->Start(), destinationMessageLen, 0, 0, mIPK, sizeof(mIPK),
                                           destinationId, destinationIdLen));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FindDestinationIdCandidate(const uint8_t * destinationId, uint16_t destinationIdLen,
                                                   const uint8_t * initiatorRandom, uint16_t initiatorRandomLen)
{
    uint8_t nCertificateSets = mOpCredSet->GetCertCount();

    for (uint8_t i = 0; i < nCertificateSets; ++i)
    {
        uint8_t candidate[kMAX_Hash_Length] = { 0 };
        const CertificateKeyId * trustedRootId;
        ChipCertificateData nodeOperationalCertificate;
        NodeId nodeId;

        trustedRootId = mOpCredSet->GetTrustedRootId(i);

        ReturnErrorOnFailure(DecodeChipCert(mOpCredSet->GetDevOpCred(*trustedRootId), mOpCredSet->GetDevOpCredLen(*trustedRootId),
                                            nodeOperationalCertificate));

        ReturnErrorOnFailure(nodeOperationalCertificate.mSubjectDN.GetCertChipId(nodeId));

        ReturnErrorOnFailure(
            GenerateDestinationID(initiatorRandom, initiatorRandomLen, trustedRootId, nodeId, candidate, sizeof(candidate)));

        if (memcmp(destinationId, candidate, destinationIdLen) == 0)
        {
            mTrustedRootId.mId = reinterpret_cast<const uint8_t *>(chip::Platform::MemoryAlloc(kTrustedRootIdSize));
            VerifyOrReturnError(mTrustedRootId.mId != nullptr, CHIP_ERROR_NO_MEMORY);

            memcpy(const_cast<uint8_t *>(mTrustedRootId.mId), trustedRootId->mId, trustedRootId->mLen);
            mTrustedRootId.mLen = trustedRootId->mLen;

            break;
        }
    }

    VerifyOrReturnError(mTrustedRootId.mId != nullptr, CHIP_ERROR_CERT_NOT_TRUSTED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::FindValidTrustedRoot(const CertificateKeyId & trustedRootId)
{
    if (mOpCredSet->IsTrustedRootIn(trustedRootId))
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_CERT_NOT_TRUSTED;
    }
}

CHIP_ERROR CASESession::ConstructSaltSigmaR2(const uint8_t * rand, uint16_t randLen, const P256PublicKey & pubkey,
                                             const uint8_t * ipk, size_t ipkLen, System::PacketBufferHandle & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt->Start(), 0, salt->DataLength());
    Encoding::LittleEndian::BufferWriter bbuf(salt->Start(), salt->DataLength());

    bbuf.Put(ipk, ipkLen);
    bbuf.Put(rand, randLen);
    bbuf.Put(pubkey, pubkey.Length());
    ReturnErrorOnFailure(mCommissioningHash.Finish(md));
    bbuf.Put(md, kSHA256_Hash_Length);
    ReturnErrorOnFailure(mCommissioningHash.Begin());

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigmaR3(const uint8_t * ipk, size_t ipkLen, System::PacketBufferHandle & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt->Start(), 0, salt->DataLength());
    Encoding::LittleEndian::BufferWriter bbuf(salt->Start(), salt->DataLength());

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
    ChipCertificateData chipCertData;
    ChipCertificateData * resultCert = nullptr;

    Encoding::LittleEndian::BufferWriter bbuf(responderID, responderID.Length());
    ReturnErrorOnFailure(DecodeChipCert(responderOpCert, responderOpCertLen, chipCertData));

    bbuf.Put(chipCertData.mPublicKey, chipCertData.mPublicKeyLen);

    VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_NO_MEMORY);

    // Validate responder identity located in msg_r2_encrypted
    ReturnErrorOnFailure(
        mOpCredSet->FindCertSet(mTrustedRootId)
            ->LoadCert(responderOpCert, responderOpCertLen, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash)));

    ReturnErrorOnFailure(SetEffectiveTime());
    // Locate the subject DN and key id that will be used as input the FindValidCert() method.
    {
        const ChipDN & subjectDN              = chipCertData.mSubjectDN;
        const CertificateKeyId & subjectKeyId = chipCertData.mSubjectKeyId;

        ReturnErrorOnFailure(mOpCredSet->FindValidCert(mTrustedRootId, subjectDN, subjectKeyId, mValidContext, resultCert));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS2Data(const uint8_t * responderOpCert, uint32_t responderOpCertLen,
                                          System::PacketBufferHandle & tbsData)
{
    System::PacketBufferTLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(std::move(tbsData));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kNOC, responderOpCert, responderOpCertLen));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&tbsData));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBS3Data(const uint8_t * responderOpCert, uint32_t responderOpCertLen,
                                          System::PacketBufferHandle & tbsData)
{
    System::PacketBufferTLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(std::move(tbsData));
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kNOC, responderOpCert, responderOpCertLen));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(CASETLVTag::kInitiatorEphPubKey, mRemotePubKey, static_cast<uint32_t>(mRemotePubKey.Length())));
    ReturnErrorOnFailure(tlvWriter.PutBytes(CASETLVTag::kResponderEphPubKey, mEphemeralKey.Pubkey(),
                                            static_cast<uint32_t>(mEphemeralKey.Pubkey().Length())));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&tbsData));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ComputeIPK(const uint16_t sessionID, uint8_t * ipk, size_t ipkLen)
{
    HKDF_sha_crypto mHKDF;
    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(fabricSecret, fabricSecret.Length(), reinterpret_cast<const uint8_t *>(&sessionID),
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
            // Close the incoming exchange explicitly, as the cleanup code only closes mExchangeCtxt
            ec->Close();
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

void CASESession::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                    System::PacketBufferHandle && msg)
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
        Clear();
        mDelegate->OnSessionEstablishmentError(err);
    }
}

} // namespace chip
