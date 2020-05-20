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
 *      This file implements the CHIP Secure Channel object.
 *
 */

#include <core/CHIPSecureChannel.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/CodeUtils.h>

namespace chip {

using namespace Crypto;

ChipSecureChannel::ChipSecureChannel() : mKeyAvailable(false), mNextIV(0) {}

CHIP_ERROR ChipSecureChannel::Init(const unsigned char * remote_public_key, const size_t public_key_length,
                                   const unsigned char * local_private_key, const size_t private_key_length,
                                   const unsigned char * salt, const size_t salt_length, const unsigned char * info,
                                   const size_t info_length)
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

CHIP_ERROR ChipSecureChannel::Encrypt(const unsigned char * input, size_t input_length, unsigned char * output,
                                      size_t output_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint64_t tag     = 0;
    security_header_t header;
    size_t overhead = EncryptionOverhead();

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output_length >= overhead + input_length, error = CHIP_ERROR_INVALID_ARGUMENT);

    header.payload_length = input_length;
    header.IV             = mNextIV;
    header.tag            = 0;

    error = AES_CCM_encrypt(input, input_length, (const unsigned char *) &header, sizeof(header), mKey, sizeof(mKey),
                            (const unsigned char *) &header.IV, sizeof(header.IV), &output[overhead], (unsigned char *) &tag,
                            sizeof(tag));
    if (error == CHIP_NO_ERROR)
    {
        header.tag = tag;
        memcpy(output, &header, sizeof(header));
        mNextIV++;
    }

exit:
    return error;
}

CHIP_ERROR ChipSecureChannel::Decrypt(const unsigned char * input, size_t input_length, unsigned char * output,
                                      size_t & output_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint64_t tag     = 0;
    security_header_t header;
    size_t overhead = EncryptionOverhead();

    VerifyOrExit(mKeyAvailable, error = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(input != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length > overhead, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(&header, input, sizeof(header));
    tag        = header.tag;
    header.tag = 0;

    VerifyOrExit(output_length >= header.payload_length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input_length >= header.payload_length + overhead, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = AES_CCM_decrypt(&input[overhead], header.payload_length, (const unsigned char *) &header, sizeof(header),
                            (const unsigned char *) &tag, sizeof(tag), mKey, sizeof(mKey), (const unsigned char *) &header.IV,
                            sizeof(header.IV), output);
    if (error == CHIP_NO_ERROR)
    {
        output_length = header.payload_length;
    }
exit:
    return error;
}

size_t ChipSecureChannel::EncryptionOverhead(void)
{
    return sizeof(security_header_t);
}

} // namespace chip
