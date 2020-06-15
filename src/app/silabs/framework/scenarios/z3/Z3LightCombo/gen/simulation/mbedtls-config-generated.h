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
#if defined (MBEDTLS_DEVICE_ACCELERATION_CONFIG_FILE)
#include MBEDTLS_DEVICE_ACCELERATION_CONFIG_FILE
#endif

#if !defined(MBEDTLS_ENTROPY_C)
// Workaround for issues caused when the device-acceleration-config file is used,
// *and* every file is added to the build. Mbed TLS code should not only check
// presence of MBEDTLS_ENTROPY_RAIL_C or MBEDTLS_ENTROPY_ADC_C, but
// MBEDTLS_ENTROPY_C as well.
// TODO: First two is adderessed in mbedtls PR-78. Needs to take care of the rest.
#undef MBEDTLS_ENTROPY_RAIL_C
#undef MBEDTLS_ENTROPY_ADC_C
#undef MBEDTLS_ENTROPY_HARDWARE_ALT
#undef MBEDTLS_ENTROPY_HARDWARE_ALT_RAIL
#endif

#if !defined(MBEDTLS_CTR_DRBG_C)
// Workaround for issues caused when the device-acceleration-config file is used,
// *and* every file is added to the build.
// TODO: Needs a way to not define it if not needed by the application.
#undef MBEDTLS_TRNG_C
#endif

#if !defined(MBEDTLS_CMAC_C)
// Workaround for issues caused when the device-acceleration-config file is used,
// *and* every file is added to the build.
// TODO: Needs a way to not define it if not needed by the application.
#undef MBEDTLS_CMAC_ALT
#endif

// Inclusion of the mbed TLS config_check.h header file.
#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_GENERATED_H */
