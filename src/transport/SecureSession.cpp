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
 *      This file implements the CHIP Secure Session object.
 *
 */

#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <transport/SecureSession.h>
#include <transport/raw/MessageHeader.h>

#include <string.h>

namespace chip {

namespace {

constexpr size_t kAESCCMIVLen = 12;
constexpr size_t kMaxAADLen   = 128;

/* Session Establish Key Info */
constexpr uint8_t SEKeysInfo[] = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };

/* Session Resumption Key Info */
constexpr uint8_t RSEKeysInfo[] = { 0x53, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x52, 0x65, 0x73, 0x75,
                                    0x6d, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x4b, 0x65, 0x79, 0x73 };

} // namespace

using namespace Crypto;

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

SecureSession::SecureSession() : mKeyAvailable(false) {}

CHIP_ERROR SecureSession::InitFromSecret(const ByteSpan & secret, const ByteSpan & salt, SessionInfoType infoType, SessionRole role)
{
    HKDF_sha_crypto mHKDF;
    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(secret.data() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(secret.size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((salt.size() == 0) || (salt.data() != nullptr), CHIP_ERROR_INVALID_ARGUMENT);

    const uint8_t * info = SEKeysInfo;
    size_t infoLen       = sizeof(SEKeysInfo);

    if (infoType == SessionInfoType::kSessionResumption)
    {
        info    = RSEKeysInfo;
        infoLen = sizeof(RSEKeysInfo);
    }

    ReturnErrorOnFailure(
        mHKDF.HKDF_SHA256(secret.data(), secret.size(), salt.data(), salt.size(), info, infoLen, &mKeys[0][0], sizeof(mKeys)));

    mKeyAvailable = true;
    mSessionRole  = role;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Init(const Crypto::P256Keypair & local_keypair, const Crypto::P256PublicKey & remote_public_key,
                               const ByteSpan & salt, SessionInfoType infoType, SessionRole role)
{

    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);

    P256ECDHDerivedSecret secret;
    ReturnErrorOnFailure(local_keypair.ECDH_derive_secret(remote_public_key, secret));

    return InitFromSecret(ByteSpan(secret, secret.Length()), salt, infoType, role);
}

void SecureSession::Reset()
{
    mKeyAvailable = false;
    memset(mKeys, 0, sizeof(mKeys));
}

CHIP_ERROR SecureSession::GetIV(const PacketHeader & header, uint8_t * iv, size_t len)
{

    VerifyOrReturnError(len == kAESCCMIVLen, CHIP_ERROR_INVALID_ARGUMENT);

    Encoding::LittleEndian::BufferWriter bbuf(iv, len);

    bbuf.Put64(header.GetSourceNodeId().ValueOr(0));
    bbuf.Put32(header.GetMessageId());

    return bbuf.Fit() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR SecureSession::GetAdditionalAuthData(const PacketHeader & header, uint8_t * aad, uint16_t & len)
{
    VerifyOrReturnError(len >= header.EncodeSizeBytes(), CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    uint16_t actualEncodedHeaderSize;

    ReturnErrorOnFailure(header.Encode(aad, len, &actualEncodedHeaderSize));
    VerifyOrReturnError(len >= actualEncodedHeaderSize, CHIP_ERROR_INVALID_ARGUMENT);

    len = actualEncodedHeaderSize;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                  MessageAuthenticationCode & mac) const
{

    constexpr Header::EncryptionType encType = Header::EncryptionType::kAESCCMTagLen16;

    const size_t taglen = MessageAuthenticationCode::TagLenForEncryptionType(encType);
    VerifyOrDie(taglen <= kMaxTagLen);

    VerifyOrReturnError(mKeyAvailable, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t AAD[kMaxAADLen];
    uint8_t IV[kAESCCMIVLen];
    uint16_t aadLen = sizeof(AAD);
    uint8_t tag[kMaxTagLen];

    ReturnErrorOnFailure(GetIV(header, IV, sizeof(IV)));
    ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));

    KeyUsage usage = kR2IKey;

    // Message is encrypted before sending. If the secure session was created by session
    // initiator, we'll use I2R key to encrypt the message that's being transmittted.
    // Otherwise, we'll use R2I key, as the responder is sending the message.
    if (mSessionRole == SessionRole::kInitiator)
    {
        usage = kI2RKey;
    }

    ReturnErrorOnFailure(AES_CCM_encrypt(input, input_length, AAD, aadLen, mKeys[usage], kAES_CCM128_Key_Length, IV, sizeof(IV),
                                         output, tag, taglen));

    mac.SetTag(&header, encType, tag, taglen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                  const MessageAuthenticationCode & mac) const
{
    const size_t taglen = MessageAuthenticationCode::TagLenForEncryptionType(header.GetEncryptionType());
    const uint8_t * tag = mac.GetTag();
    uint8_t IV[kAESCCMIVLen];
    uint8_t AAD[kMaxAADLen];
    uint16_t aadLen = sizeof(AAD);

    VerifyOrReturnError(mKeyAvailable, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(GetIV(header, IV, sizeof(IV)));
    ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));

    KeyUsage usage = kI2RKey;

    // Message is decrypted on receive. If the secure session was created by session
    // initiator, we'll use R2I key to decrypt the message (as it was sent by responder).
    // Otherwise, we'll use I2R key, as the responder is sending the message.
    if (mSessionRole == SessionRole::kInitiator)
    {
        usage = kR2IKey;
    }

    return AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen, mKeys[usage], kAES_CCM128_Key_Length, IV, sizeof(IV),
                           output);
}

} // namespace chip
