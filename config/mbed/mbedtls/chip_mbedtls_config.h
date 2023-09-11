/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2006-2015, ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/**
 *
 * \brief CHIP mbedTLS configuration options for mbed platform
 *
 *  This set of compile-time options may be used to enable
 *  or disable features selectively, and reduce the global
 *  memory footprint.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef CHIP_MBEDTLS_CONFIG_H
#define CHIP_MBEDTLS_CONFIG_H

#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_PEM_WRITE_C
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PKCS5_C
#define MBEDTLS_SHA1_C
#undef MBEDTLS_NET_C
#undef MBEDTLS_TIMING_C
#undef MBEDTLS_FS_IO

#endif /* CHIP_MBEDTLS_CONFIG_H */
