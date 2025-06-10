/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      Header that exposes some ECC-related API which used by P256KeyPair class
 */

#pragma once

#include "CHIPCryptoPAL.h"
#include <openssl/x509.h>

namespace chip {
namespace Crypto {

#if CHIP_CRYPTO_BORINGSSL
using boringssl_size_t_openssl_int = size_t;
#else
using boringssl_size_t_openssl_int = int;
#endif

enum class ECName
{
    None   = 0,
    P256v1 = 1,
};

/**
 * @brief Collect and print SSL-related error information
 **/
void CryptoPAL_logSSLError();

/**
 * @brief Get the NID of an elliptic curve by name
 **/
int CryptoPAL_nidForCurve(ECName name);

/**
 * @brief Get the elliptic curve name by key type
 **/
ECName CryptoPAL_MapECName(SupportedECPKeyTypes keyType);

/**
 * @brief Get public key via EC_KEY
 **/
CHIP_ERROR P256PublicKeyFromECKey(EC_KEY * ec_key, P256PublicKey & pubkey);

} // namespace Crypto
} // namespace chip
