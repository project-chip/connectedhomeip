/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header that exposes the platform agnostic CHIP crypto primitives
 */

#pragma once

#include "CHIPCryptoPALHsm_se05x_config.h"
#include <lib/core/DataModelTypes.h>

#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))
typedef struct hsm_pake_context_s
{
    uint8_t spake_context[32];
    size_t spake_context_len;
    uint8_t spake_objId;
} hsm_pake_context_t;
#endif //#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))

namespace chip {
namespace Crypto {

#define NXP_CRYPTO_KEY_MAGIC                                                                                                       \
    {                                                                                                                              \
        0xA5, 0xA6, 0xB5, 0xB6, 0xA5, 0xA6, 0xB5, 0xB6                                                                             \
    }

static const uint8_t se05x_magic_no[] = NXP_CRYPTO_KEY_MAGIC;

#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))
/* Spake HSM class */

class Spake2pHSM_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    ~Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    CHIP_ERROR Init(const uint8_t * context, size_t context_len) override;

#if ENABLE_SE05X_SPAKE_VERIFIER
    CHIP_ERROR BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                             size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                             size_t Lin_len) override;
#endif

#if ENABLE_SE05X_SPAKE_PROVER
    CHIP_ERROR BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                           size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                           size_t w1in_len) override;
#endif

    CHIP_ERROR ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len) override;

    CHIP_ERROR KeyConfirm(const uint8_t * in, size_t in_len) override;

    hsm_pake_context_t hsm_pake_context;
};

#endif //#if ((ENABLE_SE05X_SPAKE_VERIFIER) || (ENABLE_SE05X_SPAKE_PROVER))

} // namespace Crypto
} // namespace chip
