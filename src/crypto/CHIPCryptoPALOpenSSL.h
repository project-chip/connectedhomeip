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
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

#include <memory>

namespace chip {
namespace Crypto {

#if CHIP_CRYPTO_BORINGSSL
using boringssl_size_t_openssl_int = size_t;
#else
using boringssl_size_t_openssl_int = int;
#endif

/**
 * @brief Deleter for an owned OpenSSL object. Free is a template parameter, not a member, so the
 *        deleter is empty and the handle stays pointer-sized.
 **/
template <typename T, void (*Free)(T *)>
struct OpenSSLDelete
{
    void operator()(T * obj) const { Free(obj); }
};

/**
 * @brief Owning handle for a heap-allocated OpenSSL object.
 *
 * Free is part of the type rather than derived from T because several types have both a plain and a
 * zeroizing variant (EC_POINT_free / EC_POINT_clear_free, BN_free / BN_clear_free) and the right one
 * depends on whether the object held secret material. Declare an alias per (type, free) pair used.
 *
 * Only a `void (T *)` cleanup can be named. Macros such as sk_X509_free and two-argument forms such
 * as sk_<TYPE>_pop_free cannot; do not adapt them with a local wrapper, since one with internal
 * linkage yields a distinct specialization per translation unit.
 *
 * The handle always frees what it holds - the base is private and release() is not re-exported, so
 * ownership cannot be handed out. Only pass get() to functions that will not free the object
 * themselves. EVP_PKEY_set1_EC_KEY is fine; EVP_PKEY_assign_EC_KEY and the *_set0_* family take
 * ownership and would leave it freed twice.
 **/
template <typename T, void (*Free)(T *)>
class ScopedOpenSSLObject : private std::unique_ptr<T, OpenSSLDelete<T, Free>>
{
    using Base = std::unique_ptr<T, OpenSSLDelete<T, Free>>;

public:
    using Base::Base;
    using Base::get;
    using Base::reset;
    using Base::operator bool;
};

using ScopedEcGroup    = ScopedOpenSSLObject<EC_GROUP, EC_GROUP_free>;
using ScopedEcKey      = ScopedOpenSSLObject<EC_KEY, EC_KEY_free>;
using ScopedEcPoint    = ScopedOpenSSLObject<EC_POINT, EC_POINT_free>;
using ScopedEvpPkey    = ScopedOpenSSLObject<EVP_PKEY, EVP_PKEY_free>;
using ScopedEvpPkeyCtx = ScopedOpenSSLObject<EVP_PKEY_CTX, EVP_PKEY_CTX_free>;

// A handle must stay pointer-sized: the empty deleter is expected to be folded away, and nothing may
// add state to the handle itself.
static_assert(sizeof(ScopedEcGroup) == sizeof(EC_GROUP *), "ScopedEcGroup must be pointer-sized");
static_assert(sizeof(ScopedEcKey) == sizeof(EC_KEY *), "ScopedEcKey must be pointer-sized");
static_assert(sizeof(ScopedEcPoint) == sizeof(EC_POINT *), "ScopedEcPoint must be pointer-sized");
static_assert(sizeof(ScopedEvpPkey) == sizeof(EVP_PKEY *), "ScopedEvpPkey must be pointer-sized");
static_assert(sizeof(ScopedEvpPkeyCtx) == sizeof(EVP_PKEY_CTX *), "ScopedEvpPkeyCtx must be pointer-sized");

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
