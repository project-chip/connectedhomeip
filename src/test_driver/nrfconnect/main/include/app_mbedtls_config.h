/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Enable cryptographic functions needed by CHIP which can't be enabled via Kconfig
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_HKDF_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_PKCS5_C
#define MBEDTLS_PK_WRITE_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_PEM_WRITE_C

// Define mbedtls_error()
#define MBEDTLS_ERROR_C

// Use /dev/urandom as entropy generator
#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#undef MBEDTLS_NO_PLATFORM_ENTROPY

// Configure heap usage monitoring
#define MBEDTLS_MEMORY_DEBUG
