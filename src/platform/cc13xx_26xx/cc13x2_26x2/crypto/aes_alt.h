/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
