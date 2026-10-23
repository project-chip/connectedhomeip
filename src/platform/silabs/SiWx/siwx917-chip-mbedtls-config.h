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

// includes exclusive config options
// common options and non-exclusive config options
// are defined in the sli_mbedtls_config_autogen.h file
// see sli_mbedtls_config_autogen.h for more details

#pragma once

// ChipCryptoPAL tinycrypt config
#ifdef SL_MBEDTLS_USE_TINYCRYPT

// Tinycrypt configurations
#define TINYCRYPT_PRIMITIVES
#define OPTIMIZE_TINYCRYPT_ASM

#define MBEDTLS_FREERTOS

#define MBEDTLS_MPI_WINDOW_SIZE 1

// Added for codesize optimizations
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_SHA256_SMALLER

// Added from autogen file for tinycrypt
#define MBEDTLS_ECDH_C
#define MBEDTLS_HKDF_C

#if (!(SL_MATTER_ENABLE_AWS) || !defined(SL_MATTER_ENABLE_AWS))
#define MBEDTLS_SSL_MAX_CONTENT_LEN 768
#endif // !(SL_MATTER_ENABLE_AWS)

#endif // SL_MBEDTLS_USE_TINYCRYPT

// MATTER AWS Specific Configurations
#ifdef SL_MATTER_ENABLE_AWS

#ifndef MBEDTLS_MPI_MAX_SIZE
#define MBEDTLS_MPI_MAX_SIZE 512
#endif // MBEDTLS_MPI_MAX_SIZE

#ifndef MBEDTLS_SSL_IN_CONTENT_LEN
#define MBEDTLS_SSL_IN_CONTENT_LEN 5120
#endif // MBEDTLS_SSL_IN_CONTENT_LEN

#ifndef MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_MAX_CONTENT_LEN 5120
#endif // MBEDTLS_SSL_MAX_CONTENT_LEN

#ifndef MBEDTLS_SSL_OUT_CONTENT_LEN
#define MBEDTLS_SSL_OUT_CONTENT_LEN 1560
#endif // MBEDTLS_SSL_OUT_CONTENT_LEN

// TODO: Optimize these configurations for SL_MATTER_ENABLE_AWS codebase.
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_GCM_C
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE

// TODO: Remove `MBEDTLS_DEBUG_C` for SL_MATTER_ENABLE_AWS codebase.
#define MBEDTLS_DEBUG_C

#endif // SL_MATTER_ENABLE_AWS

#if SL_MATTER_PROVISION_FLASH
#define MBEDTLS_SHA256_C
#define MBEDTLS_HMAC_DRBG_C
#if !defined(SL_MATTER_ENABLE_AWS)
#define MBEDTLS_ECDSA_DETERMINISTIC
#endif // !(SL_MATTER_ENABLE_AWS)
#endif // SL_MATTER_PROVISION_FLASH

// SLC GENERATED
#include "sl_mbedtls_config.h"
