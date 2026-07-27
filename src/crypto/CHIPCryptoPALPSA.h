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
#include "PSAKeyAllocator.h"
#include <lib/support/SafePointerCast.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

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
