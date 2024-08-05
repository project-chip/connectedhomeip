/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *      Header that exposes the options to enable trustm for required crypto operations.
 */

#pragma once

/*
 * Enable trustm for Generate EC Key
 */
#define ENABLE_TRUSTM_GENERATE_EC_KEY 1

/*
 * Enable ECDSA Verify using trustm
 */
#define ENABLE_TRUSTM_ECDSA_VERIFY 1

/*
 * Enable trustm for HKDF SHA256
 */
#define ENABLE_TRUSTM_HKDF_SHA256 0

/*
 * Enable trustm for HMAC SHA256
 */
#define ENABLE_TRUSTM_HMAC_SHA256 0

/*
 * Enable trustm for DA
 */
#define ENABLE_TRUSTM_DEVICE_ATTESTATION 1

/*
 * Enable trustm for NOC key-pair generation
 */
#define ENABLE_TRUSTM_NOC_KEYGEN 0
