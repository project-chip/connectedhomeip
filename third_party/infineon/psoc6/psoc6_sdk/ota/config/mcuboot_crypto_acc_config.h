/*
 * mbed Microcontroller Library
 * SPDX-FileCopyrightText: 2019 Cypress Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * \file    mcuboot_crypto_acc_config.h
 * \version 1.1
 */

#ifndef MCUBOOT_MBEDTLS_DEVICE_H
#define MCUBOOT_MBEDTLS_DEVICE_H

/* Currently this target supports SHA1 */
// #define MBEDTLS_SHA1_C

#define MBEDTLS_SHA1_ALT
#define MBEDTLS_SHA256_ALT
#define MBEDTLS_SHA512_ALT

/* Currently this target supports CBC, CFB, OFB, CTR and XTS cipher modes */
#define MBEDTLS_AES_ALT
// #define MBEDTLS_CIPHER_MODE_CBC
// #define MBEDTLS_CIPHER_MODE_CFB
// #define MBEDTLS_CIPHER_MODE_OFB
// #define MBEDTLS_CIPHER_MODE_CTR
// #define MBEDTLS_CIPHER_MODE_XTS

/* Only NIST-P curves are currently supported */
#define MBEDTLS_ECP_ALT
// #define MBEDTLS_ECP_DP_SECP192R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP224R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP256R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP384R1_ENABLED
// #define MBEDTLS_ECP_DP_SECP521R1_ENABLED

#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT

#endif /* MCUBOOT_MBEDTLS_DEVICE_H */
