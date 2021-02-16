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
#include <transport/CASESession.h>

#include <inttypes.h>
#include <string.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <protocols/Protocols.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <support/BufferWriter.h>
#include <transport/SecureSessionMgr.h>

namespace chip {

// Dummy x509 certificate...
// TODO: Check whether this should be kept or not
extern const TrustedRootIdentifier TrustedRoots[] = {
    { 0x96, 0xDE, 0x61, 0xF1, 0xBD, 0x1C, 0x16, 0x29, 0x53, 0x1C, 0xC0, 0xCC, 0x7D, 0x3B, 0x83, 0x00, 0x40, 0xE6, 0x1A, 0x7C },
    { 0x96, 0xDE, 0x61, 0xF1, 0xBD, 0x1C, 0x16, 0x29, 0x53, 0x1C, 0xC0, 0xCC, 0x7D, 0x3B, 0x83, 0x00, 0x40, 0xE6, 0x1A, 0x7C }
};

constexpr uint8_t kKDFSR2Info[]   = "SigmaR2";
constexpr uint8_t kKDFSR3Info[]   = "SigmaR3";
constexpr size_t kKDFInfoLength   = 7;
constexpr uint8_t kKDFSEInfo[]    = "SessionKeys";
constexpr size_t kKDFSEInfoLength = 11;

constexpr uint8_t kIVSR2[] = "SigmaR2";
constexpr uint8_t kIVSR3[] = "SigmaR3";
constexpr size_t kIVLength = 7;

constexpr size_t kTAGSize = 16;

using namespace Crypto;
using namespace Credentials;

CASESession::CASESession() {}

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
    mLocalNodeId     = kUndefinedNodeId;
    mPairingComplete = false;
    mConnectionState.Reset();
}

CHIP_ERROR CASESession::Serialize(CASESessionSerialized & output)
{
    CHIP_ERROR error       = CHIP_NO_ERROR;
    uint16_t serializedLen = 0;
    CASESessionSerializable serializable;

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

CHIP_ERROR CASESession::Deserialize(CASESessionSerialized & input)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    CASESessionSerializable serializable;
    size_t maxlen            = BASE64_ENCODED_LEN(sizeof(serializable));
    size_t len               = strnlen(Uint8::to_char(input.inner), maxlen);
    uint16_t deserializedLen = 0;

    VerifyOrExit(len < sizeof(CASESessionSerialized), error = CHIP_ERROR_INVALID_ARGUMENT);
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

CHIP_ERROR CASESession::ToSerializable(CASESessionSerializable & serializable)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    const NodeId peerNodeId = mConnectionState.GetPeerNodeId();
    VerifyOrExit(CanCastTo<uint16_t>(mI2RR2IKeyLen), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(mLocalNodeId), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint64_t>(peerNodeId), error = CHIP_ERROR_INTERNAL);

    memset(&serializable, 0, sizeof(serializable));
    serializable.mI2RR2IKeyLen    = static_cast<uint16_t>(mI2RR2IKeyLen);
    serializable.mPairingComplete = (mPairingComplete) ? 1 : 0;
    serializable.mLocalNodeId     = mLocalNodeId;
    serializable.mPeerNodeId      = peerNodeId;
    serializable.mLocalKeyId      = mConnectionState.GetLocalKeyID();
    serializable.mPeerKeyId       = mConnectionState.GetPeerKeyID();

    memcpy(serializable.mI2RR2IKey, mI2RR2IKey, mI2RR2IKeyLen);

exit:
    return error;
}

CHIP_ERROR CASESession::FromSerializable(const CASESessionSerializable & serializable)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    mPairingComplete = (serializable.mPairingComplete == 1);
    mI2RR2IKeyLen    = static_cast<size_t>(serializable.mI2RR2IKeyLen);

    VerifyOrExit(mI2RR2IKeyLen <= sizeof(mI2RR2IKey), error = CHIP_ERROR_INVALID_ARGUMENT);
    memset(mI2RR2IKey, 0, sizeof(mI2RR2IKey));
    memcpy(mI2RR2IKey, serializable.mI2RR2IKey, mI2RR2IKeyLen);

    mLocalNodeId = serializable.mLocalNodeId;
    mConnectionState.SetPeerNodeId(serializable.mPeerNodeId);
    mConnectionState.SetLocalKeyID(serializable.mLocalKeyId);
    mConnectionState.SetPeerKeyID(serializable.mPeerKeyId);

exit:
    return error;
}

CHIP_ERROR CASESession::Init(ChipCertificateSet * chipCertificateSet, const uint8_t * devOpCert, uint16_t devOpCertLen,
                             P256SerializedKeypair & myDeviceOpKeys, NodeId myNodeId, uint16_t myKeyId,
                             SessionEstablishmentDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(delegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(chipCertificateSet->GetCertCount() > 0, err = CHIP_ERROR_CERT_NOT_FOUND);

    err = mCommissioningHash.Begin();
    SuccessOrExit(err);

    mDelegate    = delegate;
    mLocalNodeId = myNodeId;
    mConnectionState.SetLocalKeyID(myKeyId);
    err = mDeviceOpKeys.Deserialize(myDeviceOpKeys);
    SuccessOrExit(err);
    mDeviceOpCert       = devOpCert;
    mDeviceOpCertLen    = devOpCertLen;
    mChipCertificateSet = chipCertificateSet;

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

exit:
    return err;
}

CHIP_ERROR
CASESession::WaitForSessionEstablishment(ChipCertificateSet * chipCertificateSet, const uint8_t * myDevOpCert,
                                         uint16_t myDevOpCertLen, P256SerializedKeypair & myDeviceOpKeys, NodeId myNodeId,
                                         uint16_t myKeyId, SessionEstablishmentDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = Init(chipCertificateSet, myDevOpCert, myDevOpCertLen, myDeviceOpKeys, myNodeId, myKeyId, delegate);
    SuccessOrExit(err);

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR1;
    mPairingComplete = false;

    ChipLogDetail(Inet, "Waiting for SigmaR1 msg");

exit:
    return err;
}

CHIP_ERROR CASESession::AttachHeaderAndSend(Protocols::SecureChannel::MsgType msgType, System::PacketBufferHandle msgBuf)
{
    PayloadHeader payloadHeader;

    payloadHeader.SetMessageType(msgType);

    CHIP_ERROR err = payloadHeader.EncodeBeforeData(msgBuf);
    SuccessOrExit(err);

    err = mDelegate->SendSessionEstablishmentMessage(PacketHeader()
                                                         .SetSourceNodeId(mLocalNodeId)
                                                         .SetDestinationNodeId(mConnectionState.GetPeerNodeId())
                                                         .SetEncryptionKeyID(mConnectionState.GetLocalKeyID()),
                                                     mConnectionState.GetPeerAddress(), std::move(msgBuf));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CASESession::EstablishSession(const Transport::PeerAddress peerAddress, ChipCertificateSet * chipCertificateSet,
                                         const uint8_t * myDevOpCert, uint16_t myDevOpCertLen,
                                         P256SerializedKeypair & myDeviceOpKeys, NodeId myNodeId, NodeId peerNodeId,
                                         uint16_t myKeyId, SessionEstablishmentDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = Init(chipCertificateSet, myDevOpCert, myDevOpCertLen, myDeviceOpKeys, myNodeId, myKeyId, delegate);
    SuccessOrExit(err);

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

CHIP_ERROR CASESession::DeriveSecureSession(const uint8_t * info, size_t info_len, SecureSession & session)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mPairingComplete, err = CHIP_ERROR_INCORRECT_STATE);

    err = session.InitFromSecret(mI2RR2IKey, mI2RR2IKeyLen, nullptr, 0, info, info_len);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CASESession::SendSigmaR1()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t data_len = kSigmaParamRandomNumberSize + sizeof(uint16_t) + sizeof(TrustedRoots) + kP256_PublicKey_Length;

    System::PacketBufferHandle msg_R1;
    uint8_t * msg = nullptr;

    msg_R1 = System::PacketBufferHandle::New(data_len);
    VerifyOrReturnError(!msg_R1.IsNull(), CHIP_SYSTEM_ERROR_NO_MEMORY);

    msg = msg_R1->Start();

    // Step 1
    // Fill in the random value
    err = DRBG_get_bytes(msg, kSigmaParamRandomNumberSize);
    SuccessOrExit(err);

    // Step 4
    err = mEphemeralKey.Initialize();
    SuccessOrExit(err);

    // Step 5
    // Let's construct the rest of the message using Encoding::LittleEndian::BufferWriter
    {
        uint16_t n_trusted_roots = sizeof(TrustedRoots) / sizeof(TrustedRootIdentifier);
        Encoding::LittleEndian::BufferWriter bbuf(&msg[kSigmaParamRandomNumberSize], data_len - kSigmaParamRandomNumberSize);
        // Step 2
        // Omitting SessionID here - Mirroring PASESession, which omits SessionID as well
        // Step 3
        bbuf.Put16(n_trusted_roots);
        for (size_t i = 0; i < n_trusted_roots; ++i)
        {
            bbuf.Put(TrustedRoots[i], sizeof(TrustedRootIdentifier));
        }
        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R1->SetDataLength(data_len);

    err = mCommissioningHash.AddData(msg_R1->Start(), msg_R1->DataLength());
    SuccessOrExit(err);

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR2;

    // Call delegate to send the msg to peer
    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR1, std::move(msg_R1));
    SuccessOrExit(err);

    ChipLogDetail(Inet, "Sent SigmaR1 msg");

exit:
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR1_and_SendSigmaR2(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = HandleSigmaR1(header, msg);
    SuccessOrExit(err);

    err = SendSigmaR2();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR1(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    const uint8_t * buf   = msg->Start();
    uint16_t buflen       = msg->DataLength();
    uint16_t fixed_buflen = kSigmaParamRandomNumberSize + sizeof(uint16_t) + sizeof(TrustedRoots) + kP256_PublicKey_Length;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(buflen >= fixed_buflen, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    ChipLogDetail(Inet, "Received SigmaR1 msg");

    err = mCommissioningHash.AddData(msg->Start(), msg->DataLength());
    SuccessOrExit(err);

    {
        // Let's skip the random number portion of the message
        const uint8_t * msgptr = &buf[kSigmaParamRandomNumberSize];
        // Omitting SessionID here - Mirroring PASESession, which omits SessionID as well
        uint32_t n_trusted_roots = chip::Encoding::LittleEndian::Read16(msgptr);
        // Step 1/2
        // obtain certificate(s)
        // this is dummy...
        msgptr += kRootOfTrustCertSize * n_trusted_roots;
        // write public key from message
        Encoding::LittleEndian::BufferWriter bbuf(&*mRemotePubKey, mRemotePubKey.Length());
        bbuf.Put(msgptr, kP256_PublicKey_Length);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    if (header.GetSourceNodeId().HasValue() && mConnectionState.GetPeerNodeId() == kUndefinedNodeId)
    {
        mConnectionState.SetPeerNodeId(header.GetSourceNodeId().Value());
    }

    mConnectionState.SetPeerKeyID(header.GetEncryptionKeyID());

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

    System::PacketBufferHandle msg_rand;

    System::PacketBufferHandle msg_R2_Signed;
    uint16_t msg_r2_signed_len;

    System::PacketBufferHandle msg_R2_Encrypted;
    size_t msg_r2_signed_enc_len;

    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t sr2k[kAEADKeySize];
    P256ECDSASignature sigmaR2Signature;

    uint8_t * ipk   = nullptr;
    uint16_t ipklen = 0;

    uint8_t tag[kTAGSize];

    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    msg_rand = System::PacketBufferHandle::New(kSigmaParamRandomNumberSize);
    VerifyOrExit(!msg_rand.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    // Step 1
    // Fill in the random value
    err = DRBG_get_bytes(msg_rand->Start(), kSigmaParamRandomNumberSize);
    SuccessOrExit(err);

    msg_rand->SetDataLength(kSigmaParamRandomNumberSize);

    // Step 3
    // hardcoded to use a p256keypair
    err = mEphemeralKey.Initialize();
    SuccessOrExit(err);

    // Step 4
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    // Step 5
    {
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);

        bbuf.Put(ipk, ipklen); // placeholder...
        bbuf.Put(msg_rand->Start(), kSigmaParamRandomNumberSize);
        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_salt->SetDataLength(saltlen);

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                      kAEADKeySize);

    // Step 6
    msg_r2_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + mDeviceOpCertLen + kP256_PublicKey_Length * 2);

    msg_R2_Signed = System::PacketBufferHandle::New(msg_r2_signed_len);
    VerifyOrExit(!msg_R2_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R2_Signed->Start(), msg_r2_signed_len);

        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());
        bbuf.Put16(mDeviceOpCertLen);
        bbuf.Put(mDeviceOpCert, mDeviceOpCertLen);
        bbuf.Put(&*mRemotePubKey, mRemotePubKey.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R2_Signed->SetDataLength(msg_r2_signed_len);

    // Step 7
    mDeviceOpKeys.ECDSA_sign_msg(msg_R2_Signed->Start(), msg_R2_Signed->DataLength(), sigmaR2Signature);
    SuccessOrExit(err);

    // Step 8
    msg_r2_signed_enc_len = sizeof(uint16_t) + mDeviceOpCertLen + sigmaR2Signature.Length();

    msg_R2_Encrypted = System::PacketBufferHandle::New(msg_r2_signed_enc_len);
    VerifyOrExit(!msg_R2_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R2_Encrypted->Start(), msg_r2_signed_enc_len);

        bbuf.Put16(mDeviceOpCertLen);
        bbuf.Put(mDeviceOpCert, mDeviceOpCertLen);
        bbuf.Put(&*sigmaR2Signature, sigmaR2Signature.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R2_Encrypted->SetDataLength(static_cast<uint16_t>(msg_r2_signed_enc_len));

    // Step 9
    err = AES_CCM_encrypt(msg_R2_Encrypted->Start(), msg_r2_signed_enc_len, nullptr, 0, sr2k, kAEADKeySize, kIVSR2, kIVLength,
                          msg_R2_Encrypted->Start(), tag, sizeof(tag));
    SuccessOrExit(err);

    data_len = static_cast<uint16_t>(kSigmaParamRandomNumberSize + sizeof(TrustedRoots[0]) + kP256_PublicKey_Length +
                                     msg_r2_signed_enc_len + sizeof(tag));

    msg_R2 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R2.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    // Step 10
    // now construct sigmaR2
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R2->Start(), data_len);

        bbuf.Put(msg_rand->Start(), kSigmaParamRandomNumberSize);
        // Step 2
        // Omitting SessionID here - Mirroring PASESession, which omits SessionID as well
        bbuf.Put(TrustedRoots[0], sizeof(TrustedRoots[0]));
        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());
        bbuf.Put(msg_R2_Encrypted->Start(), msg_r2_signed_enc_len);
        bbuf.Put(tag, sizeof(tag));

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R2->SetDataLength(data_len);

    err = mCommissioningHash.AddData(msg_R2->Start(), msg_R2->DataLength());
    SuccessOrExit(err);

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaR3;

    // Call delegate to send the msg to peer
    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR2, std::move(msg_R2));
    SuccessOrExit(err);

    ChipLogDetail(Inet, "Sent SigmaR2 msg");

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR2_and_SendSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = HandleSigmaR2(header, msg);
    SuccessOrExit(err);

    err = SendSigmaR3();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR2(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t * buf = msg->Start();
    size_t buflen = msg->DataLength();

    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    System::PacketBufferHandle msg_R2_Signed;
    uint16_t msg_r2_signed_len;

    uint8_t sr2k[kAEADKeySize];

    P256ECDSASignature sigmaR2SignedData;

    P256PublicKey remoteCredential;

    const uint8_t * remoteDeviceOpCert;
    uint16_t remoteDeviceOpCertLen;

    uint8_t * remoteDeviceOpCHIPCert = nullptr;

    uint8_t * msg_r2_encrypted;

    uint8_t * tag = buf + buflen - kTAGSize;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);

    ChipLogDetail(Inet, "Received SigmaR2 msg");

    // Step 1
    // TODO
    // ValidateCertificate(&buf[kSigmaParamRandomNumberSize]);

    {
        // Omitting SessionID here - Mirroring PASESession, which omits SessionID as well
        const uint8_t * msgptr = &buf[kSigmaParamRandomNumberSize + kRootOfTrustCertSize];

        Encoding::LittleEndian::BufferWriter bbuf(&*mRemotePubKey, mRemotePubKey.Length());
        bbuf.Put(msgptr, mRemotePubKey.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    // Step 2
    err = mEphemeralKey.ECDH_derive_secret(mRemotePubKey, mSharedSecret);
    SuccessOrExit(err);

    // Step 3
    saltlen = kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        const uint8_t * responder_pubkey = &buf[kSigmaParamRandomNumberSize + kRootOfTrustCertSize];
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);

        //        bbuf.Put(ipk, ipklen); // placeholder... TODO
        bbuf.Put(buf, kSigmaParamRandomNumberSize);
        bbuf.Put(responder_pubkey, kP256_PublicKey_Length);
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_salt->SetDataLength(saltlen);

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR2Info, kKDFInfoLength, sr2k,
                      kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(msg->Start(), msg->DataLength());
    SuccessOrExit(err);

    // Step 4
    msg_r2_encrypted = &buf[kSigmaParamRandomNumberSize + kRootOfTrustCertSize + kP256_PublicKey_Length];

    err = AES_CCM_decrypt(msg_r2_encrypted,
                          buflen - kSigmaParamRandomNumberSize - kRootOfTrustCertSize - kP256_PublicKey_Length - kTAGSize, nullptr,
                          0, tag, kTAGSize, sr2k, kAEADKeySize, kIVSR2, kIVLength, msg_r2_encrypted);
    SuccessOrExit(err);

    // Step 5
    // Validate responder identity located in msg_r2_encrypted
    // Constructing responder identity
    {
        const uint8_t * msgptr = msg_r2_encrypted;
        ChipCertificateData chipCertData;
        uint32_t chipcertlen;
        ChipCertificateData * resultCert = nullptr;

        remoteDeviceOpCertLen = chip::Encoding::LittleEndian::Read16(msgptr);
        remoteDeviceOpCert    = msgptr;

        Encoding::LittleEndian::BufferWriter bbuf(&*remoteCredential, remoteCredential.Length());
        // Derive remoteCredential from remoteDeviceOpCert
        remoteDeviceOpCHIPCert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(remoteDeviceOpCertLen));
        VerifyOrExit(remoteDeviceOpCHIPCert != nullptr, err = CHIP_ERROR_NO_MEMORY);
        err = ConvertX509CertToChipCert(remoteDeviceOpCert, remoteDeviceOpCertLen, remoteDeviceOpCHIPCert, remoteDeviceOpCertLen,
                                        chipcertlen);
        SuccessOrExit(err);
        err = DecodeChipCert(remoteDeviceOpCHIPCert, chipcertlen, chipCertData);
        SuccessOrExit(err);

        bbuf.Put(chipCertData.mPublicKey, chipCertData.mPublicKeyLen);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);

        // Validate responder identity located in msg_r2_encrypted
        err = mChipCertificateSet->LoadCert(remoteDeviceOpCHIPCert, chipcertlen,
                                            BitFlags<uint8_t, CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);

        err = SetEffectiveTime();
        SuccessOrExit(err);
        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = chipCertData.mSubjectDN;
        const CertificateKeyId & subjectKeyId = chipCertData.mSubjectKeyId;

        err = mChipCertificateSet->FindValidCert(subjectDN, subjectKeyId, mValidContext, resultCert);
        SuccessOrExit(err);
    }

    // Step 6 - Construct msg_R2_Signed and validate the signature in msg_r2_encrypted
    msg_r2_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + remoteDeviceOpCertLen + kP256_PublicKey_Length * 2);

    msg_R2_Signed = System::PacketBufferHandle::New(msg_r2_signed_len);
    VerifyOrExit(!msg_R2_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R2_Signed->Start(), msg_r2_signed_len);

        bbuf.Put(&*mRemotePubKey, mRemotePubKey.Length());
        bbuf.Put16(remoteDeviceOpCertLen);
        bbuf.Put(remoteDeviceOpCert, remoteDeviceOpCertLen);
        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R2_Signed->SetDataLength(msg_r2_signed_len);

    {
        const uint8_t * signed_data = &buf[kSigmaParamRandomNumberSize + kRootOfTrustCertSize + kP256_PublicKey_Length +
                                           sizeof(uint16_t) + remoteDeviceOpCertLen];
        uint16_t signed_data_len =
            static_cast<uint16_t>(buflen - kSigmaParamRandomNumberSize - kRootOfTrustCertSize - kP256_PublicKey_Length -
                                  sizeof(uint16_t) - remoteDeviceOpCertLen - kTAGSize);

        sigmaR2SignedData.SetLength(signed_data_len);
        Encoding::LittleEndian::BufferWriter bbuf(&*sigmaR2SignedData, sigmaR2SignedData.Length());
        bbuf.Put(signed_data, sigmaR2SignedData.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    err = remoteCredential.ECDSA_validate_msg_signature(msg_R2_Signed->Start(), msg_r2_signed_len, sigmaR2SignedData);
    SuccessOrExit(err);

    mConnectionState.SetPeerKeyID(header.GetEncryptionKeyID());

exit:

    if (remoteDeviceOpCHIPCert != nullptr)
    {
        chip::Platform::MemoryFree(remoteDeviceOpCHIPCert);
    }

    if (err != CHIP_NO_ERROR)
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

    System::PacketBufferHandle msg_R3_Encrypted;
    uint16_t msg_r3_encrypted_len;

    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t sr3k[kAEADKeySize];

    System::PacketBufferHandle msg_R3_Signed;
    uint16_t msg_r3_signed_len;

    P256ECDSASignature sigmaR3Signature;

    uint8_t tag[kTAGSize];

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);

        // bbuf.Put(ipk, ipklen); // placeholder... TODO
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                      kAEADKeySize);
    SuccessOrExit(err);

    // Step 2
    msg_r3_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + mDeviceOpCertLen + kP256_PublicKey_Length * 2);

    msg_R3_Signed = System::PacketBufferHandle::New(msg_r3_signed_len);
    VerifyOrExit(!msg_R3_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R3_Signed->Start(), msg_r3_signed_len);

        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());
        bbuf.Put16(mDeviceOpCertLen);
        bbuf.Put(mDeviceOpCert, mDeviceOpCertLen);
        bbuf.Put(&*mRemotePubKey, mRemotePubKey.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R3_Signed->SetDataLength(msg_r3_signed_len);

    // Step 3
    err = mDeviceOpKeys.ECDSA_sign_msg(msg_R3_Signed->Start(), msg_R3_Signed->DataLength(), sigmaR3Signature);
    SuccessOrExit(err);

    // Step 4
    msg_r3_encrypted_len =
        static_cast<uint16_t>(sizeof(uint16_t) + mDeviceOpCertLen + static_cast<uint16_t>(sigmaR3Signature.Length()));

    msg_R3_Encrypted = System::PacketBufferHandle::New(msg_r3_encrypted_len);
    VerifyOrExit(!msg_R3_Encrypted.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R3_Encrypted->Start(), msg_r3_encrypted_len);

        bbuf.Put16(mDeviceOpCertLen);
        bbuf.Put(mDeviceOpCert, mDeviceOpCertLen);
        bbuf.Put(&*sigmaR3Signature, sigmaR3Signature.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R3_Encrypted->SetDataLength(msg_r3_encrypted_len);

    // Step 5
    err = AES_CCM_encrypt(msg_R3_Encrypted->Start(), msg_r3_encrypted_len, nullptr, 0, sr3k, kAEADKeySize, kIVSR3, kIVLength,
                          msg_R3_Encrypted->Start(), tag, sizeof(tag));
    SuccessOrExit(err);

    // Step 6
    data_len = static_cast<uint16_t>(sizeof(tag) + msg_r3_encrypted_len);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R3->Start(), data_len);

        bbuf.Put(msg_R3_Encrypted->Start(), msg_R3_Encrypted->DataLength());
        bbuf.Put(tag, sizeof(tag));

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R3->SetDataLength(data_len);

    err = mCommissioningHash.AddData(msg_R3->Start(), msg_R3->DataLength());
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaR3, std::move(msg_R3));
    SuccessOrExit(err);

    ChipLogDetail(Inet, "Sent SigmaR3 msg");

    // Generate Session Encryption Keys - I2RR2I keys
    saltlen = kIPKSize + kSHA256_Hash_Length;
    {
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);
        //        bbuf.Put(ipk, ipklen); // placeholder...
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSEInfo, kKDFSEInfoLength, mI2RR2IKey,
                      mI2RR2IKeyLen);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigmaR3(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg_R3_Signed;
    uint16_t msg_r3_signed_len;

    uint8_t sr3k[kAEADKeySize];

    P256ECDSASignature sigmaR3SignedData;

    P256PublicKey remoteCredential;

    const uint8_t * remoteDeviceOpCert;
    uint16_t remoteDeviceOpCertLen;

    uint8_t * remoteDeviceOpCHIPCert = nullptr;

    System::PacketBufferHandle msg_salt;
    uint16_t saltlen;

    uint8_t * tag = msg->Start() + msg->DataLength() - kTAGSize;

    ChipLogDetail(Inet, "Received SigmaR3 msg");

    mNextExpectedMsg = Protocols::SecureChannel::MsgType::CASE_SigmaErr;

    // Step 1
    saltlen = kIPKSize + kSHA256_Hash_Length;

    msg_salt = System::PacketBufferHandle::New(saltlen);
    VerifyOrExit(!msg_salt.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);

        // bbuf.Put(ipk, ipklen); // placeholder... TODO
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_salt->SetDataLength(saltlen);

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSR3Info, kKDFInfoLength, sr3k,
                      kAEADKeySize);
    SuccessOrExit(err);

    err = mCommissioningHash.AddData(msg->Start(), msg->DataLength());
    SuccessOrExit(err);

    // Step 2
    err = AES_CCM_decrypt(msg->Start(), msg->DataLength() - kTAGSize, nullptr, 0, tag, kTAGSize, sr3k, kAEADKeySize, kIVSR3,
                          kIVLength, msg->Start());
    SuccessOrExit(err);

    // Step 3
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    {
        const uint8_t * msgptr = msg->Start();
        ChipCertificateData chipCertData;
        uint32_t chipcertlen;
        ChipCertificateData * resultCert = nullptr;

        remoteDeviceOpCertLen = chip::Encoding::LittleEndian::Read16(msgptr);
        remoteDeviceOpCert    = msgptr;

        Encoding::LittleEndian::BufferWriter bbuf(&*remoteCredential, remoteCredential.Length());
        // Derive remoteCredential from remoteDeviceOpCert
        remoteDeviceOpCHIPCert = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(remoteDeviceOpCertLen));
        VerifyOrExit(remoteDeviceOpCHIPCert != nullptr, err = CHIP_ERROR_NO_MEMORY);
        err = ConvertX509CertToChipCert(remoteDeviceOpCert, remoteDeviceOpCertLen, remoteDeviceOpCHIPCert, remoteDeviceOpCertLen,
                                        chipcertlen);
        SuccessOrExit(err);
        err = DecodeChipCert(remoteDeviceOpCHIPCert, chipcertlen, chipCertData);
        SuccessOrExit(err);

        bbuf.Put(chipCertData.mPublicKey, chipCertData.mPublicKeyLen);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);

        // Validate responder identity located in msg->Start()
        err = mChipCertificateSet->LoadCert(remoteDeviceOpCHIPCert, chipcertlen,
                                            BitFlags<uint8_t, CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);

        err = SetEffectiveTime();
        SuccessOrExit(err);
        // Locate the subject DN and key id that will be used as input the FindValidCert() method.
        const ChipDN & subjectDN              = chipCertData.mSubjectDN;
        const CertificateKeyId & subjectKeyId = chipCertData.mSubjectKeyId;

        err = mChipCertificateSet->FindValidCert(subjectDN, subjectKeyId, mValidContext, resultCert);
        SuccessOrExit(err);
    }

    // Step 4
    msg_r3_signed_len = static_cast<uint16_t>(sizeof(uint16_t) + remoteDeviceOpCertLen + kP256_PublicKey_Length * 2);

    msg_R3_Signed = System::PacketBufferHandle::New(msg_r3_signed_len);
    VerifyOrExit(!msg_R3_Signed.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg_R3_Signed->Start(), msg_r3_signed_len);

        bbuf.Put(&*mRemotePubKey, mRemotePubKey.Length());
        bbuf.Put16(remoteDeviceOpCertLen);
        bbuf.Put(remoteDeviceOpCert, remoteDeviceOpCertLen);
        bbuf.Put(mEphemeralKey.Pubkey(), mEphemeralKey.Pubkey().Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msg_R3_Signed->SetDataLength(msg_r3_signed_len);

    {
        const uint8_t * msgptr = msg->Start() + sizeof(uint16_t) + remoteDeviceOpCertLen;

        sigmaR3SignedData.SetLength(msg->DataLength() - sizeof(uint16_t) - remoteDeviceOpCertLen - kTAGSize);
        Encoding::LittleEndian::BufferWriter bbuf(&*sigmaR3SignedData, sigmaR3SignedData.Length());
        bbuf.Put(msgptr, sigmaR3SignedData.Length());

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    err = remoteCredential.ECDSA_validate_msg_signature(msg_R3_Signed->Start(), msg_r3_signed_len, sigmaR3SignedData);
    SuccessOrExit(err);

    VerifyOrExit(header.GetSourceNodeId().ValueOr(kUndefinedNodeId) == mConnectionState.GetPeerNodeId(),
                 err = CHIP_ERROR_WRONG_NODE_ID);
    VerifyOrExit(header.GetEncryptionKeyID() == mConnectionState.GetPeerKeyID(), err = CHIP_ERROR_INVALID_KEY_ID);

    // Generate Session Encryption Keys - I2RR2I keys

    saltlen = kIPKSize + kSHA256_Hash_Length;
    {
        uint8_t md[kSHA256_Hash_Length];
        Encoding::LittleEndian::BufferWriter bbuf(msg_salt->Start(), saltlen);
        //        bbuf.Put(ipk, ipklen); // placeholder...
        err = mCommissioningHash.Finish(md);
        SuccessOrExit(err);
        bbuf.Put(md, kSHA256_Hash_Length);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    err = HKDF_SHA256(&*mSharedSecret, mSharedSecret.Length(), msg_salt->Start(), saltlen, kKDFSEInfo, kKDFSEInfoLength, mI2RR2IKey,
                      mI2RR2IKeyLen);
    SuccessOrExit(err);

    mPairingComplete = true;

    // Call delegate to indicate pairing completion
    mDelegate->OnSessionEstablished();

exit:

    if (remoteDeviceOpCHIPCert != nullptr)
    {
        chip::Platform::MemoryFree(remoteDeviceOpCHIPCert);
    }

    if (err != CHIP_NO_ERROR)
    {
        SendErrorMsg(SigmaErrorType::kUnexpected);
    }
    return err;
}

void CASESession::SendErrorMsg(SigmaErrorType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg;
    uint16_t msglen      = sizeof(SigmaErrorMsg);
    SigmaErrorMsg * pMsg = nullptr;

    msg = System::PacketBufferHandle::New(msglen);
    VerifyOrExit(!msg.IsNull(), err = CHIP_SYSTEM_ERROR_NO_MEMORY);

    pMsg        = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    pMsg->error = errorCode;

    msg->SetDataLength(msglen);

    err = AttachHeaderAndSend(Protocols::SecureChannel::MsgType::CASE_SigmaErr, std::move(msg));
    SuccessOrExit(err);

exit:
    Clear();
}

// PLACEHOLDER... MIGHT REMOVE
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

    return PackCertTime(effectiveTime, mValidContext.mEffectiveTime);
}

void CASESession::HandleErrorMsg(const PacketHeader & header, const System::PacketBufferHandle & msg)
{
    // Error message processing
    const uint8_t * buf  = msg->Start();
    size_t buflen        = msg->DataLength();
    SigmaErrorMsg * pMsg = nullptr;

    VerifyOrExit(buf != nullptr, ChipLogError(Inet, "Null error msg received during pairing"));
    static_assert(sizeof(SigmaErrorMsg) == sizeof(uint8_t),
                  "Assuming size of SigmaErrorMsg message is 1 octet, so that endian-ness conversion is not needed");
    VerifyOrExit(buflen == sizeof(SigmaErrorMsg), ChipLogError(Inet, "Error msg with incorrect length received during pairing"));

    pMsg = reinterpret_cast<SigmaErrorMsg *>(msg->Start());
    ChipLogError(Inet, "Received error (%d) during CASE pairing process", pMsg->error);

exit:
    Clear();
}

CHIP_ERROR CASESession::HandlePeerMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                          System::PacketBufferHandle msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PayloadHeader payloadHeader;

    Protocols::SecureChannel::MsgType msgType;

    VerifyOrExit(!msg.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = payloadHeader.DecodeAndConsume(msg);
    SuccessOrExit(err);

    VerifyOrExit(payloadHeader.GetProtocolID() == Protocols::kProtocol_SecureChannel, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    msgType = static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType());
    VerifyOrExit(msgType == mNextExpectedMsg, err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mConnectionState.SetPeerAddress(peerAddress);
    VerifyOrExit(mLocalNodeId == packetHeader.GetDestinationNodeId().Value(), err = CHIP_ERROR_WRONG_NODE_ID);

    switch (msgType)
    {
    case Protocols::SecureChannel::MsgType::CASE_SigmaR1:
        err = HandleSigmaR1_and_SendSigmaR2(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR2:
        err = HandleSigmaR2_and_SendSigmaR3(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaR3:
        err = HandleSigmaR3(packetHeader, msg);
        break;

    case Protocols::SecureChannel::MsgType::CASE_SigmaErr:
        HandleErrorMsg(packetHeader, msg);
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
        mDelegate->OnSessionEstablishmentError(err);
    }

    return err;
}

} // namespace chip
