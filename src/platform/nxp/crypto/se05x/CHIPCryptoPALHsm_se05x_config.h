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
