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

int CHIP_aes_ccm_encrypt(unsigned char * plaintext, int plaintext_len, unsigned char * aad, int aad_len, unsigned char * key,
                         unsigned char * iv, unsigned char * ciphertext, unsigned char * tag)
{
    // TODO: Need to implement openSSL based AES-CCM #256
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}

int CHIP_aes_ccm_decrypt(unsigned char * ciphertext, int ciphertext_len, unsigned char * aad, int aad_len, unsigned char * tag,
                         unsigned char * key, unsigned char * iv, unsigned char * plaintext);
{
    // TODO: Need to implement openSSL based AES-CCM #256
    return CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE;
}