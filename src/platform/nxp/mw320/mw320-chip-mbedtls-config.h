/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#ifndef MW320_MBEDTLS_CONFIG_H
#define MW320_MBEDTLS_CONFIG_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(MBEDTLS_ECP_WINDOW_SIZE)
#undef MBEDTLS_ECP_WINDOW_SIZE
#define MBEDTLS_ECP_WINDOW_SIZE 4 /**< Maximum window size used */
#endif

#if defined(MBEDTLS_ECP_FIXED_POINT_OPTIM)
#undef MBEDTLS_ECP_FIXED_POINT_OPTIM
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 1 /**< Enable fixed-point speed-up */
#endif

/**
 * \def MBEDTLS_AES_ALT
 *
 * Enable hardware acceleration for the AES block cipher
 *
 * See MBEDTLS_AES_C for more information.
 */

#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECP_RESTARTABLE
#define MBEDTLS_ECDH_C

#endif // MW320_MBEDTLS_CONFIG_H
