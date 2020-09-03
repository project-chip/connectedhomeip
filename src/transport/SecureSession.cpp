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
#include <crypto/CHIPCryptoPAL.h>
#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <transport/MessageHeader.h>
#include <transport/SecureSession.h>

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
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(mKeyAvailable == false, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(secret != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (salt_length > 0)
    {
        VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = HKDF_SHA256(secret, secret_length, salt, salt_length, info, info_length, mKey, sizeof(mKey));
    SuccessOrExit(error);

    mKeyAvailable = true;

exit:
    return error;
}

CHIP_ERROR SecureSession::Init(const uint8_t * remote_public_key, const size_t public_key_length, const uint8_t * local_private_key,
                               const size_t private_key_length, const uint8_t * salt, const size_t salt_length,
                               const uint8_t * info, const size_t info_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t secret[kMax_ECDH_Secret_Length];
    size_t secret_size = sizeof(secret);

    VerifyOrExit(mKeyAvailable == false, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(remote_public_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(public_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(local_private_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(private_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (salt_length > 0)
    {
        VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = ECDH_derive_secret(remote_public_key, public_key_length, local_private_key, private_key_length, secret, secret_size);
    SuccessOrExit(error);

    error = InitFromSecret(secret, secret_size, salt, salt_length, info, info_length);

exit:
    return error;
}

void SecureSession::Reset(void)
{
    mKeyAvailable = false;
    memset(mKey, 0, sizeof(mKey));
}

CHIP_ERROR SecureSession::GetIV(const MessageHeader & header, uint8_t * iv, size_t len)
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

CHIP_ERROR SecureSession::GetAdditionalAuthData(const MessageHeader & header, uint8_t * aad, size_t & len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t actualEncodedHeaderSize;

    VerifyOrExit(len >= header.EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    err = header.Encode(aad, len, &actualEncodedHeaderSize);
    SuccessOrExit(err);

    VerifyOrExit(len >= actualEncodedHeaderSize, err = CHIP_ERROR_INVALID_ARGUMENT);
    len = actualEncodedHeaderSize;

exit:
    return err;
}

CHIP_ERROR SecureSession::Encrypt(const uint8_t * input, size_t input_length, uint8_t * output, MessageHeader & header)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t IV[kAESCCMIVLen];
    uint8_t AAD[kMaxAADLen];
    size_t aadLen = sizeof(AAD);

    MessageHeader::EncryptionType encType = MessageHeader::EncryptionType::kAESCCMTagLen16;

    const size_t taglen = header.TagLenForEncryptionType(encType);
    uint8_t tag[taglen];

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = GetIV(header, IV, sizeof(IV));
    SuccessOrExit(error);

    error = GetAdditionalAuthData(header, AAD, aadLen);
    SuccessOrExit(error);

    error = AES_CCM_encrypt(input, input_length, AAD, aadLen, mKey, sizeof(mKey), IV, sizeof(IV), output, tag, taglen);
    SuccessOrExit(error);

    header.SetTag(encType, tag, taglen);

exit:
    return error;
}

CHIP_ERROR SecureSession::Decrypt(const uint8_t * input, size_t input_length, uint8_t * output, const MessageHeader & header)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    size_t taglen       = header.GetTagLength();
    const uint8_t * tag = header.GetTag();
    uint8_t IV[kAESCCMIVLen];
    uint8_t AAD[kMaxAADLen];
    size_t aadLen = sizeof(AAD);

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = GetIV(header, IV, sizeof(IV));
    SuccessOrExit(error);

    error = GetAdditionalAuthData(header, AAD, aadLen);
    SuccessOrExit(error);

    error = AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen, mKey, sizeof(mKey), IV, sizeof(IV), output);
exit:
    return error;
}

} // namespace chip
