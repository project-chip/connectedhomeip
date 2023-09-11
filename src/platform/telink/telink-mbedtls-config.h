/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Telink mbedtls configuration file.
 *
 */

#ifndef MBEDTLS_TSLR9_CONF_H
#define MBEDTLS_TSLR9_CONF_H

#define MBEDTLS_HKDF_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CSR_WRITE_C

#define MBEDTLS_AES_ALT
#define MBEDTLS_ECP_ALT

#endif /* MBEDTLS_TSLR9_CONF_H */
