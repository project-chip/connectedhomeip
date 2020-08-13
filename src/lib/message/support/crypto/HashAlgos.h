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
 *      This file provides SHA1 and SHA256 hash functions for the CHIP layer.
 *      Functions in this file are platform specific and their various custom
 *      implementations can be enabled.
 *
 *      Platforms that wish to provide their own implementation of hash
 *      functions should assert #CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
 *      and create a platform-specific header file containing the necessary
 *      declarations.  Platforms that support hardware acceleration of hash
 *      algorithms are likely to use this option.
 *
 *      The default name for the platform-specific hash implementation header
 *      file is "ChipProjectHashAlgos.h".  This can be overridden by defining
 *      #CHIP_HASH_ALGOS_PLATFORM_INCLUDE.
 *
 *      The platform-specific header file should include declarations of the
 *      SHA_CTX_PLATFORM and SHA256_CTX_PLATFORM context structures.
 *
 */

#ifndef HashAlgos_H_
#define HashAlgos_H_

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

#include "CHIPCrypto.h"

#if CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL && !CHIP_WITH_OPENSSL
#error                                                                                                                             \
    "INVALID CHIP CONFIG: OpenSSL hash implementation enabled but OpenSSL not available (CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL == 1 && CHIP_WITH_OPENSSL == 0)."
#endif

#if CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
#include <openssl/sha.h>
#endif

#if CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#endif

#if CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
#ifndef CHIP_HASH_ALGOS_PLATFORM_INCLUDE
#define CHIP_HASH_ALGOS_PLATFORM_INCLUDE "ChipProjectHashAlgos.h"
#endif
#include CHIP_HASH_ALGOS_PLATFORM_INCLUDE
#endif // CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM

namespace chip {
namespace Platform {
namespace Security {

class DLL_EXPORT SHA1
{
public:
    enum
    {
        kHashLength  = 20,
        kBlockLength = 64
    };

    SHA1(void);
    ~SHA1(void);

    void Begin(void);
    void AddData(const uint8_t * data, uint16_t dataLen);
#if CHIP_WITH_OPENSSL
    void AddData(const BIGNUM & num);
#endif
    void Finish(uint8_t * hashBuf);
    void Reset(void);

private:
#if CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
    SHA_CTX mSHACtx;
#elif CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
    mbedtls_sha1_context mSHACtx;
#elif CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
    SHA_CTX_PLATFORM mSHACtx;
#endif
};

class SHA256
{
public:
    enum
    {
        kHashLength  = 32,
        kBlockLength = 64
    };

    SHA256(void);
    ~SHA256(void);

    void Begin(void);
    void AddData(const uint8_t * data, uint16_t dataLen);
#if CHIP_WITH_OPENSSL
    void AddData(const BIGNUM & num);
#endif
    void Finish(uint8_t * hashBuf);
    void Reset(void);

private:
#if CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
    SHA256_CTX mSHACtx;
#elif CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
    mbedtls_sha256_context mSHACtx;
#elif CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
    SHA256_CTX_PLATFORM mSHACtx;
#endif
};

} // namespace Security
} // namespace Platform
} // namespace chip

#endif /* HashAlgos_H_ */
