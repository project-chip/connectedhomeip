/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_VERSION_C
#define MBEDTLS_VERSION_FEATURES

#define MBEDTLS_HKDF_C
#define MBEDTLS_PKCS5_C

// DIC Specific Configurations
#ifdef DIC_ENABLE

#define MBEDTLS_MPI_MAX_SIZE 512

#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_WITH_PADDING
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_C
#define MBEDTLS_RSA_NO_CRT
#define MBEDTLS_SHA1_C
#define MBEDTLS_SSL_IN_CONTENT_LEN 5120
#define MBEDTLS_SSL_MAX_CONTENT_LEN 5120
#define MBEDTLS_SSL_OUT_CONTENT_LEN 1560

#endif // DIC_ENABLE

#ifdef SL_MBEDTLS_USE_TINYCRYPT
#define TINYCRYPT_PRIMITIVES
#define OPTIMIZE_TINYCRYPT_ASM

#define MBEDTLS_FREERTOS

#if (!(DIC_ENABLE) || !defined(DIC_ENABLE))
/**< Maximum windows size used. */
#define MBEDTLS_MPI_WINDOW_SIZE 1

/**< Maximum number of bytes for usable MPIs. */
#define MBEDTLS_MPI_MAX_SIZE 32
/**< Maxium fragment length in bytes */
#define MBEDTLS_SSL_MAX_CONTENT_LEN 768
#endif // !(DIC_ENABLE)

#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_CFB
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDH_LEGACY_CONTEXT
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 0
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ECP_WINDOW_SIZE 2
#define MBEDTLS_ERROR_STRERROR_DUMMY
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_PK_HAVE_ECC_KEYS
#define MBEDTLS_SHA256_SMALLER
#define MBEDTLS_SHA512_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_TRNG_C

#if defined(MBEDTLS_ECP_ALT) && !defined(MBEDTLS_ECP_RESTARTABLE)
typedef void mbedtls_ecp_restart_ctx;
#endif

#endif // SL_MBEDTLS_USE_TINYCRYPT

// SLC GENERATED
#include "sl_mbedtls_config.h"
