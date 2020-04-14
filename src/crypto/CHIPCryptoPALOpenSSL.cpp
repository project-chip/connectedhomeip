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
 *      openSSL based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

#include <openssl/conf.h>
#include <openssl/rand_drbg.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/kdf.h>
#include <support/CodeUtils.h>
#include <string.h>

#define kKeyLengthInBits 256

static bool _isValidTagLength(size_t tag_length)
{
    if (tag_length == 8 || tag_length == 12 || tag_length == 16)
    {
        return true;
    }
    return false;
}

CHIP_ERROR chip::Crypto::AES_CCM_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                                             size_t aad_length, const unsigned char * key, const unsigned char * iv,
                                             size_t iv_length, unsigned char * ciphertext, unsigned char * tag, size_t tag_length)
{
    EVP_CIPHER_CTX * context = NULL;
    int bytesWritten         = 0;
    size_t ciphertext_length = 0;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int result               = 1;

    VerifyOrExit(plaintext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_EncryptInit_ex(context, EVP_aes_256_ccm(), NULL, NULL, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, iv_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in tag length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, tag_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_EncryptInit_ex(context, NULL, NULL, key, iv);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in plain text length
    result = EVP_EncryptUpdate(context, NULL, &bytesWritten, NULL, plaintext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in AAD
    if (aad_length > 0 && aad != NULL)
    {
        result = EVP_EncryptUpdate(context, NULL, &bytesWritten, aad, aad_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Encrypt
    result = EVP_EncryptUpdate(context, ciphertext, &bytesWritten, plaintext, plaintext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length = bytesWritten;

    // Finalize encryption
    result = EVP_EncryptFinal_ex(context, ciphertext + ciphertext_length, &bytesWritten);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length += bytesWritten;

    // Get tag
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_GET_TAG, tag_length, tag);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_CIPHER_CTX_free(context);
        context = NULL;
    }

    return error;
}

CHIP_ERROR chip::Crypto::AES_CCM_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_length, const unsigned char * aad,
                                             size_t aad_length, const unsigned char * tag, size_t tag_length,
                                             const unsigned char * key, const unsigned char * iv, size_t iv_length,
                                             unsigned char * plaintext)
{
    EVP_CIPHER_CTX * context = NULL;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int bytesOutput          = 0;
    int result               = 1;

    VerifyOrExit(ciphertext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_DecryptInit_ex(context, EVP_aes_256_ccm(), NULL, NULL, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, iv_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in expected tag
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, tag_length, (void *) tag);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_DecryptInit_ex(context, NULL, NULL, key, iv);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher text length
    result = EVP_DecryptUpdate(context, NULL, &bytesOutput, NULL, ciphertext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in aad
    if (aad_length > 0 && aad != NULL)
    {
        result = EVP_DecryptUpdate(context, NULL, &bytesOutput, aad, aad_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Pass in ciphertext. We wont get anything if validation fails.
    result = EVP_DecryptUpdate(context, plaintext, &bytesOutput, ciphertext, ciphertext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_CIPHER_CTX_free(context);
        context = NULL;
    }

    return error;
}

CHIP_ERROR chip::Crypto::HKDF_SHA256(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                                     const size_t salt_length, const unsigned char * info, const size_t info_length,
                                     unsigned char * out_buffer, size_t out_length)
{
    EVP_PKEY_CTX * context;
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    context = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(secret != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Info is optional
    if (info_length > 0)
    {
        VerifyOrExit(info != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_set_hkdf_md(context, EVP_sha256());
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_set1_hkdf_key(context, secret, secret_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    if (salt_length > 0 && salt != NULL)
    {
        result = EVP_PKEY_CTX_set1_hkdf_salt(context, salt, salt_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    if (info_length > 0 && info != NULL)
    {
        result = EVP_PKEY_CTX_add1_hkdf_info(context, info, info_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    result = EVP_PKEY_CTX_hkdf_mode(context, EVP_PKEY_HKDEF_MODE_EXTRACT_AND_EXPAND);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Get the OKM (Output Key Material)
    result = EVP_PKEY_derive(context, out_buffer, &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_PKEY_CTX_free(context);
    }
    return error;
}

CHIP_ERROR chip::Crypto::DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = RAND_DRBG_bytes(RAND_DRBG_get0_private(), out_buffer, out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}
