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

#pragma once

#include "CHIPCryptoPAL.h"
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafePointerCast.h>

#include <psa/crypto.h>

namespace chip {
namespace Crypto {

/**
 *  @def CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
 *
 *  @brief
 *    Base for PSA key identifier range used by Matter.
 *
 *  Cryptographic keys stored in the PSA Internal Trusted Storage must have
 *  a user-assigned identifer from the range PSA_KEY_ID_USER_MIN to
 *  PSA_KEY_ID_USER_MAX. This option allows to override the base used to derive
 *  key identifiers used by Matter to avoid overlapping with other firmware
 *  components that also use PSA crypto API. The default value was selected
 *  not to interfere with OpenThread's default base that is 0x20000.
 *
 *  Note that volatile keys like ephemeral keys used for ECDH have identifiers
 *  auto-assigned by the PSA backend.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
#define CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE 0x30000
#endif // CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE

static_assert(CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE >= PSA_KEY_ID_USER_MIN &&
                  CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE <= PSA_KEY_ID_USER_MAX,
              "PSA key ID base out of allowed range");

enum class KeyIdBase : psa_key_id_t
{
    // Define key ID range for Node Operational Certificate private keys
    Operational = CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
};

constexpr psa_key_id_t MakeOperationalKeyId(FabricIndex fabricIndex)
{
    return to_underlying(KeyIdBase::Operational) + static_cast<psa_key_id_t>(fabricIndex);
}

struct PSAP256KeypairContext
{
    psa_key_id_t key_id;
};

static inline PSAP256KeypairContext & toPSAContext(P256KeypairContext & context)
{
    return *SafePointerCast<PSAP256KeypairContext *>(&context);
}

static inline const PSAP256KeypairContext & toConstPSAContext(const P256KeypairContext & context)
{
    return *SafePointerCast<const PSAP256KeypairContext *>(&context);
}

} // namespace Crypto
} // namespace chip
