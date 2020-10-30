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
#include <transport/SecureSession.h>
#include <transport/raw/MessageHeader.h>

#include <string.h>

namespace chip {

namespace {

constexpr size_t kAESCCMIVLen = 12;
constexpr size_t kMaxAADLen   = 128;

} // namespace

using namespace Crypto;

SecureSession::SecureSession() : mState(State::kNotInitialized) {}

CHIP_ERROR SecureSession::InitFromSecret(const uint8_t * secret, const size_t secret_length, const uint8_t * salt,
                                         const size_t salt_length, const uint8_t * info, const size_t info_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kNotInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(secret != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (salt_length > 0)
    {
        VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = HKDF_SHA256(secret, secret_length, salt, salt_length, info, info_length, mKey, sizeof(mKey));
    SuccessOrExit(error);

    mState = State::kInitializedSecure;

exit:
    return error;
}

CHIP_ERROR SecureSession::Init(const Crypto::P256Keypair & local_keypair, const Crypto::P256PublicKey & remote_public_key,
                               const uint8_t * salt, const size_t salt_length, const uint8_t * info, const size_t info_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    P256ECDHDerivedSecret secret;

    VerifyOrExit(mState == State::kNotInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    if (salt_length > 0)
    {
        VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = local_keypair.ECDH_derive_secret(remote_public_key, secret);
    SuccessOrExit(error);

    error = InitFromSecret(secret, secret.Length(), salt, salt_length, info, info_length);

exit:
    return error;
}

CHIP_ERROR SecureSession::InitUnsecure()
{
    mState = State::kInitializedUnsecure;
    return CHIP_NO_ERROR;
}

void SecureSession::Reset()
{
    mState = State::kNotInitialized;
    memset(mKey, 0, sizeof(mKey));
}

CHIP_ERROR SecureSession::GetIV(const PacketHeader & header, uint8_t * iv, size_t len)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint64_t nodeID = 0;

    BufBound bbuf(iv, len);

    VerifyOrExit(len == kAESCCMIVLen, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (header.GetSourceNodeId().HasValue())
    {
        nodeID = header.GetSourceNodeId().Value();
    }

    bbuf.PutLE64(nodeID);
    bbuf.PutLE32(header.GetMessageId());
    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);

exit:
    return err;
}

CHIP_ERROR SecureSession::GetAdditionalAuthData(const PacketHeader & header, const Header::Flags payloadEncodeFlags, uint8_t * aad,
                                                uint16_t & len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t actualEncodedHeaderSize;

    VerifyOrExit(len >= header.EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    err = header.Encode(aad, len, &actualEncodedHeaderSize, payloadEncodeFlags);
    SuccessOrExit(err);

    VerifyOrExit(len >= actualEncodedHeaderSize, err = CHIP_ERROR_INVALID_ARGUMENT);
    len = actualEncodedHeaderSize;

exit:
    return err;
}

CHIP_ERROR SecureSession::Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                  Header::Flags payloadFlags, MessageAuthenticationCode & mac)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t IV[kAESCCMIVLen];
    uint8_t AAD[kMaxAADLen];
    uint16_t aadLen = sizeof(AAD);

    constexpr Header::EncryptionType encType = Header::EncryptionType::kAESCCMTagLen16;

    const size_t taglen = MessageAuthenticationCode::TagLenForEncryptionType(encType);
    uint8_t tag[taglen];

    VerifyOrExit(input != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState != State::kNotInitialized, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    if (mState == State::kInitializedSecure)
    {
        error = GetIV(header, IV, sizeof(IV));
        SuccessOrExit(error);

        error = GetAdditionalAuthData(header, payloadFlags, AAD, aadLen);
        SuccessOrExit(error);

        error = AES_CCM_encrypt(input, input_length, AAD, aadLen, mKey, sizeof(mKey), IV, sizeof(IV), output, tag, taglen);
        SuccessOrExit(error);

        mac.SetTag(&header, encType, tag, taglen);
    }
    else
    {
        memcpy(output, input, input_length);
    }

exit:
    return error;
}

CHIP_ERROR SecureSession::Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                  Header::Flags payloadFlags, const MessageAuthenticationCode & mac)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    size_t taglen       = MessageAuthenticationCode::TagLenForEncryptionType(header.GetEncryptionType());
    const uint8_t * tag = mac.GetTag();
    uint8_t IV[kAESCCMIVLen];
    uint8_t AAD[kMaxAADLen];
    uint16_t aadLen = sizeof(AAD);
    VerifyOrExit(input != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mState != State::kInitializedSecure, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    if (mState == State::kInitializedSecure)
    {
        error = GetIV(header, IV, sizeof(IV));
        SuccessOrExit(error);

        error = GetAdditionalAuthData(header, payloadFlags, AAD, aadLen);
        SuccessOrExit(error);

        error = AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen, mKey, sizeof(mKey), IV, sizeof(IV), output);
    }
    else
    {
        memcpy(output, input, input_length);
    }
exit:
    return error;
}

} // namespace chip
