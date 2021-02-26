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


/*
 * Enable HSM for SHA256
 */
#define ENABLE_HSM_HASH_SHA256 1


/*
 * Enable HSM for MultiStep SHA256 (Currently not supported)
 */
#define ENABLE_HSM_HASH_SHA256_MULTISTEP 0

/*
 * Enable HSM for HKDF-SHA256
 */
#define ENABLE_HSM_HKDF_SHA256 1

/*
 * Enable HSM for AES CCM ENCRYPT (Currently not supported)
 */
#define ENABLE_HSM_AES_CCM_ENCRYPT 0

/*
 * Enable HSM for AES CCM DECRYPT (Currently not supported)
 */
#define ENABLE_HSM_AES_CCM_DECRYPT 0

/*
 * Enable HSM for random generator
 */
#define ENABLE_HSM_RAND_GEN 1

/*
 * Enable HSM for Generate EC Key
 */
#define ENABLE_HSM_GENERATE_EC_KEY 1 //1 for ecc example

/*
 * Enable HSM for MAC
 */
#define ENABLE_HSM_MAC 1

/*
 * Enable HSM for SPAKE VERIFIER
 */
#define ENABLE_HSM_SPAKE_VERIFIER 1

/*
 * Enable HSM for SPAKE PROVER
 */
#define ENABLE_HSM_SPAKE_PROVER 1

