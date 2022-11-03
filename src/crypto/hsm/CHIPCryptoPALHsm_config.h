/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
