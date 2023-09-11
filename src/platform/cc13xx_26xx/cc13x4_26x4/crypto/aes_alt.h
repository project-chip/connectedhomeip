/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls-config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_AES_ALT)

#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    CryptoKey cryptoKey;      /*!<  structure for the AES driver */
    uint32_t keyMaterial[16]; /*!<  memory for the key bytes used by cryptoKey */
} mbedtls_aes_context;

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_AES_ALT */
