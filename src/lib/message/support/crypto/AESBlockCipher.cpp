/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements AES block cipher functions for the Weave layer.
 *      The implementation is based on the OpenSSL library functions.
 *      This implementation is used when #CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
 *      is enabled (1).
 *
 */

#include "AESBlockCipher.h"
#include "CHIPCrypto.h"

#include <string.h>

namespace chip {
namespace Platform {
namespace Security {

using namespace chip::Crypto;

AES128BlockCipher::AES128BlockCipher()
{
    memset(&mKey, 0, sizeof(mKey));
}

AES128BlockCipher::~AES128BlockCipher()
{
    Reset();
}

void AES128BlockCipher::Reset()
{
    ClearSecretData((uint8_t *) &mKey, sizeof(mKey));
}

void AES128BlockCipherEnc::SetKey(const uint8_t * key)
{
    // AES_set_encrypt_key(key, kKeyLengthBits, &mKey);
}

void AES128BlockCipherEnc::EncryptBlock(const uint8_t * inBlock, uint8_t * outBlock)
{
    // AES_encrypt(inBlock, outBlock, &mKey);
}

void AES128BlockCipherDec::SetKey(const uint8_t * key)
{
    // AES_set_decrypt_key(key, kKeyLengthBits, &mKey);
}

void AES128BlockCipherDec::DecryptBlock(const uint8_t * inBlock, uint8_t * outBlock)
{
    // AES_decrypt(inBlock, outBlock, &mKey);
}

AES256BlockCipher::AES256BlockCipher()
{
    memset(&mKey, 0, sizeof(mKey));
}

AES256BlockCipher::~AES256BlockCipher()
{
    Reset();
}

void AES256BlockCipher::Reset()
{
    ClearSecretData((uint8_t *) &mKey, sizeof(mKey));
}

void AES256BlockCipherEnc::SetKey(const uint8_t * key)
{
    // AES_set_encrypt_key(key, kKeyLengthBits, &mKey);
}

void AES256BlockCipherEnc::EncryptBlock(const uint8_t * inBlock, uint8_t * outBlock)
{
    // AES_encrypt(inBlock, outBlock, &mKey);
}

void AES256BlockCipherDec::SetKey(const uint8_t * key)
{
    // AES_set_decrypt_key(key, kKeyLengthBits, &mKey);
}

void AES256BlockCipherDec::DecryptBlock(const uint8_t * inBlock, uint8_t * outBlock)
{
    // AES_decrypt(inBlock, outBlock, &mKey);
}

} // namespace Security
} // namespace Platform
} // namespace chip
