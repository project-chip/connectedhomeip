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
 *      This file provides AES block cipher functions for the CHIP layer.
 *      Functions in this file are platform specific and their various custom
 *      implementations can be enabled.
 *
 *      Platforms that wish to provide their own implementation of AES
 *      functions should assert #CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM
 *      and create a platform-specific header file containing the necessary
 *      declarations.  The name of the platform-specific header file must be
 *      specified via the #CHIP_AES_BLOCK_CIPHER_PLATFORM_INCLUDE macro.
 *
 *      Platforms that that require specific AES context data should define
 *      the #CHIP_AES_128_CTX_PLATFORM and #CHIP_AES_256_CTX_PLATFORM macros
 *      accordingly.
 *
 */

#ifndef AES_H_
#define AES_H_

#include <limits.h>

#include "CHIPCrypto.h"

#if CHIP_CRYPTO_OPENSSL
#include <openssl/aes.h>
#endif

#if CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
#include <wmmintrin.h>
#endif

#if CHIP_CRYPTO_MBEDTLS
#include <mbedtls/aes.h>
#endif

#if CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM && defined(CHIP_AES_BLOCK_CIPHER_PLATFORM_INCLUDE)
#include CHIP_AES_BLOCK_CIPHER_PLATFORM_INCLUDE
#endif

namespace chip {
namespace Platform {
namespace Security {

class AES128BlockCipher
{
public:
    enum
    {
        kKeyLength     = 16,
        kKeyLengthBits = kKeyLength * CHAR_BIT,
        kBlockLength   = 16,
        kRoundCount    = 10
    };

    void Reset(void);

protected:
    AES128BlockCipher(void);
    ~AES128BlockCipher(void);

#if CHIP_CRYPTO_OPENSSL
    AES_KEY mKey;
#elif CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
    __m128i mKey[kRoundCount + 1];
#elif CHIP_CRYPTO_MBEDTLS
    mbedtls_aes_context mCtx;
#elif CHIP_CONFIG_AES_USE_EXPANDED_KEY
    uint8_t mKey[kBlockLength * (kRoundCount + 1)];
#elif defined(CHIP_AES_128_CTX_PLATFORM)
    CHIP_AES_128_CTX_PLATFORM mCtx;
#else
    uint8_t mKey[kKeyLength];
#endif
};

class DLL_EXPORT AES128BlockCipherEnc : public AES128BlockCipher
{
public:
    void SetKey(const uint8_t * key);
    void EncryptBlock(const uint8_t * inBlock, uint8_t * outBlock);
};

class DLL_EXPORT AES128BlockCipherDec : public AES128BlockCipher
{
public:
    void SetKey(const uint8_t * key);
    void DecryptBlock(const uint8_t * inBlock, uint8_t * outBlock);
};

class AES256BlockCipher
{
public:
    enum
    {
        kKeyLength     = 32,
        kKeyLengthBits = kKeyLength * CHAR_BIT,
        kBlockLength   = 16,
        kRoundCount    = 14
    };

    void Reset(void);

protected:
    AES256BlockCipher(void);
    ~AES256BlockCipher(void);

#if CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
    AES_KEY mKey;
#elif CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
    __m128i mKey[kRoundCount + 1];
#elif CHIP_CRYPTO_MBEDTLS
    mbedtls_aes_context mCtx;
#elif CHIP_CONFIG_AES_USE_EXPANDED_KEY
    uint8_t mKey[kBlockLength * (kRoundCount + 1)];
#elif defined(CHIP_AES_256_CTX_PLATFORM)
    CHIP_AES_256_CTX_PLATFORM mCtx;
#else
    uint8_t mKey[kKeyLength];
#endif
};

class DLL_EXPORT AES256BlockCipherEnc : public AES256BlockCipher
{
public:
    void SetKey(const uint8_t * key);
    void EncryptBlock(const uint8_t * inBlock, uint8_t * outBlock);
};

class DLL_EXPORT AES256BlockCipherDec : public AES256BlockCipher
{
public:
    void SetKey(const uint8_t * key);
    void DecryptBlock(const uint8_t * inBlock, uint8_t * outBlock);
};

} // namespace Security
} // namespace Platform
} // namespace chip

#endif /* AES_H_ */
