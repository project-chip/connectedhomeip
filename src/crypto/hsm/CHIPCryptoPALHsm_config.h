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
 *      Header that exposes the options to enable HSM for required crypto operations.
 */

#pragma once

/*
 * Enable HSM for SPAKE VERIFIER
 */
#define ENABLE_HSM_SPAKE_VERIFIER 0

/*
 * Enable HSM for SPAKE PROVER
 */
#define ENABLE_HSM_SPAKE_PROVER 0

/*
 * Enable HSM for Generate EC Key
 */
#define ENABLE_HSM_GENERATE_EC_KEY 1

/*
 * Enable HSM for PBKDF SHA256
 */
#define ENABLE_HSM_PBKDF2_SHA256 0

/*
 * Enable HSM for HKDF SHA256
 */
#define ENABLE_HSM_HKDF_SHA256 0

/*
 * Enable HSM for HMAC SHA256
 */
#define ENABLE_HSM_HMAC_SHA256 0

#if ((CHIP_CRYPTO_HSM) && ((ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER)))
#define ENABLE_HSM_SPAKE
#endif

#if ((CHIP_CRYPTO_HSM) && (ENABLE_HSM_GENERATE_EC_KEY))
#define ENABLE_HSM_EC_KEY
//#define ENABLE_HSM_ECDSA_VERIFY
//#define ENABLE_HSM_DEVICE_ATTESTATION
#endif

#if ((CHIP_CRYPTO_HSM) && (ENABLE_HSM_PBKDF2_SHA256))
#define ENABLE_HSM_PBKDF2
#endif

#if ((CHIP_CRYPTO_HSM) && (ENABLE_HSM_HKDF_SHA256))
#define ENABLE_HSM_HKDF
#endif

#if ((CHIP_CRYPTO_HSM) && (ENABLE_HSM_HMAC_SHA256))
#define ENABLE_HSM_HMAC
#endif
