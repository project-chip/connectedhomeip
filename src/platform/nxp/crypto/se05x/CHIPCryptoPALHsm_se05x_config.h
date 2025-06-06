/*
 *
 *    Copyright (c) 2020, 2025 Project CHIP Authors
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
 *      Header that exposes the options to enable SE05x for required crypto operations.
 */

#pragma once

/*
 * Enable se05x for SPAKE VERIFIER
 * Prerequisites for offloading spake operations to secure element (SE051H):
 *  1. Use SE051H and ensure to update the feature file (Enable SSS_HAVE_APPLET_SE051_H in
 * third_party\simw-top-mini\repo\fsl_sss_ftr.h)
 * 	2. Include CHIPCryptoPALHsm_se05x_spake2p.cpp for build in src\platform\nxp\crypto\se05x\BUILD.gn file
 *  3. Enable spake HSM class in src\protocols\secure_channel\PASESession.h (change Crypto::Spake2p_P256_SHA256_HKDF_HMAC
 * mSpake2p; to Crypto::Spake2pHSM_P256_SHA256_HKDF_HMAC mSpake2p;). Also include the header
 * <platform/nxp/crypto/se05x/CHIPCryptoPAL_se05x.h> in PASESession.h.
 */
#define ENABLE_SE05X_SPAKE_VERIFIER 0

/*
 * Enable se05x for SPAKE PROVER
 */
#define ENABLE_SE05X_SPAKE_PROVER 0

/*
 * Enable se05x for random number generation
 */
#define ENABLE_SE05X_RND_GEN 1

/*
 * Enable se05x for Generate EC Key
 */
#define ENABLE_SE05X_GENERATE_EC_KEY 1

/*
 * Enable ECDSA Verify using se05x
 */
#define ENABLE_SE05X_ECDSA_VERIFY 1

/*
 * Enable se05x for PBKDF SHA256
 * Note: Not supported for SE052F.
 */
#define ENABLE_SE05X_PBKDF2_SHA256 0

/*
 * Enable se05x for HKDF SHA256
 * Note: Not supported for SE052F.
 */
#define ENABLE_SE05X_HKDF_SHA256 1

/*
 * Enable se05x for HMAC SHA256
 */
#define ENABLE_SE05X_HMAC_SHA256 1

/*
 * Enable se05x for DA
 * Ensure to run the provision example (one time) `third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/` to provision the
   device attestation key at id - 0x7D300000 and
   device attestation certificate at id - 0x7D300001.
 */
#define ENABLE_SE05X_DEVICE_ATTESTATION 0
