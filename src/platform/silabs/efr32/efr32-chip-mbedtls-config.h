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

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

/**
 * DIC Specific Configurations
 */
#ifdef DIC_ENABLE
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_NO_CRT
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_RSA_C
#define MBEDTLS_SSL_IN_CONTENT_LEN 5120
#define MBEDTLS_SSL_OUT_CONTENT_LEN 1560
#define MBEDTLS_PSA_BUILTIN_ALG_TLS12_PRF
#define MBEDTLS_MPI_MAX_SIZE 512 /**< Maximum number of bytes for usable MPIs. */
#endif                           // DIC_ENABLE

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

#define MBEDTLS_AES_ROM_TABLES
// #define MBEDTLS_AES_FEWER_TABLES /* Check if we want this */

#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_SHA256_SMALLER    /* Check if we want this */
#define MBEDTLS_MPI_WINDOW_SIZE 1 /**< Maximum windows size used. */

// Includes generated file for all generic defines.
#include "sl_mbedtls_config.h"
