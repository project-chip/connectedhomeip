/*
 *  Copyright (c) 2020, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "em_device.h"
#include "em_se.h"

/**
 * Enable FreeRTOS threading support
 */
#define MBEDTLS_FREERTOS
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT

/**
 * Enable H Crypto and Entropy modules
 */
#define MBEDTLS_AES_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_CIPHER_MODE_CTR
#define MBEDTLS_TRNG_C

/**
 * \def MBEDTLS_AES_ALT
 *
 * Enable hardware acceleration for the AES block cipher
 *
 * Module:  sl_crypto/src/crypto_aes.c for devices with CRYPTO
 *          sl_crypto/src/aes_aes.c for devices with AES
 *
 * See MBEDTLS_AES_C for more information.
 */
#define MBEDTLS_AES_ALT

#if defined(EFR32MG12)
#define MBEDTLS_SHA1_C
#define TRNG_PRESENT

#if defined(CRYPTO_COUNT) && (CRYPTO_COUNT > 0)
/**
 * \def MBEDTLS_ECP_INTERNAL_ALT
 * \def ECP_SHORTWEIERSTRASS
 * \def MBEDTLS_ECP_ADD_MIXED_ALT
 * \def MBEDTLS_ECP_DOUBLE_JAC_ALT
 * \def MBEDTLS_ECP_NORMALIZE_JAC_MANY_ALT
 * \def MBEDTLS_ECP_NORMALIZE_JAC_ALT
 *
 * Enable hardware acceleration for the elliptic curve over GF(p) library.
 *
 * Module:  sl_crypto/src/crypto_ecp.c
 * Caller:  library/ecp.c
 *
 * Requires: MBEDTLS_BIGNUM_C, MBEDTLS_ECP_C and at least one
 * MBEDTLS_ECP_DP_XXX_ENABLED and (CRYPTO_COUNT > 0)
 */
#define MBEDTLS_ECP_INTERNAL_ALT
#define ECP_SHORTWEIERSTRASS
#define MBEDTLS_ECP_ADD_MIXED_ALT
#define MBEDTLS_ECP_DOUBLE_JAC_ALT
#define MBEDTLS_ECP_NORMALIZE_JAC_MANY_ALT
#define MBEDTLS_ECP_NORMALIZE_JAC_ALT
#define MBEDTLS_ECP_RANDOMIZE_JAC_ALT

/**
 * \def MBEDTLS_SHA1_ALT
 *
 * Enable hardware acceleration for the SHA-224 and SHA-256 cryptographic
 * hash algorithms.
 * Module: sl_crypto/src/crypto_sha.c
 * Caller: library/mbedtls_md.c
 *         library/ssl_cli.c
 *         library/ssl_srv.c
 *         library/ssl_tls.c
 *         library/x509write_crt.c
 *
 * Requires: MBEDTLS_SHA1_C and (CRYPTO_COUNT > 0)
 * See MBEDTLS_SHA1_C for more information.
 */
#define MBEDTLS_SHA1_ALT

/**
 * \def MBEDTLS_SHA256_ALT
 *
 * Enable hardware acceleration for the SHA-224 and SHA-256 cryptographic
 * hash algorithms.
 *
 * Module:  sl_crypto/src/crypto_sha.c
 * Caller:  library/entropy.c
 *          library/mbedtls_md.c
 *          library/ssl_cli.c
 *          library/ssl_srv.c
 *          library/ssl_tls.c
 *
 * Requires: MBEDTLS_SHA256_C and (CRYPTO_COUNT > 0)
 * See MBEDTLS_SHA256_C for more information.
 */
#define MBEDTLS_SHA256_ALT
#endif

#elif defined(EFR32MG21)
#define MBEDTLS_SHA1_ALT
#define MBEDTLS_SHA1_PROCESS_ALT
#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA256_PROCESS_ALT
#define MBEDTLS_SHA512_ALT
#define MBEDTLS_SHA512_PROCESS_ALT

#define MBEDTLS_CCM_ALT
#define MBEDTLS_CMAC_ALT

/* Turning on ECC acceleration is dependant on not requiring curve25519 when
 * running on EFR32xG21A devices */
#if (defined(_SILICON_LABS_SECURITY_FEATURE) && (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)) ||       \
    !defined(MBEDTLS_ECP_DP_CURVE25519_ENABLED)
#define MBEDTLS_ECDH_GEN_PUBLIC_ALT
#define MBEDTLS_ECDSA_GENKEY_ALT
#define MBEDTLS_ECDH_COMPUTE_SHARED_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT
#endif /* EFR32xG21B or curve25519 not enabled */
#endif // Familiy Selection

#if defined(MBEDTLS_ECP_ALT) && !defined(MBEDTLS_ECP_RESTARTABLE)
typedef void mbedtls_ecp_restart_ctx;
#endif

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_MD_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_SHA256_SMALLER
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_ERROR_STRERROR_DUMMY
#define MBEDTLS_HKDF_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_WRITE_C
#define MBEDTLS_PKCS5_C

#if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE || OPENTHREAD_CONFIG_COMMISSIONER_ENABLE || OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_SRV_C
#endif

#if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#endif

#ifdef MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_BASE64_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_OID_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#endif

#define MBEDTLS_MPI_WINDOW_SIZE 1       /**< Maximum windows size used. */
#define MBEDTLS_MPI_MAX_SIZE 32         /**< Maximum number of bytes for usable MPIs. */
#define MBEDTLS_ECP_MAX_BITS 256        /**< Maximum bit size of groups */
#define MBEDTLS_ECP_WINDOW_SIZE 2       /**< Maximum window size used */
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 0 /**< Enable fixed-point speed-up */
#define MBEDTLS_ENTROPY_MAX_SOURCES 1   /**< Maximum number of sources supported */

#define MBEDTLS_PLATFORM_STD_CALLOC calloc /**< Default allocator to use, can be undefined */
#define MBEDTLS_PLATFORM_STD_FREE free     /**< Default free to use, can be undefined */

#if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_SSL_MAX_CONTENT_LEN 900 /**< Maxium fragment length in bytes */
#else
#define MBEDTLS_SSL_MAX_CONTENT_LEN 768 /**< Maxium fragment length in bytes */
#endif

#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

#if defined(MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#endif
