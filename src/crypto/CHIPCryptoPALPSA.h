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
 * @def CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END
 *
 * @brief
 *   End of the PSA key identifier range used by Matter.
 *
 * This setting establishes the maximum limit for the key range specific to Matter, in order to
 * prevent any overlap with other firmware components that also employ the PSA crypto API.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END
#define CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END 0x3FFFF
#endif // CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END

static_assert(PSA_KEY_ID_USER_MIN <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE && CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END <= PSA_KEY_ID_USER_MAX,
              "Matter specific PSA key range doesn't fit within PSA allowed range");

// Each ICD client requires storing two keys- AES and HMAC
static constexpr uint32_t kMaxICDClientKeys = 2 * CHIP_CONFIG_CRYPTO_PSA_ICD_MAX_CLIENTS;

static_assert(kMaxICDClientKeys >= CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS,
              "Number of allocated ICD key slots is lower than maximum number of supported ICD clients");

/**
 * @brief Defines subranges of the PSA key identifier space used by Matter.
 */
enum class KeyIdBase : psa_key_id_t
{
    Minimum          = CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE,
    Operational      = Minimum, ///< Base of the PSA key ID range for Node Operational Certificate private keys
    DACPrivKey       = Operational + kMaxValidFabricIndex + 1,
    ICDKeyRangeStart = DACPrivKey + 1,
    Maximum          = ICDKeyRangeStart + kMaxICDClientKeys,
};

static_assert(to_underlying(KeyIdBase::Minimum) >= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE &&
                  to_underlying(KeyIdBase::Maximum) <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END,
              "PSA key ID base out of allowed range");

/**
 * @brief Finds first free persistent Key slot ID within range.
 *
 * @param[out] keyId Key ID handler to which free ID will be set.
 * @param[in]  start Starting ID in search range.
 * @param[in]  range Search range.
 *
 * @retval CHIP_NO_ERROR               On success.
 * @retval CHIP_ERROR_INTERNAL         On PSA crypto API error.
 * @retval CHIP_ERROR_NOT_FOUND        On no free Key ID within range.
 * @retval CHIP_ERROR_INVALID_ARGUMENT On search arguments out of PSA allowed range.
 */
CHIP_ERROR FindFreeKeySlotInRange(psa_key_id_t & keyId, psa_key_id_t start, uint32_t range);

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
    CHIP_ERROR Init(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info);

    /**
     * @brief Initializes the key derivation operation.
     */
    CHIP_ERROR Init(const HkdfKeyHandle & hkdfKey, const ByteSpan & salt, const ByteSpan & info);

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
    CHIP_ERROR InitOperation(psa_key_id_t hkdfKey, const ByteSpan & salt, const ByteSpan & info);

    psa_key_id_t mSecretKeyId                 = PSA_KEY_ID_NULL;
    psa_key_derivation_operation_t mOperation = PSA_KEY_DERIVATION_OPERATION_INIT;
};

/**
 * @brief Log PSA status code if it indicates an error.
 */
void LogPsaError(psa_status_t status);

} // namespace Crypto
} // namespace chip
