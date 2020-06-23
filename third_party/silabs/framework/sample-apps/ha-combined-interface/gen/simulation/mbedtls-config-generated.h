/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file mbedtls-config-generated.h
 * @brief mbed TLS configuration file. This file is generated do not modify it directly. Please use the mbed TLS setup instead.
 *
 *******************************************************************************
   ******************************************************************************/

#ifndef MBEDTLS_CONFIG_GENERATED_H
#define MBEDTLS_CONFIG_GENERATED_H

#if !defined(EMBER_TEST)
#define MBEDTLS_NO_PLATFORM_ENTROPY

#else
// mbedtls/library/entropy_poll.c needs this,
// implicit declaration of function 'syscall' otherwise
#define _GNU_SOURCE
#endif

// Generated content that is coming from contributor plugins

#define MBEDTLS_AES_C

#if defined(MBEDTLS_ENTROPY_C)
// Enable RAIL radio as entropy source by default
// This is going to help on devices that does not have TRNG support by software
// (FR32XG13 (SDID_89) and EFR32XG14 (SDID_95))
#define MBEDTLS_ENTROPY_RAIL_C
#endif

// Inclusion of the Silabs specific device acceleration configuration file.
#if defined(MBEDTLS_DEVICE_ACCELERATION_CONFIG_FILE)
#include MBEDTLS_DEVICE_ACCELERATION_CONFIG_FILE
#endif

// Inclusion of the app specific device acceleration configuration file.
#if defined(MBEDTLS_DEVICE_ACCELERATION_CONFIG_APP_FILE)
#include MBEDTLS_DEVICE_ACCELERATION_CONFIG_APP_FILE
#endif

// Inclusion of the mbed TLS config_check.h header file.
#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_GENERATED_H */
