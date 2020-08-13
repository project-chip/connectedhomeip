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
#include <support/CodeUtils.h>
#include <transport/MessageHeader.h>
#include <transport/SecureSession.h>

#include <string.h>

namespace chip {

namespace {

const char * kManualKeyExchangeChannelInfo = "Manual Key Exchanged Channel";

} // namespace

using namespace Crypto;

SecureSession::SecureSession() : mKeyAvailable(false) {}

CHIP_ERROR SecureSession::InitFromSecret(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                                         const size_t salt_length, const unsigned char * info, const size_t info_length)
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

CHIP_ERROR SecureSession::Init(const unsigned char * remote_public_key, const size_t public_key_length,
                               const unsigned char * local_private_key, const size_t private_key_length, const unsigned char * salt,
                               const size_t salt_length, const unsigned char * info, const size_t info_length)
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

uint64_t SecureSession::GetIV(const MessageHeader & header)
{
    // The message ID is a 4 byte value. It's assumed that the security
    // session will be rekeyed before (or on) message ID rollover.
    uint64_t IV = header.GetMessageId() & 0xffffffff;

    if (header.GetSourceNodeId().HasValue())
    {
        uint64_t nodeID = header.GetSourceNodeId().Value();
        IV |= nodeID & 0xffffffff00000000;
        IV |= (nodeID & 0xffffffff) << 32;
    }
    return IV;
}

CHIP_ERROR SecureSession::Encrypt(const unsigned char * input, size_t input_length, unsigned char * output, MessageHeader & header)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint64_t tag     = 0;
    uint64_t IV      = SecureSession::GetIV(header);

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = AES_CCM_encrypt(input, input_length, NULL, 0, mKey, sizeof(mKey), (const unsigned char *) &IV, sizeof(IV), output,
                            (unsigned char *) &tag, sizeof(tag));
    SuccessOrExit(error);

    header.SetTag(MessageHeader::EncryptionType::kAESCCMTagLen8, (uint8_t *) &tag, sizeof(tag));

exit:
    return error;
}

CHIP_ERROR SecureSession::Decrypt(const unsigned char * input, size_t input_length, unsigned char * output,
                                  const MessageHeader & header)
{
    CHIP_ERROR error    = CHIP_NO_ERROR;
    size_t taglen       = header.GetTagLength();
    const uint8_t * tag = header.GetTag();
    uint64_t IV         = SecureSession::GetIV(header);

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = AES_CCM_decrypt(input, input_length, NULL, 0, (const unsigned char *) tag, taglen, mKey, sizeof(mKey),
                            (const unsigned char *) &IV, sizeof(IV), output);
exit:
    return error;
}

CHIP_ERROR SecureSession::TemporaryManualKeyExchange(const unsigned char * remote_public_key, const size_t public_key_length,
                                                     const unsigned char * local_private_key, const size_t private_key_length)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    size_t info_len = strlen(kManualKeyExchangeChannelInfo);

    err = Init(remote_public_key, public_key_length, local_private_key, private_key_length, NULL, 0,
               (const unsigned char *) kManualKeyExchangeChannelInfo, info_len);
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace chip
