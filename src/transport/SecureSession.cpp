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
#include <transport/SecureSession.h>
#include <transport/SecurityHeader.h>

#include <string.h>

namespace chip {

using namespace Crypto;

SecureSession::SecureSession() : mKeyAvailable(false), mNextIV(0) {}

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

    mNextIV = 0;

    error = ECDH_derive_secret(remote_public_key, public_key_length, local_private_key, private_key_length, secret, secret_size);
    if (error == CHIP_NO_ERROR)
    {
        error         = HKDF_SHA256(secret, sizeof(secret), salt, salt_length, info, info_length, mKey, sizeof(mKey));
        mKeyAvailable = (error == CHIP_NO_ERROR);
    }

exit:
    return error;
}

void SecureSession::Close(void)
{
    mKeyAvailable = false;
    mNextIV       = 0;
}

CHIP_ERROR SecureSession::Encrypt(const unsigned char * input, size_t input_length, unsigned char * output, size_t & output_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t overhead  = EncryptionOverhead();
    uint64_t tag     = 0;
    SecurityHeader header;

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output_length >= overhead + input_length, error = CHIP_ERROR_INVALID_ARGUMENT);

    header.SetIV(mNextIV).SetTag(tag);
    error =
        AES_CCM_encrypt(input, input_length, header.RawHeader(), SecurityHeader::RawHeaderLen(), mKey, sizeof(mKey),
                        (const unsigned char *) &mNextIV, sizeof(mNextIV), &output[overhead], (unsigned char *) &tag, sizeof(tag));
    SuccessOrExit(error);

    header.SetTag(tag);
    error = header.Serialize(output, overhead);
    SuccessOrExit(error);

    mNextIV++;
    output_length = overhead + input_length;

exit:
    return error;
}

CHIP_ERROR SecureSession::Decrypt(const unsigned char * input, size_t input_length, unsigned char * output, size_t & output_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    size_t overhead  = EncryptionOverhead();
    uint64_t length  = input_length - overhead;
    uint64_t tag     = 0;
    uint64_t IV      = 0;
    SecurityHeader header;

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > overhead, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output_length >= length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = header.Deserialize(input, overhead);
    SuccessOrExit(error);

    tag = header.Tag();
    IV  = header.IV();

    // The tag is not used in the AAD buffer. So 0 it out.
    header.SetTag(0);

    error =
        AES_CCM_decrypt(&input[overhead], length, header.RawHeader(), SecurityHeader::RawHeaderLen(), (const unsigned char *) &tag,
                        sizeof(tag), mKey, sizeof(mKey), (const unsigned char *) &IV, sizeof(IV), output);
    SuccessOrExit(error);
    output_length = length;

exit:
    return error;
}

size_t SecureSession::EncryptionOverhead(void)
{
    return SecurityHeader::RawHeaderLen();
}

} // namespace chip
