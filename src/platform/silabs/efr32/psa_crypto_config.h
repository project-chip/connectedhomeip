/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

// ADD USER PSA CRYPTO CONFIG HERE
#define SL_PSA_KEY_USER_SLOT_COUNT (4)

#define SL_PSA_ITS_USER_MAX_FILES (128)

#include "em_device.h"
#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)
// Use accelerated HMAC when we have it
#define MBEDTLS_PSA_ACCEL_ALG_HMAC
// Use accelerated ECDSA/ECDH when we have it
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_PUBLIC_KEY
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256
#define MBEDTLS_PSA_ACCEL_ALG_ECDH
#define MBEDTLS_PSA_ACCEL_ALG_ECDSA
#else
// Devices without SEMAILBOX or CRYPTOACC don't have HMAC top-level accelerated
#define MBEDTLS_PSA_BUILTIN_ALG_HMAC 1
// Devices without SEMAILBOX or CRYPTOACC don't have fully implemented ECDSA/ECDH
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_KEY_PAIR 1
#define MBEDTLS_PSA_BUILTIN_ECC_SECP_R1_256 1
#define MBEDTLS_PSA_BUILTIN_ALG_ECDH 1
#define MBEDTLS_PSA_BUILTIN_ALG_ECDSA 1
#endif
// end of user configuration section >>>

// AUTO GENERATED PSA CONFIG SECTION BELOW. **KEEP AS IS FUTURE GSDK UPDATE**
#define PSA_WANT_KEY_TYPE_AES
#define PSA_WANT_ALG_CMAC
#define PSA_WANT_ALG_SHA_256
#define PSA_WANT_ALG_CCM
#define PSA_WANT_ALG_ECB_NO_PADDING
#define PSA_WANT_ALG_ECDSA
#define PSA_WANT_ALG_ECDH
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY
#define PSA_WANT_ECC_SECP_R1_256
#define PSA_WANT_ALG_HKDF
#define PSA_WANT_ALG_HMAC
#define PSA_WANT_KEY_TYPE_HMAC
#ifdef DIC_ENABLE
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY
#endif // DIC_ENABLE
#define MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
#define MBEDTLS_PSA_ACCEL_ALG_SHA_1
#define MBEDTLS_PSA_ACCEL_ALG_SHA_224
#define MBEDTLS_PSA_ACCEL_ALG_SHA_256
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_AES
#define MBEDTLS_PSA_ACCEL_ALG_ECB_NO_PADDING
#define MBEDTLS_PSA_ACCEL_ALG_CBC_NO_PADDING
#define MBEDTLS_PSA_ACCEL_ALG_CBC_PKCS7
#define MBEDTLS_PSA_ACCEL_ALG_CTR
#define MBEDTLS_PSA_ACCEL_ALG_CFB
#define MBEDTLS_PSA_ACCEL_ALG_OFB
#define MBEDTLS_PSA_ACCEL_ALG_GCM
#define MBEDTLS_PSA_ACCEL_ALG_CCM
#define MBEDTLS_PSA_ACCEL_ALG_CMAC

#define MBEDTLS_PSA_KEY_SLOT_COUNT (15 + 1 + SL_PSA_KEY_USER_SLOT_COUNT)
#define SL_PSA_ITS_MAX_FILES (1 + SL_PSA_ITS_USER_MAX_FILES)
