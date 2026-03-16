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
 *  4. Modify build files -
 *    4a - Define ENABLE_SE05X_SPAKE_VERIFIER in  src/app/server/BUILD.gn,
 *    4b - Define ENABLE_SE05X_SPAKE_VERIFIER in  src/controller/BUILD.gn,
 *    4c - Define ENABLE_SE05X_SPAKE_VERIFIER in  src/protocols/secure_channel/BUILD.gn.
 */

#ifndef ENABLE_SE05X_SPAKE_VERIFIER
#ifdef CONFIG_CHIP_SE05X_SPAKE_VERIFIER
#define ENABLE_SE05X_SPAKE_VERIFIER CONFIG_CHIP_SE05X_SPAKE_VERIFIER
#else
#define ENABLE_SE05X_SPAKE_VERIFIER 0
#endif
#endif // ENABLE_SE05X_SPAKE_VERIFIER

/*
 * Enable se05x for SPAKE PROVER
 */
#ifndef ENABLE_SE05X_SPAKE_PROVER
#ifdef CONFIG_CHIP_SE05X_SPAKE_PROVER
#define ENABLE_SE05X_SPAKE_PROVER CONFIG_CHIP_SE05X_SPAKE_PROVER
#else
#define ENABLE_SE05X_SPAKE_PROVER 0
#endif
#endif // ENABLE_SE05X_SPAKE_PROVER

/*
 * Enable se05x for random number generation
 */
#ifndef ENABLE_SE05X_RND_GEN
#ifdef CONFIG_CHIP_SE05X_RND_GEN
#define ENABLE_SE05X_RND_GEN CONFIG_CHIP_SE05X_RND_GEN
#else
#define ENABLE_SE05X_RND_GEN 0
#endif
#endif // ENABLE_SE05X_RND_GEN

/*
 * Enable se05x for DA
 * Ensure to run the provision example (one time) `third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/` to provision the
   device attestation key at id - 0x7FFF3007 and
   device attestation certificate at id - 0x7FFF3003.
 */
#ifndef ENABLE_SE05X_DEVICE_ATTESTATION
#ifdef CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
#define ENABLE_SE05X_DEVICE_ATTESTATION CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
#else
#define ENABLE_SE05X_DEVICE_ATTESTATION 0
#endif
#endif // ENABLE_SE05X_DEVICE_ATTESTATION

/*
 * Enable se05x for SPAKE VERIFIER by using Trust provisioned W0 and L values
 */
#ifndef ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
#ifdef CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
#define ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
#else
#define ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES 0
#endif
#endif // ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES

/*
 * SE05X TRUST PROVISIONED SPAKE VERIFIER SET NUMBER
 */
#ifndef SE05X_SPAKE_VERIFIER_TP_SET_NO
#ifdef CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO
#define SE05X_SPAKE_VERIFIER_TP_SET_NO CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_SET_NO
#else
#define SE05X_SPAKE_VERIFIER_TP_SET_NO 1
#endif
#endif // SE05X_SPAKE_VERIFIER_TP_SET_NO

/*
 * SE05X TRUST PROVISIONED SPAKE VERIFIER ITERATION COUNT
 */
#ifndef SE05X_SPAKE_VERIFIER_TP_ITER_CNT
#ifdef CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITER_CNT
#define SE05X_SPAKE_VERIFIER_TP_ITER_CNT CONFIG_CHIP_SE05X_SPAKE_VERIFIER_TP_ITER_CNT
#else
#define SE05X_SPAKE_VERIFIER_TP_ITER_CNT 1000
#endif
#endif // SE05X_SPAKE_VERIFIER_TP_ITER_CNT
