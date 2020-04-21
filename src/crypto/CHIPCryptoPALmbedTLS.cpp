/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      mbedTLS based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include <support/CodeUtils.h>
#include <string.h>

CHIP_ERROR chip::Crypto::AES_CCM_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                                             size_t aad_length, const unsigned char * key, const unsigned char * iv,
                                             size_t iv_length, unsigned char * ciphertext, unsigned char * tag, size_t tag_length)
{
    // TODO: Need mbedTLS based implementation for AES-CCM #264
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}

CHIP_ERROR chip::Crypto::AES_CCM_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_len, const unsigned char * aad,
                                             size_t aad_len, const unsigned char * tag, size_t tag_length,
                                             const unsigned char * key, const unsigned char * iv, size_t iv_length,
                                             unsigned char * plaintext)
{
    // TODO: Need mbedTLS based implementation for AES-CCM #264
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}

CHIP_ERROR chip::Crypto::HKDF_SHA256(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                                     const size_t salt_length, const unsigned char * info, const size_t info_length,
                                     unsigned char * out_buffer, size_t out_length)
{
    // TODO: Need mbedTLS based implementation for HKDF
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}

static mbedtls_ctr_drbg_context * get_mbedtls_drbg_context()
{
    static mbedtls_ctr_drbg_context drbg_ctxt;
    static bool initialized = false;

    if (initialized)
    {
        return &drbg_ctxt;
    }

    static mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);

    mbedtls_ctr_drbg_context * ctxt = NULL;
    mbedtls_ctr_drbg_init(&drbg_ctxt);

    int status = mbedtls_ctr_drbg_seed(&drbg_ctxt, mbedtls_entropy_func, &entropy, NULL, 0);
    if (status == 0)
    {
        initialized = true;
        ctxt        = &drbg_ctxt;
    }

    return ctxt;
}

CHIP_ERROR chip::Crypto::DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length)
{
    CHIP_ERROR error                     = CHIP_NO_ERROR;
    int result                           = 0;
    mbedtls_ctr_drbg_context * drbg_ctxt = NULL;

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    drbg_ctxt = get_mbedtls_drbg_context();
    VerifyOrExit(drbg_ctxt != NULL, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ctr_drbg_random(drbg_ctxt, out_buffer, out_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}
