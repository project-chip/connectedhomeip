/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls-config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SHA256_ALT)
#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

typedef struct
{
    SHA2_Handle hndl;         /*!< A handle that is returned by the SHA driver  */
    SHA2_Config config;       /*!< structure containing SHA2 driver specific implementation  */
    SHA2CC26X2_Object object; /*!< Pointer to a driver specific data object */
} mbedtls_sha256_context;

#ifdef __cplusplus
}
#endif
#endif /* MBEDTLS_SHA256_ALT */
