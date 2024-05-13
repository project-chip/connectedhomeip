/**
 ******************************************************************************
 * @file    matter_config.h
 * @author  MCD Application Team
 * @brief   config file for mbedtls
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#pragma once

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// #if CHIP_HAVE_CONFIG_H
// #include <crypto/CryptoBuildConfig.h>
// #endif // CHIP_HAVE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enable H Crypto and Entropy modules
 */
#define MBEDTLS_AES_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ECDH_LEGACY_CONTEXT
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ENTROPY_FORCE_SHA256
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_ERROR_STRERROR_DUMMY
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HKDF_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_MD_C

#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

#define MBEDTLS_OID_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PEM_WRITE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#if CHIP_CRYPTO_MBEDTLS
#define MBEDTLS_PKCS5_C
#endif
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_SHA256_SMALLER
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C

#if CHIP_CRYPTO_PLATFORM
#define MBEDTLS_USE_PSA_CRYPTO
#endif

#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_X509_CRL_PARSE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_X509_USE_C

#define MBEDTLS_MPI_WINDOW_SIZE 1       /**< Maximum windows size used. */
#define MBEDTLS_MPI_MAX_SIZE 32         /**< Maximum number of bytes for usable MPIs. */
#define MBEDTLS_ECP_MAX_BITS 256        /**< Maximum bit size of groups */
#define MBEDTLS_ECP_WINDOW_SIZE 2       /**< Maximum window size used */
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 0 /**< Enable fixed-point speed-up */
#define MBEDTLS_ENTROPY_MAX_SOURCES 2   /**< Maximum number of sources supported */

#if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_SSL_MAX_CONTENT_LEN 900 /**< Maxium fragment length in bytes */
#else
#define MBEDTLS_SSL_MAX_CONTENT_LEN 768 /**< Maxium fragment length in bytes */
#endif

#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

#define MBEDTLS_CIPHER_MODE_WITH_PADDING

#define MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_C
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_PKCS5_C

#define MBEDTLS_ERROR_C 1

// #include "check_crypto_config.h"
#include "mbedtls/check_config.h"
#include "mbedtls/config_psa.h"

#ifdef __cplusplus
}
#endif
