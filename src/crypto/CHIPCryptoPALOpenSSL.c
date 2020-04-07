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

CHIP_ERROR CHIP_aes_ccm_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                                    size_t aad_length, const unsigned char * key, const unsigned char * iv, size_t iv_length,
                                    unsigned char * ciphertext, size_t & ciphertext_length, unsigned char * tag, size_t tag_length)
{
    // TODO: Need to implement openSSL based AES-CCM #256
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}

CHIP_ERROR CHIP_aes_ccm_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_len, const unsigned char * aad,
                                    size_t aad_len, const unsigned char * tag, size_t tag_length, const unsigned char * key,
                                    const unsigned char * iv, size_t iv_length, unsigned char * plaintext)
{
    // TODO: Need to implement openSSL based AES-CCM #256
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}