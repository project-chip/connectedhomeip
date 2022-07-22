/*
 * mbed Microcontroller Library
 * Copyright (c) 2019 Cypress Semiconductor Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
