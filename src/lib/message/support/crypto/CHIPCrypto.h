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
 *      This file provides general purpose cryptographic functions for the CHIP layer.
 *
 */

#ifndef CHIPCRYPTO_H_
#define CHIPCRYPTO_H_

#if CHIP_SEPARATE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stddef.h>
#include <stdint.h>

#include <core/CHIPConfig.h>
#include <core/CHIPError.h>
#include <support/DLLUtil.h>

// If one or more of the enabled CHIP features depends on OpenSSL but CHIP_WITH_OPENSSL has
// NOT been defined, then assume we will be using OpenSSL and set CHIP_WITH_OPENSSL == 1.
#define CHIP_CONFIG_REQUIRES_OPENSSL                                                                                               \
    (CHIP_CONFIG_USE_OPENSSL_ECC || CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL || CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL ||            \
     CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL || CHIP_CONFIG_SUPPORT_PASE_CONFIG1 ||                                                \
     CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT)
#if CHIP_CONFIG_REQUIRES_OPENSSL && !defined(CHIP_WITH_OPENSSL)
#define CHIP_WITH_OPENSSL 1
#endif

#if CHIP_WITH_OPENSSL
#include <openssl/bn.h>
#include <openssl/crypto.h>
#ifdef OPENSSL_IS_BORINGSSL
#include <openssl/mem.h>
#endif // OPENSSL_IS_BORINGSSL
#endif // CHIP_WITH_OPENSSL

/**
 *   @namespace chip::Crypto
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for shared
 *     cryptographic support.
 */

namespace chip {
namespace Crypto {

/**
 * Signature of a function used to gather random data from an entropy source.
 */
typedef int (*EntropyFunct)(uint8_t * buf, size_t bufSize);

extern bool ConstantTimeCompare(const uint8_t * buf1, const uint8_t * buf2, uint16_t len);
extern void ClearSecretData(uint8_t * buf, uint32_t len);

#if CHIP_WITH_OPENSSL

extern CHIP_ERROR EncodeBIGNUMValueLE(const BIGNUM & val, uint16_t size, uint8_t *& p);
extern CHIP_ERROR DecodeBIGNUMValueLE(BIGNUM & val, uint16_t size, const uint8_t *& p);

#endif // CHIP_WITH_OPENSSL

} // namespace Crypto
} // namespace chip

using namespace chip::Crypto;

#include "AESBlockCipher.h"
#include "CHIPRNG.h"

#endif /* CHIPCRYPTO_H_ */
