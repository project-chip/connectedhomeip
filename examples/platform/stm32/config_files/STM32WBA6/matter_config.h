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

#include "stm32wbaxx.h"
#include "app_conf.h"

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
#define MBEDTLS_GCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ECDH_LEGACY_CONTEXT
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_DETERMINISTIC
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
//#define MBEDTLS_ECP_NIST_OPTIM
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
#define MBEDTLS_SSL_IN_CONTENT_LEN 900 /**< Maximum fragment length in bytes */
#define MBEDTLS_SSL_OUT_CONTENT_LEN 900 /*new mbededtls include a IN and OUT param*/
#else
#define MBEDTLS_SSL_IN_CONTENT_LEN 768 /**< Maximum fragment length in bytes */
#define MBEDTLS_SSL_OUT_CONTENT_LEN 768 /*new mbededtls include a IN and OUT param*/
#endif


#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

#define MBEDTLS_CIPHER_MODE_WITH_PADDING

#define MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_C
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define  MBEDTLS_PKCS5_C

#define MBEDTLS_ERROR_C 1
/**
 * \def MBEDTLS_PK_PARSE_EC_EXTENDED
 *
 * Enhance support for reading EC keys using variants of SEC1 not allowed by
 * RFC 5915 and RFC 5480.
 *
 * Currently this means parsing the SpecifiedECDomain choice of EC
 * parameters (only known groups are supported, not arbitrary domains, to
 * avoid validation issues).
 *
 * Disable if you only need to support RFC 5915 + 5480 key formats.
 */
#define MBEDTLS_PK_PARSE_EC_EXTENDED

/**
  * @brief MBEDTLS_HAL_AES_ALT Enables ST AES alternative module to replace mbed
  *        TLS AES module by ST AES alternative implementation based on STM32
  *        AES hardware accelerator.
  *
  *        Uncomment a macro to enable ST AES hardware alternative module.
  *        Requires: MBEDTLS_AES_C, MBEDTLS_AES_ALT.
  */

/**
 * \def MBEDTLS_PSA_KEY_STORE_DYNAMIC
 *
 * Dynamically resize the PSA key store to accommodate any number of
 * volatile keys (until the heap memory is exhausted).
 *
 * If this option is disabled, the key store has a fixed size
 * #MBEDTLS_PSA_KEY_SLOT_COUNT for volatile keys and loaded persistent keys
 * together.
 *
 * This option has no effect when #MBEDTLS_PSA_CRYPTO_C is disabled.
 *
 * Module:  library/psa_crypto.c
 * Requires: MBEDTLS_PSA_CRYPTO_C
 */
#define MBEDTLS_PSA_KEY_STORE_DYNAMIC

/**
 * \def MBEDTLS_PSA_ASSUME_EXCLUSIVE_BUFFERS
 *
 * Assume all buffers passed to PSA functions are owned exclusively by the
 * PSA function and are not stored in shared memory.
 *
 * This option may be enabled if all buffers passed to any PSA function reside
 * in memory that is accessible only to the PSA function during its execution.
 *
 * This option MUST be disabled whenever buffer arguments are in memory shared
 * with an untrusted party, for example where arguments to PSA calls are passed
 * across a trust boundary.
 *
 * \note Enabling this option reduces memory usage and code size.
 *
 * \note Enabling this option causes overlap of input and output buffers
 *       not to be supported by PSA functions.
 */
#define MBEDTLS_PSA_ASSUME_EXCLUSIVE_BUFFERS

#define MBEDTLS_AES_ALT
#define MBEDTLS_HAL_AES_ALT

/**
  * @brief MBEDTLS_HAL_GCM_ALT Enables ST GCM alternative module to replace mbed
  *        TLS GCM module by ST GCM alternative implementation based on STM32
  *        AES hardware accelerator.
  *
  *        Uncomment a macro to enable ST GCM hardware alternative module.
  *        Requires: MBEDTLS_AES_C, MBEDTLS_GCM_C, MBEDTLS_GCM_ALT.
  */
#define MBEDTLS_GCM_ALT
#define MBEDTLS_HAL_GCM_ALT

/**
  * @brief HW_CRYPTO_DPA_GCM Allows DPA resistance for GCM by using secure crypto
  *        processor (SAES) when this option is enabled, GCM becomes DPA-protected.
  *
  * @note Using DPA resistance degrades the performance.
  *
  *       Comment this option if your system can run cryptographic services
  *       without DPA resistance for the highest performance benefit.
  *       Requires: MBEDTLS_HAL_GCM_ALT.
 */
#define HW_CRYPTO_DPA_GCM

/**
  * @brief HW_CRYPTO_DPA_CTR_FOR_GCM Allows DPA resistance for GCM through CTR by
  *        using secure crypto processor (SAES) when this option is enabled,
  *        CTR becomes DPA-protected.
  *        CTR protected mode is mixed with software to create GCM protected mode.
  *        This option can be enabled when the hardware don't support protected GCM.
  *
  * @note Using DPA resistance degrades the performance.
  *
  *       Comment this option if your system can run cryptographic services
  *       without DPA resistance for the highest performance benefit.
  *       Requires: MBEDTLS_HAL_GCM_ALT, HW_CRYPTO_DPA_GCM.
 */
//#define HW_CRYPTO_DPA_CTR_FOR_GCM

#if defined(HW_CRYPTO_DPA_GCM) && defined(HW_CRYPTO_DPA_CTR_FOR_GCM)
#error "HW_CRYPTO_DPA_GCM and HW_CRYPTO_DPA_CTR_FOR_GCM cannot be defined simultaneously"
#endif /* HW_CRYPTO_DPA_GCM && HW_CRYPTO_DPA_CTR_FOR_GCM */

/**
  * @brief MBEDTLS_HAL_SHA256_ALT Enables ST SHA-224 and SHA-256 alternative
  *        modules to replace mbed TLS SHA-224 and SHA-256 modules by ST SHA-224
  *        and SHA-256 alternative implementation based on STM32 HASH hardware
  *        accelerator.
  *
  *        Uncomment a macro to enable ST SHA256 hardware alternative module.
  *        Requires: MBEDTLS_SHA256_C, MBEDTLS_SHA256_ALT.
  */
//#define MBEDTLS_HAL_SHA256_ALT

/**
  * @brief ST_HW_CONTEXT_SAVING Enables ST HASH save context
  *        The HASH context of the interrupted task can be saved from the HASH
  *        registers to memory, and then be restored from memory to the HASH
  *        registers.
  *
  *        Uncomment a macro to enable ST HASH save context.
  *        Requires: MBEDTLS_SHA256_ALT.
  */
//#define ST_HW_CONTEXT_SAVING

#if defined(ST_HW_CONTEXT_SAVING) && (USE_HAL_HASH_SUSPEND_RESUME != 1U)
#error "Enable USE_HAL_HASH_SUSPEND_RESUME flag to save HASH context"
#endif /* ST_HW_CONTEXT_SAVING && USE_HAL_HASH_SUSPEND_RESUME */

/**
  * @brief MBEDTLS_HAL_ECDSA_ALT Enables ST ECDSA alternative module to replace
  *        mbed TLS ECDSA sign and  verify modules by ST ECDSA alternative
  *        implementation based on STM32 PKA hardware accelerator.
  *
  *        Uncomment a macro to enable ST ECDSA hardware alternative module.
  *        Requires: MBEDTLS_ECDSA_C, MBEDTLS_ECDSA_SIGN_ALT,
  *                  MBEDTLS_ECDSA_VERIFY_ALT, MBEDTLS_ECP_ALT.
  */
#define MBEDTLS_ECDSA_VERIFY_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_GENKEY_ALT
#define MBEDTLS_ECDSA_ALT
#define MBEDTLS_HAL_ECDSA_ALT

/**
  * @brief MBEDTLS_HAL_ECDH_ALT Enables ST ECDH alternative module to replace
  *        mbed TLS Compute shared secret module by ST Compute shared secret
  *        alternative implementation based on STM32 PKA hardware accelerator.
  *
  *        Uncomment a macro to enable ST ECDH hardware alternative module.
  *        Requires: MBEDTLS_ECDH_C, MBEDTLS_ECDH_COMPUTE_SHARED_ALT,
  *                  MBEDTLS_ECP_ALT!!!!!.
  */
#define MBEDTLS_ECDH_ALT
#define MBEDTLS_HAL_ECDH_ALT

/**
  * @brief MBEDTLS_HAL_ECP_ALT Enables ST ECP alternative modules to replace
  *        mbed TLS ECP module by ST ECP alternative implementation based on
  *        STM32 PKA hardware accelerator.
  *
  *        Uncomment a macro to enable ST ECP hardware alternative module.
  *        Requires: MBEDTLS_ECP_C, MBEDTLS_ECP_ALT.
  */
#define MBEDTLS_ECP_ALT
#define MBEDTLS_HAL_ECP_ALT

/**
  * @brief MBEDTLS_HAL_RSA_ALT Enables ST RSA alternative modules to replace
  *        mbed TLS RSA module by ST RSA alternative implementation based on
  *        STM32 PKA hardware accelerator.
  *
  *        Uncomment a macro to enable ST RSA hardware alternative module.
  *        Requires: MBEDTLS_RSA_C, MBEDTLS_RSA_ALT.
  */
#define MBEDTLS_RSA_ALT
#define MBEDTLS_HAL_RSA_ALT

/**
  * @brief MBEDTLS_HAL_ENTROPY_HARDWARE_ALT Enables ST entropy source modules
  *        to replace mbed TLS entropy module by ST entropy implementation
  *        based on STM32 RNG hardware accelerator.
  *
  *        Uncomment a macro to enable ST entropy hardware alternative module.
  *        Requires: MBEDTLS_ENTROPY_C, MBEDTLS_ENTROPY_HARDWARE_ALT.
  */
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_HAL_ENTROPY_HARDWARE_ALT


#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PSA_CRYPTO_STORAGE_C


/**
  * @brief PSA_USE_ITS_ALT Enables ITS alternative module to replace
  *        PSA ITS over files module by ITS alternative implementation.
  *
  *        Uncomment a macro to enable ITS alternative module.
  *        Requires: MBEDTLS_PSA_CRYPTO_STORAGE_C.
                     user should disable MBEDTLS_PSA_ITS_FILE_C.
  */
#define PSA_USE_ITS_ALT



/**
  * @brief PSA_USE_ENCRYPTED_ITS Enables encryption feature for ITS.
  *        alternative module using imported user key.
  *
  *        Uncomment a macro to enable Encrypted ITS.
  *        Requires: MBEDTLS_PSA_CRYPTO_STORAGE_C, PSA_USE_ITS_ALT.
  */
#define PSA_USE_ENCRYPTED_ITS

#include "check_crypto_config.h"
#include "mbedtls/build_info.h"
//#include "mbedtls/check_config.h" /*not needed according to migration guide*/
//#include "mbedtls/config_psa.h"   /*not needed according to migration guide*/

int mbedtls_ssl_safer_memcmp( const void *a, const void *b, size_t n );
#ifdef __cplusplus
}
#endif
