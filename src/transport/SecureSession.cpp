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
 *      This file implements the CHIP Secure Session object.
 *
 */

#include <core/CHIPEncoding.h>
#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <transport/SecureSession.h>
#include <transport/raw/MessageHeader.h>

#include <string.h>

namespace chip {

namespace {

constexpr size_t kAESCCMIVLen = 12;
constexpr size_t kMaxAADLen   = 128;

} // namespace

using namespace Crypto;

SecureSession::SecureSession() : mKeyAvailable(false) {}

CHIP_ERROR SecureSession::InitFromSecret(const uint8_t * secret, const size_t secret_length, const uint8_t * salt,
                                         const size_t salt_length, const uint8_t * info, const size_t info_length)
{

    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(secret != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(secret_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((salt_length == 0) || (salt != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(HKDF_SHA256(secret, secret_length, salt, salt_length, info, info_length, mKey, sizeof(mKey)));

    mKeyAvailable = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Init(const Crypto::P256Keypair & local_keypair, const Crypto::P256PublicKey & remote_public_key,
                               const uint8_t * salt, const size_t salt_length, const uint8_t * info, const size_t info_length)
{

    VerifyOrReturnError(mKeyAvailable == false, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((salt_length == 0) || (salt != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    P256ECDHDerivedSecret secret;
    ReturnErrorOnFailure(local_keypair.ECDH_derive_secret(remote_public_key, secret));

    return InitFromSecret(secret, secret.Length(), salt, salt_length, info, info_length);
}

void SecureSession::Reset()
{
    mKeyAvailable = false;
    memset(mKey, 0, sizeof(mKey));
}

CHIP_ERROR SecureSession::GetIV(const PacketHeader & header, uint8_t * iv, size_t len)
{

    VerifyOrReturnError(len == kAESCCMIVLen, CHIP_ERROR_INVALID_ARGUMENT);

    BufBound bbuf(iv, len);

    bbuf.Put64(header.GetSourceNodeId().ValueOr(0));
    bbuf.Put32(header.GetMessageId());

    return bbuf.Fit() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR SecureSession::GetAdditionalAuthData(const PacketHeader & header, const Header::Flags payloadEncodeFlags, uint8_t * aad,
                                                uint16_t & len)
{
    VerifyOrReturnError(len >= header.EncodeSizeBytes(), CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    uint16_t actualEncodedHeaderSize;

    ReturnErrorOnFailure(header.Encode(aad, len, &actualEncodedHeaderSize, payloadEncodeFlags));
    VerifyOrReturnError(len >= actualEncodedHeaderSize, CHIP_ERROR_INVALID_ARGUMENT);

    len = actualEncodedHeaderSize;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                  Header::Flags payloadFlags, MessageAuthenticationCode & mac)
{

    constexpr Header::EncryptionType encType = Header::EncryptionType::kAESCCMTagLen16;

    const size_t taglen = MessageAuthenticationCode::TagLenForEncryptionType(encType);
    assert(taglen <= kMaxTagLen);

    VerifyOrReturnError(mKeyAvailable, CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t AAD[kMaxAADLen];
    uint8_t IV[kAESCCMIVLen];
    uint16_t aadLen = sizeof(AAD);
    uint8_t tag[kMaxTagLen];

    ReturnErrorOnFailure(GetIV(header, IV, sizeof(IV)));
    ReturnErrorOnFailure(GetAdditionalAuthData(header, payloadFlags, AAD, aadLen));
    ReturnErrorOnFailure(
        AES_CCM_encrypt(input, input_length, AAD, aadLen, mKey, sizeof(mKey), IV, sizeof(IV), output, tag, taglen));

    mac.SetTag(&header, encType, tag, taglen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSession::Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                  Header::Flags payloadFlags, const MessageAuthenticationCode & mac)
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
    ReturnErrorOnFailure(GetAdditionalAuthData(header, payloadFlags, AAD, aadLen));

    return AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen, mKey, sizeof(mKey), IV, sizeof(IV), output);
}

} // namespace chip
