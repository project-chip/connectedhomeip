/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * TF-PSA-Crypto user config header for MCXW70.
 *
 * The SGI-PKC HW crypto driver (CONFIG_PSA_CRYPTO_DRIVER_SGI_PKC) only adds the
 * runtime HW dispatch path; it does not stop tf-psa-crypto from also compiling
 * the SW builtin for every PSA_WANT_ALG_*. Declaring MBEDTLS_PSA_ACCEL_* here
 * tells tf-psa-crypto the HW driver covers those algorithms, so the redundant
 * SW builtins are dropped, saving Flash and RAM.
 *
 * Only accelerate what the generated psa_crypto_driver_wrappers.h actually
 * implements; accelerating an unsupported op would strip a SW fallback the HW
 * cannot replace. The set matches the validated FreeRTOS reference
 * third_party/nxp/nxp_matter_support/gn_build/mbedtls/config/mcxw72_matter_mbedtls_config.h.
 */

#ifndef NXP_MCXW70_PSA_CRYPTO_CONFIG_H
#define NXP_MCXW70_PSA_CRYPTO_CONFIG_H

/* Algorithm acceleration */
#define MBEDTLS_PSA_ACCEL_ALG_SHA_256 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDSA 1
#define MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDH 1
#define MBEDTLS_PSA_ACCEL_ALG_HMAC 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXTRACT 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXPAND 1
#define MBEDTLS_PSA_ACCEL_ALG_CCM 1

/* AES-CMAC is used by Matter BLE session establishment. */
#define MBEDTLS_PSA_ACCEL_ALG_CMAC 1

/* OpenThread PBKDF2 uses AES-CMAC-PRF-128. */
#if defined(CONFIG_OPENTHREAD)
#define MBEDTLS_PSA_ACCEL_ALG_PBKDF2_AES_CMAC_PRF_128 1
#endif

/* Curve acceleration */
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256 1

/* Key type acceleration */
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1

/*
 * AES / ECB acceleration. Safe now that the boot-seed DRBG is HMAC-DRBG
 * (MBEDTLS_HMAC_DRBG_C) instead of CTR_DRBG; stripping the SW AES builtin no
 * longer flips the boot seed into PSA mode, so no hard fault at psa_crypto_init.
 */
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_AES 1
#define MBEDTLS_PSA_ACCEL_ALG_ECB_NO_PADDING 1

/*
 * NOT accelerated (keep the SW builtin):
 * - ECC key IMPORT/EXPORT/PUBLIC_KEY/BASIC/GENERATE: no matching HW entry point.
 * - ALG_HKDF (single-step): required in SW by PsaKdf::Init; only the two-step
 *   HKDF_EXTRACT/HKDF_EXPAND forms are HW-backed.
 */

#endif /* NXP_MCXW70_PSA_CRYPTO_CONFIG_H */
