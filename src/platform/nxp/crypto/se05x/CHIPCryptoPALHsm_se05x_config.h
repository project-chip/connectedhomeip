/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header that exposes the options to enable SE05x for required crypto operations.
 */

#pragma once

/*
 * Enable se05x for SPAKE VERIFIER
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
 * Enable Key Import for se05x
 */
#define ENABLE_SE05X_KEY_IMPORT 0

/*
 * Enable se05x for PBKDF SHA256
 */
#define ENABLE_SE05X_PBKDF2_SHA256 0

/*
 * Enable se05x for HKDF SHA256
 */
#define ENABLE_SE05X_HKDF_SHA256 1

/*
 * Enable se05x for HMAC SHA256
 */
#define ENABLE_SE05X_HMAC_SHA256 1

/*
 * Enable se05x for DA
 */
#define ENABLE_SE05X_DEVICE_ATTESTATION 0
