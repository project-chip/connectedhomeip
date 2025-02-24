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

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

// DIC Specific Configurations
#ifdef DIC_ENABLE
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_NO_CRT
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_RSA_C
#define MBEDTLS_SSL_IN_CONTENT_LEN 5120
#define MBEDTLS_SSL_OUT_CONTENT_LEN 1560
#define MBEDTLS_PSA_BUILTIN_ALG_TLS12_PRF
#define MBEDTLS_MPI_MAX_SIZE 512
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CCM_C
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_PK_HAVE_CURVE_SECP256R1
#define MBEDTLS_PK_HAVE_JPAKE
#define PSA_WANT_ALG_SOME_PAKE
#define MBEDTLS_SSL_MAX_CONTENT_LEN 768

/**
 * Enable H Crypto and Entropy modules
 */
#define MBEDTLS_ECDH_C
#define MBEDTLS_ENTROPY_C

#endif // DIC_ENABLE

// Configurations necessary for ot coap cert libs
#if SL_USE_COAP_CONFIG
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CCM_C
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_PK_HAVE_CURVE_SECP256R1
#define MBEDTLS_PK_HAVE_JPAKE
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_SSL_HANDSHAKE_WITH_CERT_ENABLED
#endif // SL_USE_COAP_CONFIG

#if SL_MATTER_PROVISION_FLASH
#define MBEDTLS_SHA256_C
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_ECDSA_DETERMINISTIC
#endif // SL_MATTER_PROVISION_FLASH

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#define MBEDTLS_AES_ROM_TABLES
// #define MBEDTLS_AES_FEWER_TABLES /* Check if we want this */

#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_SHA256_SMALLER    /* Check if we want this */
#define MBEDTLS_MPI_WINDOW_SIZE 1 /**< Maximum windows size used. */

// Includes generated file for all generic defines.
#include "sl_mbedtls_config.h"
