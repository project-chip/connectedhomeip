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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */
#ifndef _CHIP_CRYPTO_PAL_HSM_SE05X_H_
#define _CHIP_CRYPTO_PAL_HSM_SE05x_H_

#include <fsl_sss_se05x_apis.h>

#if ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER))

typedef struct hsm_pake_context_s
{
    // NXP-SE
    // To store the context
    // Context is calculated as SHA256("Const String" || PBKDFParamRequest || PBKDFParamResponse). So context is always 32 bytes.
    // But test vectors can pass any context. So keeping the buffer as 128 bytes.
    uint8_t spake_context[128]; //= { 0, };
    size_t spake_context_len;   // = 0;
    SE05x_CryptoObjectID_t spake_objId;

} hsm_pake_context_t;
#endif

#endif /*_CHIP_CRYPTO_PAL_HSM_SE05x_H_*/
