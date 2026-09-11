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

namespace detail {
// Aborts unless we're running against an appropriate version of OpenSSL.
// All OpenSSL 3.x versions share the same soname, and there are no explicit ABI versioning symbols
// that would prevent a binary compiled against a newer version from running against an older
// library version. Especially when OSSL_PARAMs are used, where the library silently ignores
// unknown parameters, this can result in code silently misbehaving at runtime, even when it was
// guarded by appropriate OPENSSL_VERSION_NUMBER guards at build time.
// For example, OpenSSL 3.2 adds support for the OSSL_SIGNATURE_PARAM_NONCE_TYPE parameter to
// create deterministic ECDSA signatures, but when linked against an OpenSSL 3.0 shared library at
// runtime that parameter is silently ignored and non-deterministic signatures are produced instead.
// To prevent such issues, ensure that the runtime version is not older than the version we compiled
// against (masked to major/minor version only).
void AssertOpenSSLVersion();

// CHIPCryptoPAL.h has no initialization API that we could hook into. Force a call to
// AssertOpenSSLVersion() via a static initializer with inline linkage.
// This ensures the check runs as long as any TU that includes this header is linked.
inline struct AssertOpenSSLVersionCaller
{
    AssertOpenSSLVersionCaller() { AssertOpenSSLVersion(); }
} gAssertOpenSSLVersion;
} // namespace detail
#endif

enum class ECName
{
    None   = 0,
    P256v1 = 1,
};

/**
 * @brief Collect and print SSL-related error information
 **/
void SSLErrorLog();

/**
 * @brief Get the NID of an elliptic curve by name
 **/
int GetNidForCurve(ECName name);

/**
 * @brief Get the elliptic curve name by key type
 **/
ECName MapECName(SupportedECPKeyTypes keyType);

/**
 * @brief Get public key via EC_KEY
 **/
CHIP_ERROR P256PublicKeyFromECKey(EC_KEY * ec_key, P256PublicKey & pubkey);

} // namespace Crypto
} // namespace chip
