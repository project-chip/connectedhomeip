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
 *  4. Define ENABLE_SE05X_SPAKE_VERIFIER in  src/app/server/BUILD.gn, src/controller/BUILD.gn,
 * src/protocols/secure_channel/BUILD.gn . add the following lines in BUILD.gn file -
 * import("${chip_root}/src/platform/nxp/crypto/se05x/args.gni") and if (chip_se05x_spake_verifier) { defines += [
 * "ENABLE_SE05X_SPAKE_VERIFIER=1" ]}
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
 * Enable se05x for Generate EC Key
 */
#ifndef ENABLE_SE05X_GENERATE_EC_KEY
#ifdef CONFIG_CHIP_SE05X_GENERATE_EC_KEY
#define ENABLE_SE05X_GENERATE_EC_KEY CONFIG_CHIP_SE05X_GENERATE_EC_KEY
#else
#define ENABLE_SE05X_GENERATE_EC_KEY 0
#endif
#endif // ENABLE_SE05X_GENERATE_EC_KEY

/*
 * Enable ECDSA Verify using se05x
 */
#ifndef ENABLE_SE05X_ECDSA_VERIFY
#ifdef CONFIG_CHIP_SE05X_ECDSA_VERIFY
#define ENABLE_SE05X_ECDSA_VERIFY CONFIG_CHIP_SE05X_ECDSA_VERIFY
#else
#define ENABLE_SE05X_ECDSA_VERIFY 0
#endif
#endif // ENABLE_SE05X_ECDSA_VERIFY

/*
 * Enable se05x for PBKDF SHA256
 * Note: Not supported for SE052F.
 */
#ifndef ENABLE_SE05X_PBKDF2_SHA256
#ifdef CONFIG_CHIP_SE05X_PBKDF2_SHA256
#define ENABLE_SE05X_PBKDF2_SHA256 CONFIG_CHIP_SE05X_PBKDF2_SHA256
#else
#define ENABLE_SE05X_PBKDF2_SHA256 0
#endif
#endif // ENABLE_SE05X_PBKDF2_SHA256

/*
 * Enable se05x for HKDF SHA256
 * Note: Not supported for SE052F.
 */
#ifndef ENABLE_SE05X_HKDF_SHA256
#ifdef CONFIG_CHIP_SE05X_HKDF_SHA256
#define ENABLE_SE05X_HKDF_SHA256 CONFIG_CHIP_SE05X_HKDF_SHA256
#else
#define ENABLE_SE05X_HKDF_SHA256 0
#endif
#endif // ENABLE_SE05X_HKDF_SHA256

/*
 * Enable se05x for HMAC SHA256
 */
#ifndef ENABLE_SE05X_HMAC_SHA256
#ifdef CONFIG_CHIP_SE05X_HMAC_SHA256
#define ENABLE_SE05X_HMAC_SHA256 CONFIG_CHIP_SE05X_HMAC_SHA256
#else
#define ENABLE_SE05X_HMAC_SHA256 0
#endif
#endif // ENABLE_SE05X_HMAC_SHA256

/*
 * Enable se05x for DA
 * Ensure to run the provision example (one time) `third_party/simw-top-mini/repo/demos/se05x_dev_attest_key_prov/` to provision the
   device attestation key at id - 0x7D300000 and
   device attestation certificate at id - 0x7D300001.
 */
#ifndef ENABLE_SE05X_DEVICE_ATTESTATION
#ifdef CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
#define ENABLE_SE05X_DEVICE_ATTESTATION CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
#else
#define ENABLE_SE05X_DEVICE_ATTESTATION 0
#endif
#endif // ENABLE_SE05X_DEVICE_ATTESTATION
