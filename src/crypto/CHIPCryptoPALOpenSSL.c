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
#include <openssl/evp.h>
#include <openssl/err.h>
#include <support/CodeUtils.h>

#define kKeyLengthInBits 256

bool _isValidTagLength(size_t tag_length)
{
    if (tag_length == 8 || tag_length == 12 || tag_length == 16) {
        return true;
    }
    return false;
}

CHIP_ERROR CHIP_aes_ccm_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                                    size_t aad_length, const unsigned char * key, const unsigned char * iv, size_t iv_length,
                                    unsigned char * ciphertext, size_t & ciphertext_length, unsigned char * tag, size_t tag_length)
{
    EVP_CIPHER_CTX * context = NULL;
    size_t bytesWritten            = 0;
    size_t ciphertext_length = 0;
    CHIP_ERROR error         = CHIP_NO_ERROR;

    VerifyOrExit((plaintext != NULL && plaintext_len > 0), error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad)
    {
        VerifyOrExit(aad_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(strlen(key) * sizeof(unsigned char) == kKeyLengthInBits, error == CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL && iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);


    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    int result = EVP_EncryptInit_ex(context, EVP_aes_256_ccm(), NULL, NULL, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, iv_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in tag length
    result = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, tag_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in plain text length
    result = EVP_EncryptUpdate(ctx, NULL, &bytesWritten, NULL, plaintext_len);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in AAD
    if (aad)  {
        result = EVP_EncryptUpdate(ctx, NULL, &bytesWritten, aad, aad_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Encrypt
    result = EVP_EncryptUpdate(ctx, ciphertext, &bytesWritten, plaintext, plaintext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length = bytesWritten;

    // Finalize encryption
    result = EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_length, &bytesWritten);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length += bytesWritten;

        exit : if (context != NULL)
    {
        EVP_CIPHER_CTX_free(context);
        context = null;
    }

    return error;
}

CHIP_ERROR CHIP_aes_ccm_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_len, const unsigned char * aad,
                                    size_t aad_len, const unsigned char * tag, size_t tag_length, const unsigned char * key,
                                    const unsigned char * iv, size_t iv_length, unsigned char * plaintext)
{
    // TODO: Need to implement openSSL based AES-CCM #256
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}