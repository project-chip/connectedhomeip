/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 * @file
 *   Header file that contains private definitions used by PSA crypto backend.
 *
 * This file should not be included directly by the application. Instead, use
 * cryptographic primitives defined in CHIPCryptoPAL.h or SessionKeystore.h.
 */

#pragma once

#include "CHIPCryptoPAL.h"
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafePointerCast.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

/**
 * @def CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
 *
 * @brief
 *   Base of the PSA key identifier range used by Matter.
 *
 * Cryptographic keys stored in the PSA Internal Trusted Storage must have
 * a user-assigned identifer from the range PSA_KEY_ID_USER_MIN to
 * PSA_KEY_ID_USER_MAX. This option allows to override the base used to derive
 * key identifiers used by Matter to avoid overlapping with other firmware
 * components that also use PSA crypto API. The default value was selected
 * not to interfere with OpenThread's default base that is 0x20000.
 *
 * Note that volatile keys like ephemeral keys used for ECDH have identifiers
 * auto-assigned by the PSA backend.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
#define CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE 0x30000
#endif // CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE

/**
 * @brief Defines subranges of the PSA key identifier space used by Matter.
 */
enum class KeyIdBase : psa_key_id_t
{
    Minimum     = CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE,
    Operational = Minimum, ///< Base of the PSA key ID range for Node Operational Certificate private keys
    Maximum     = Operational + kMaxValidFabricIndex,
};

static_assert(to_underlying(KeyIdBase::Minimum) >= PSA_KEY_ID_USER_MIN && to_underlying(KeyIdBase::Maximum) <= PSA_KEY_ID_USER_MAX,
              "PSA key ID base out of allowed range");

/**
 * @brief Calculates PSA key ID for Node Operational Certificate private key for the given fabric.
 */
constexpr psa_key_id_t MakeOperationalKeyId(FabricIndex fabricIndex)
{
    return to_underlying(KeyIdBase::Operational) + static_cast<psa_key_id_t>(fabricIndex);
}

/**
 * @brief Concrete P256 keypair context used by PSA crypto backend.
 */
struct PsaP256KeypairContext
{
    psa_key_id_t key_id;
};

inline PsaP256KeypairContext & ToPsaContext(P256KeypairContext & context)
{
    return *SafePointerCast<PsaP256KeypairContext *>(&context);
}

inline const PsaP256KeypairContext & ToConstPsaContext(const P256KeypairContext & context)
{
    return *SafePointerCast<const PsaP256KeypairContext *>(&context);
}

/**
 * @brief Wrapper for PSA key derivation API.
 */
class PsaKdf
{
public:
    ~PsaKdf()
    {
        psa_key_derivation_abort(&mOperation);
        psa_destroy_key(mSecretKeyId);
    }

    /**
     * @brief Initializes the key derivation operation.
     */
    CHIP_ERROR Init(psa_algorithm_t algorithm, const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info);

    /**
     * @brief Derives raw key material from the operation.
     *
     * This method together with @p DeriveKeys can be called multiple times to
     * derive several keys.
     *
     * @param[out] output Span that provides location and length for the derived key material.
     *
     * @retval CHIP_NO_ERROR       On success.
     * @retval CHIP_ERROR_INTERNAL On PSA crypto API error.
     */
    CHIP_ERROR DeriveBytes(const MutableByteSpan & output);

    /**
     * @brief Derives a key from the operation.
     *
     * This method together with @p DeriveBytes can be called multiple times to
     * derive several keys.
     *
     * @param[in] attributes Attributes of the derived key.
     * @param[out] keyId     PSA key ID of the derived key.
     *
     * @retval CHIP_NO_ERROR       On success.
     * @retval CHIP_ERROR_INTERNAL On PSA crypto API error.
     */
    CHIP_ERROR DeriveKey(const psa_key_attributes_t & attributes, psa_key_id_t & keyId);

private:
    psa_key_id_t mSecretKeyId                 = 0;
    psa_key_derivation_operation_t mOperation = PSA_KEY_DERIVATION_OPERATION_INIT;
};

} // namespace Crypto
} // namespace chip
