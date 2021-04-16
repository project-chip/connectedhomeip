/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Header that exposes the platform agnostic CHIP crypto primitives
 */

#ifndef _CHIP_CRYPTO_PAL_HSM_H_
#define _CHIP_CRYPTO_PAL_HSM_H_

#include "CHIPCryptoPALHsm_config.h"

#if CHIP_CRYPTO_HSM_NXP
#include <fsl_sss_se05x_apis.h>
#endif

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

typedef struct hsm_pake_context_s
{
    uint8_t spake_context[32];
    size_t spake_context_len;
#if CHIP_CRYPTO_HSM_NXP
    SE05x_CryptoObjectID_t spake_objId;
#endif
} hsm_pake_context_t;

namespace chip {
namespace Crypto {

class Spake2pHSM_P256_SHA256_HKDF_HMAC : public Spake2p_P256_SHA256_HKDF_HMAC
{
public:
    Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    ~Spake2pHSM_P256_SHA256_HKDF_HMAC() {}

    CHIP_ERROR Init(const uint8_t * context, size_t context_len);

#if ENABLE_HSM_SPAKE_VERIFIER
    CHIP_ERROR BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                             size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin, size_t Lin_len);
#endif

#if ENABLE_HSM_SPAKE_PROVER
    CHIP_ERROR BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                           size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in, size_t w1in_len);
#endif

    CHIP_ERROR ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len);

    CHIP_ERROR ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len);

    CHIP_ERROR KeyConfirm(const uint8_t * in, size_t in_len);

    hsm_pake_context_t hsm_pake_context;
};

} // namespace Crypto
} // namespace chip

#endif //#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

#endif //#ifndef _CHIP_CRYPTO_PAL_HSM_H_
