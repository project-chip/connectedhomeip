/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *    @file
 *      This file defines default compile-time configuration constants
 *      for CHIP.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific chipProjectConfig.h header and then assert
 *      HAVE_CHIPPROJECTCONFIG_H via the package configuration tool
 *      via --with-chip-project-includes=DIR where DIR is the
 *      directory that contains the header.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#if CHIP_HAVE_CONFIG_H
#include <core/CHIPBuildConfig.h>
#endif

#include <ble/BleConfig.h>
#include <system/SystemConfig.h>

/* COMING SOON: making the INET Layer optional entails making this inclusion optional. */
//#include "InetConfig.h"
/*
#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL <= 0
#error "chip SDK requires INET_TCP_IDLE_CHECK_INTERVAL > 0"
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL <= 0
*/
/* Include a project-specific configuration file, if defined.
 *
 * An application or module that incorporates chip can define a project configuration
 * file to override standard chip configuration with application-specific values.
 * The chipProjectConfig.h file is typically located outside the CHIP source tree,
 * alongside the source code for the application.
 */
#ifdef CHIP_PROJECT_CONFIG_INCLUDE
#include CHIP_PROJECT_CONFIG_INCLUDE
#endif

/* Include a platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard chip configuration
 * that are specific to the platform or OS on which chip is running.  It is typically
 * provided as apart of an adaptation layer that adapts CHIP to the target
 * environment.  This adaptation layer may be included in the CHIP source tree
 * itself or implemented externally.
 */
#ifdef CHIP_PLATFORM_CONFIG_INCLUDE
#include CHIP_PLATFORM_CONFIG_INCLUDE
#endif

// Profile-specific Configuration Headers

/**
 *  @def CHIP_CONFIG_ERROR_TYPE
 *
 *  @brief
 *    This defines the data type used to represent errors for chip.
 *
 */
#ifndef CHIP_CONFIG_ERROR_TYPE
#include <inttypes.h>
#include <stdint.h>

#define CHIP_CONFIG_ERROR_TYPE int32_t
#define CHIP_CONFIG_ERROR_FORMAT PRId32
#endif // CHIP_CONFIG_ERROR_TYPE

/**
 *  @def CHIP_CONFIG_CORE_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum chip error number range.
 *
 */
#ifndef CHIP_CONFIG_CORE_ERROR_MIN
#define CHIP_CONFIG_CORE_ERROR_MIN 4000
#endif // CHIP_CONFIG_CORE_ERROR_MIN

/**
 *  @def CHIP_CONFIG_CORE_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum chip error number range.
 *
 */
#ifndef CHIP_CONFIG_CORE_ERROR_MAX
#define CHIP_CONFIG_CORE_ERROR_MAX 4999
#endif // CHIP_CONFIG_CORE_ERROR_MAX

/**
 *  @def CHIP_CONFIG_CORE_ERROR
 *
 *  @brief
 *    This defines a mapping function for chip errors that allows
 *    mapping such errors into a platform- or system-specific manner.
 *
 */
#ifndef CHIP_CONFIG_CORE_ERROR
#define CHIP_CONFIG_CORE_ERROR(e) (CHIP_CONFIG_CORE_ERROR_MIN + (e))
#endif // CHIP_CONFIG_CORE_ERROR

/**
 *  @def CHIP_CONFIG_USE_OPENSSL_ECC
 *
 *  @brief
 *    Use the OpenSSL implementation of the elliptic curve primitives
 *    for chip communication.
 *
 *    Note that this option is mutually exclusive with
 *    #CHIP_CONFIG_USE_MICRO_ECC.
 */
#ifndef CHIP_CONFIG_USE_OPENSSL_ECC
#define CHIP_CONFIG_USE_OPENSSL_ECC 1
#endif // CHIP_CONFIG_USE_OPENSSL_ECC

/**
 *  @def CHIP_CONFIG_USE_MICRO_ECC
 *
 *  @brief
 *    Use the Micro ECC implementation of the elliptic curve primitives
 *    for chip communication.
 *
 *    Note that this option is mutually exclusive with
 *    #CHIP_CONFIG_USE_OPENSSL_ECC.
 *
 */
#ifndef CHIP_CONFIG_USE_MICRO_ECC
#define CHIP_CONFIG_USE_MICRO_ECC 0
#endif // CHIP_CONFIG_USE_MICRO_ECC

#if CHIP_CONFIG_USE_MICRO_ECC && CHIP_CONFIG_USE_OPENSSL_ECC
#error "Please assert one of either CHIP_CONFIG_USE_MICRO_ECC or CHIP_CONFIG_USE_OPENSSL_ECC, but not both."
#endif // CHIP_CONFIG_USE_MICRO_ECC && CHIP_CONFIG_USE_OPENSSL_ECC

/**
 *  @name chip Elliptic Curve Security Configuration
 *
 *  @brief
 *    The following definitions enable one or more of four potential
 *    elliptic curves:
 *
 *      * #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
 *      * #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
 *      * #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
 *      * #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp160r1 elliptic curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
#define CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1 0
#endif // CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1

/**
 *  @def CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp192r1 elliptic curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#define CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1 1
#endif // CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1

/**
 *  @def CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp224r1 / National
 *    Institute of Standards (NIST) P-224 elliptic curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#define CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1 1
#endif // CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1

/**
 *  @def CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp256r1 / American
 *    National Standards Institute (ANSI) prime256v1 / National
 *    Institute of Standards (NIST) P-256 elliptic curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1 1
#endif // CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1

/**
 *  @}
 */

/**
 *  @name chip Password Authenticated Session Establishment (PASE) Configuration
 *
 *  @brief
 *    The following definitions define the configurations supported
 *    for chip's Password Authenticated Session Establishment (PASE)
 *    protocol.
 *
 *    This protocol is used primarily for establishing a secure
 *    session for provisioning. chip supports the following PASE
 *    configurations:
 *
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG1
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG2
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG3
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG4
 *    * #CHIP_CONFIG_SUPPORT_PASE_CONFIG5
 *
 *    which are summarized in the table below:
 *
 *    | Configuration  |  J-PAKE Style   |   Curve   |  Test Only  | Notes                                |
 *    | :------------: | :-------------- | :-------: | :---------: | :----------------------------------- |
 *    | 0              | -               | -         | Y           | Test-only                            |
 *    | 1              | Finite Field    | -         | N           | Original chip default configuration |
 *    | 2              | Elliptic Curve  | secp160r1 | N           |                                      |
 *    | 3              | Elliptic Curve  | secp192r1 | N           |                                      |
 *    | 4              | Elliptic Curve  | secp224r1 | N           | Future chip default configuration   |
 *    | 5              | Elliptic Curve  | secp256r1 | N           |                                      |
 *
 *  @{
 *
 */

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
 *
 *  @brief
 *    This chip PASE configuration does not use the J-PAKE algorithm
 *    and sends deterministic messages over the communications
 *    channel. The size and structure of the messages are similar to
 *    #CHIP_CONFIG_SUPPORT_PASE_CONFIG5.
 *
 *  @note The results of this configuration are insecure because the
 *        computational overhead of the cryptography has largely been
 *        disabled since the focus of this configuration is testing
 *        the overall PASE protocol exchange, independently of the
 *        cryptography.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY 0
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG1
 *
 *  @brief
 *    This chip PASE configuration uses Finite Field J-PAKE and is
 *    the original, default chip PASE configuration.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG1
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG1 1
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG1

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG2
 *
 *  @brief
 *    This chip PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp160r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1)
 *          should also be enabled.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG2
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG2 0
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG2

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG3
 *
 *  @brief
 *    This chip PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp192r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1)
 *          should also be enabled.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG3
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG3 0
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG3

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG4
 *
 *  @brief
 *    This chip PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp224r1 curve and will be the new, default chip PASE
 *    configuration.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1)
 *          should also be enabled.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG4
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG4 1
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG4

/**
 *  @def CHIP_CONFIG_SUPPORT_PASE_CONFIG5
 *
 *  @brief
 *    This chip PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp256r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1)
 *          should also be enabled.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASE_CONFIG5
#define CHIP_CONFIG_SUPPORT_PASE_CONFIG5 0
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG5

/**
 *  @}
 */

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG2 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
#error "Please assert CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1 when CHIP_CONFIG_SUPPORT_PASE_CONFIG2 is asserted"
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG2 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG3 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#error "Please assert CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1 when CHIP_CONFIG_SUPPORT_PASE_CONFIG3 is asserted"
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG3 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG4 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#error "Please assert CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1 when CHIP_CONFIG_SUPPORT_PASE_CONFIG4 is asserted"
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG4 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG5 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#error "Please assert CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1 when CHIP_CONFIG_SUPPORT_PASE_CONFIG5 is asserted"
#endif // CHIP_CONFIG_SUPPORT_PASE_CONFIG5 && !CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1

/**
 *  @def CHIP_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT
 *
 *  @brief
 *    Align payload on 4-byte boundary for PASE messages.
 *    Currently, payload alignment is required only when micro-ecc
 *    library is used and it is compiled with ARM assembly.
 *    If implementation guarantees that payload is always 4-byte
 *    aligned this option should stay deasserted to save code size.
 *
 */
#ifndef CHIP_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT
#if CHIP_CONFIG_USE_MICRO_ECC
#define CHIP_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT 1
#else
#define CHIP_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT 0
#endif // CHIP_CONFIG_USE_MICRO_ECC
#endif // CHIP_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT

/**
 *  @def CHIP_CONFIG_PASE_RATE_LIMITER_TIMEOUT
 *
 *  @brief
 *    The amount of time (in milliseconds) in which the Security Manager
 *    is allowed to have maximum #CHIP_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
 *    counted PASE attempts.
 *
 */
#ifndef CHIP_CONFIG_PASE_RATE_LIMITER_TIMEOUT
#define CHIP_CONFIG_PASE_RATE_LIMITER_TIMEOUT 15000
#endif // CHIP_CONFIG_PASE_RATE_LIMITER_TIMEOUT

/**
 *  @def CHIP_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
 *
 *  @brief
 *    The maximum number of PASE attempts after which the
 *    next PASE session establishment attempt will be allowed
 *    only after #CHIP_CONFIG_PASE_RATE_LIMITER_TIMEOUT expires.
 *     * For PASE negotiations with key confirmation option enabled:
 *       only attempts that failed with key confirmation error are counted.
 *       Successful PASE negotiations do not reset the rate limiter.
 *     * For PASE negotiations with key confirmation option disabled:
 *       every PASE negotiation, successful or otherwise, is added
 *       to the rate limiter.
 *
 */
#ifndef CHIP_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
#define CHIP_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS 3
#endif // CHIP_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS

/**
 *  @name chip Security Manager Memory Management Configuration
 *
 *  @brief
 *    The following definitions enable one of three potential chip
 *    Security Manager memory-management options:
 *
 *      * #CHIP_CONFIG_MEMORY_MGMT_PLATFORM
 *      * #CHIP_CONFIG_MEMORY_MGMT_SIMPLE
 *      * #CHIP_CONFIG_MEMORY_MGMT_MALLOC
 *
 *    Note that these options are mutually exclusive and only one
 *    of these options should be set.
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_MEMORY_MGMT_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of chip Security Manager memory-management
 *    functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_MEMORY_MGMT_SIMPLE and
 *        #CHIP_CONFIG_MEMORY_MGMT_MALLOC.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_MGMT_PLATFORM
#define CHIP_CONFIG_MEMORY_MGMT_PLATFORM 0
#endif // CHIP_CONFIG_MEMORY_MGMT_PLATFORM

/**
 *  @def CHIP_CONFIG_MEMORY_MGMT_SIMPLE
 *
 *  @brief
 *    Enable (1) or disable (0) support for a chip-provided
 *    implementation of chip Security Manager memory-management
 *    functions based on temporary network buffer allocation /
 *    release.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_MEMORY_MGMT_PLATFORM and
 *        #CHIP_CONFIG_MEMORY_MGMT_MALLOC.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_MGMT_SIMPLE
#define CHIP_CONFIG_MEMORY_MGMT_SIMPLE 0
#endif // CHIP_CONFIG_MEMORY_MGMT_SIMPLE

/**
 *  @def CHIP_CONFIG_MEMORY_MGMT_MALLOC
 *
 *  @brief
 *    Enable (1) or disable (0) support for a chip-provided
 *    implementation of chip Security Manager memory-management
 *    functions based on the C Standard Library malloc / free
 *    functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_MEMORY_MGMT_PLATFORM and
 *        #CHIP_CONFIG_MEMORY_MGMT_SIMPLE.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_MGMT_MALLOC
#define CHIP_CONFIG_MEMORY_MGMT_MALLOC 1
#endif // CHIP_CONFIG_MEMORY_MGMT_MALLOC

/**
 *  @}
 */

#if ((CHIP_CONFIG_MEMORY_MGMT_PLATFORM + CHIP_CONFIG_MEMORY_MGMT_SIMPLE + CHIP_CONFIG_MEMORY_MGMT_MALLOC) != 1)
#error                                                                                                                             \
    "Please assert exactly one of CHIP_CONFIG_MEMORY_MGMT_PLATFORM, CHIP_CONFIG_MEMORY_MGMT_SIMPLE, or CHIP_CONFIG_MEMORY_MGMT_MALLOC."
#endif // ((CHIP_CONFIG_MEMORY_MGMT_PLATFORM + CHIP_CONFIG_MEMORY_MGMT_SIMPLE + CHIP_CONFIG_MEMORY_MGMT_MALLOC) != 1)

#if !CHIP_CONFIG_MEMORY_MGMT_MALLOC && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#error "!CHIP_CONFIG_MEMORY_MGMT_MALLOC but getifaddrs() uses malloc()"
#endif

/**
 *  @def CHIP_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS
 *
 *  @brief
 *    Enable (1) or disable (0) simple memory allocator support
 *    for small size network buffers. When enabled, this configuration
 *    requires 4 network buffers with minimum available payload size of
 *    600 bytes.
 *
 *  @note This configuration is only relevant when
 *        #CHIP_CONFIG_MEMORY_MGMT_SIMPLE is set and
 *        ignored otherwise.
 *
 */
#ifndef CHIP_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS
#define CHIP_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS 0
#endif // CHIP_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS

/**
 *  @def CHIP_CONFIG_MEMORY_DEBUG_CHECKS
 *
 *  @brief
 *    Enable (1) or disable (0) building with additional code
 *    for memory-related checks.
 */
#ifndef CHIP_CONFIG_MEMORY_DEBUG_CHECKS
#define CHIP_CONFIG_MEMORY_DEBUG_CHECKS 0
#endif // CHIP_CONFIG_MEMORY_DEBUG_CHECKS

/**
 *  @def CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
 *
 *  @brief
 *    Enable (1) or disable (0) malloc memory allocator support
 *    for dmalloc, an open-source debug malloc library. When enabled,
 *    additional checks and logging of allocations may be performed,
 *    with some performance cost.
 *
 *  @note This configuration is most relevant when
 *        #CHIP_CONFIG_MEMORY_MGMT_MALLOC is set, but may also
 *        affect other configurations where application or platform
 *        code uses the malloc() family.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
#define CHIP_CONFIG_MEMORY_DEBUG_DMALLOC 0
#endif // CHIP_CONFIG_MEMORY_DEBUG_DMALLOC

/**
 *  @name chip Security Manager Time-Consuming Crypto Alerts.
 *
 *  @brief
 *    The following definitions enable one of two potential chip
 *    Security Manager time-consuming crypto alerts implementations:
 *
 *      * #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
 *      * #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
 *
 *    Note that these options are mutually exclusive and only one
 *    of these options should be set.
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
 *
 *  @brief
 *    Enable (1) or disable (0) support for chip-provided dummy
 *    implementation of chip security manager time-consuming
 *    crypto alerts functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM.
 *
 */
#ifndef CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
#define CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY 1
#endif // CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY

/**
 *  @def CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for a platform-specific
 *    implementation of chip security manager time-consuming
 *    crypto alerts functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY.
 *
 */
#ifndef CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
#define CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM 0
#endif // CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM

/**
 *  @}
 */

#if ((CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY + CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM) != 1)
#error "Please assert exactly one of CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY or CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM."
#endif // ((CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY + CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM) != 1)

/**
 *  @name chip Random Number Generator (RNG) Implementation Configuration
 *
 *  @brief
 *    The following definitions enable one of three potential chip
 *    RNG implementation options:
 *
 *      * #CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM
 *      * #CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
 *      * #CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the chip Random Number Generator.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG and
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL.
 *
 */
#ifndef CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM
#define CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM 0
#endif // CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM

/**
 *  @def CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
 *
 *  @brief
 *    Enable (1) or disable (0) support for a chip-provided
 *    implementation of the chip Random Number Generator.
 *    This implementation is based on AES-CTR DRBG as
 *    specified in the NIST SP800-90A document.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM and
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL.
 *
 */
#ifndef CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
#define CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG 0
#endif // CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG

/**
 *  @def CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for a standard OpenSSL
 *    implementation of the chip Random Number Generator.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM and
 *        #CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG.
 *
 */
#ifndef CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL
#define CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL 1
#endif // CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL

/**
 *  @}
 */

#if ((CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM + CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG +                                          \
      CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL) != 1)
#error                                                                                                                             \
    "Please assert exactly one of CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM, CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG, or CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL."
#endif // ((CHIP_CONFIG_RNG_IMPLEMENTATION_PLATFORM + CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG +
       // CHIP_CONFIG_RNG_IMPLEMENTATION_OPENSSL) != 1)

/**
 *  @def CHIP_CONFIG_DEV_RANDOM_DRBG_SEED
 *
 *  @brief
 *    Enable (1) or disable (0) a function for seeding the DRBG with
 *    entropy from the /dev/(u)random device.
 *
 *  @note When enabled along with #CHIP_CONFIG_RNG_IMPLEMENTATION_CHIPDRBG
 *        this function becomes the default seeding function for the DRBG if
 *        another isn't specified at initialization time.
 *
 */
#ifndef CHIP_CONFIG_DEV_RANDOM_DRBG_SEED
#define CHIP_CONFIG_DEV_RANDOM_DRBG_SEED 0
#endif // CHIP_CONFIG_DEV_RANDOM_DRBG_SEED

/**
 *  @def CHIP_CONFIG_DEV_RANDOM_DEVICE_NAME
 *
 *  @brief
 *    The device name used by the dev random entropy function.
 *
 *  @note Only meaningful when #CHIP_CONFIG_DEV_RANDOM_DRBG_SEED is enabled.
 *
 */
#ifndef CHIP_CONFIG_DEV_RANDOM_DEVICE_NAME
#define CHIP_CONFIG_DEV_RANDOM_DEVICE_NAME "/dev/urandom"
#endif // CHIP_CONFIG_DEV_RANDOM_DEVICE_NAME

/**
 *  @name chip AES Block Cipher Algorithm Implementation Configuration.
 *
 *  @brief
 *    The following definitions enable one of the potential chip
 *    AES implementation options:
 *
 *      * #CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM
 *      * #CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the chip AES functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL and
 *        #CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
 *
 */
#ifndef CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM
#define CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM 0
#endif // CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM

/**
 *  @def CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for the OpenSSL
 *    implementation of the chip AES functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL
#define CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL 1
#endif // CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL

/**
 *  @def CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
 *
 *  @brief
 *    Enable (1) or disable (0) support for an implementation
 *    of the chip AES functions using Intel AES-NI intrinsics.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_AES_IMPLEMENTATION_AESNI
#define CHIP_CONFIG_AES_IMPLEMENTATION_AESNI 0
#endif // CHIP_CONFIG_AES_IMPLEMENTATION_AESNI

/**
 *  @def CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS
 *
 *  @brief
 *    Enable (1) or disable (0) support the mbed TLS
 *    implementation of the chip AES functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS
#define CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS 0
#endif // CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS

/**
 *  @}
 */

#if ((CHIP_CONFIG_AES_IMPLEMENTATION_PLATFORM + CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL + CHIP_CONFIG_AES_IMPLEMENTATION_AESNI +    \
      CHIP_CONFIG_AES_IMPLEMENTATION_MBEDTLS) != 1)
#error "Please assert exactly one CHIP_CONFIG_AES_IMPLEMENTATION_... option."
#endif

/**
 *  @def CHIP_CONFIG_AES_USE_EXPANDED_KEY
 *
 *  @brief
 *    Defines whether AES key is used in its expanded (1) or native (0) form.
 *
 *  @note OpenSSL AES implementation uses its own AES key declaration
 *        and this configuration option is ignored when
 *        #CHIP_CONFIG_AES_IMPLEMENTATION_OPENSSL is set.
 *
 */
#ifndef CHIP_CONFIG_AES_USE_EXPANDED_KEY
#define CHIP_CONFIG_AES_USE_EXPANDED_KEY 0
#endif // CHIP_CONFIG_AES_USE_EXPANDED_KEY

/**
 *  @name chip SHA1 and SHA256 Hash Algorithms Implementation Configuration.
 *
 *  @brief
 *    The following definitions enable one of three potential chip
 *    hash implementation options:
 *
 *      * #CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
 *      * #CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
 *      * #CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
 *      * #CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the chip SHA1 and SHA256 hashes.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM
#define CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM 0
#endif // CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM

/**
 *  @def CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
 *
 *  @brief
 *    Enable (1) or disable (0) support for a chip-provided
 *    implementation of the chip SHA1 and SHA256 hash functions.
 *    This implementation is using sha1 and sha256 engines from
 *    mincrypt library of Android core.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT 0
#endif // CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT

/**
 *  @def CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for the OpenSSL
 *    implementation of the chip SHA1 and SHA256 hash functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL
#define CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL 1
#endif // CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL

/**
 *  @def CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
 *
 *  @brief
 *    Enable (1) or disable (0) support for the mbedTLS
 *    implementation of the chip SHA1 and SHA256 hash functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        CHIP_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
#define CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS 0
#endif // CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS

/**
 *  @}
 */

#if ((CHIP_CONFIG_HASH_IMPLEMENTATION_PLATFORM + CHIP_CONFIG_HASH_IMPLEMENTATION_MINCRYPT +                                        \
      CHIP_CONFIG_HASH_IMPLEMENTATION_OPENSSL + CHIP_CONFIG_HASH_IMPLEMENTATION_MBEDTLS) != 1)
#error "Please assert exactly one CHIP_CONFIG_HASH_IMPLEMENTATION_... option."
#endif

/**
 *  @name chip key export protocol configuration.
 *
 *  @brief
 *    The following definitions define the configurations supported
 *    for chip's key export protocol.
 *
 *    This protocol is used to export secret key material from chip device.
 *    chip supports the following protocol configurations:
 *
 *    * #CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
 *    * #CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
 *
 *    which are summarized in the table below:
 *
 *    | Configuration  |   Curve   | Notes                   |
 *    | :------------: | :-------: | :---------------------- |
 *    | 1              | secp224r1 | Default configuration   |
 *    | 2              | secp256r1 |                         |
 *
 *  @{
 *
 */

/**
 *  @def CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
 *
 *  @brief
 *    This chip key export protocol configuration uses secp224r1
 *    Elliptic Curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
#define CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1 1
#endif // CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1

/**
 *  @def CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
 *
 *  @brief
 *    This chip key export protocol configuration uses secp256r1
 *    Elliptic Curve.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
#define CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2 1
#endif // CHIP_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2

/**
 *  @}
 */

/**
 *  @def CHIP_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES
 *
 *  @brief
 *    Allow the use of elliptic curves beyond the standard ones
 *    supported by chip.
 *
 */
#ifndef CHIP_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES
#define CHIP_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES 0
#endif // CHIP_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES

/**
 *  @def CHIP_CONFIG_MAX_EC_BITS
 *
 *  @brief
 *    The maximum size elliptic curve supported, in bits.
 *
 */
#ifndef CHIP_CONFIG_MAX_EC_BITS
#define CHIP_CONFIG_MAX_EC_BITS 256
#endif // CHIP_CONFIG_MAX_EC_BITS

/**
 *  @def CHIP_CONFIG_MAX_RSA_BITS
 *
 *  @brief
 *    The maximum size RSA modulus supported, in bits.
 *
 */
#ifndef CHIP_CONFIG_MAX_RSA_BITS
#define CHIP_CONFIG_MAX_RSA_BITS 4096
#endif // CHIP_CONFIG_MAX_RSA_BITS

/**
 *  @def CHIP_CONFIG_MAX_PEER_NODES
 *
 *  @brief
 *    Maximum number of peer nodes that the local node can communicate
 *    with.
 *
 */
#ifndef CHIP_CONFIG_MAX_PEER_NODES
#define CHIP_CONFIG_MAX_PEER_NODES 128
#endif // CHIP_CONFIG_MAX_PEER_NODES

/**
 *  @def CHIP_CONFIG_MAX_CONNECTIONS
 *
 *  @brief
 *    Maximum number of simultaneously active connections.
 *
 */
#ifndef CHIP_CONFIG_MAX_CONNECTIONS
#define CHIP_CONFIG_MAX_CONNECTIONS INET_CONFIG_NUM_TCP_ENDPOINTS
#endif // CHIP_CONFIG_MAX_CONNECTIONS

/**
 *  @def CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
 *
 *  @brief
 *    Maximum number of simultaneously active inbound TCP connections.
 *
 *    Regardless of what #CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
 *    is set to, the total number of inbound connections cannot exceed
 *    #CHIP_CONFIG_MAX_CONNECTIONS, which is the overall limit for
 *    inbound and outbound connections.
 */
#ifndef CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
#define CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS (CHIP_CONFIG_MAX_CONNECTIONS * 4 / 5)
#endif // CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS

/**
 *  @def CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
 *
 *  @brief
 *    Maximum number of simultaneously active inbound TCP connections
 *    from the single IP address.
 *
 *    Regardless of what #CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
 *    is set to, the total number of inbound connections from a single IP
 *    address cannot exceed #CHIP_CONFIG_MAX_CONNECTIONS or
 *    #CHIP_CONFIG_MAX_INCOMING_TCP_CONNECTIONS.
 */
#ifndef CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
#define CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP 2
#endif // CHIP_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP

/**
 *  @def CHIP_CONFIG_MAX_SESSION_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously active session keys.
 *
 */
#ifndef CHIP_CONFIG_MAX_SESSION_KEYS
#define CHIP_CONFIG_MAX_SESSION_KEYS CHIP_CONFIG_MAX_CONNECTIONS
#endif // CHIP_CONFIG_MAX_SESSION_KEYS

/**
 *  @def CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously supported application epoch keys.
 *    This define should be set to the maximum number of epoch keys
 *    that can be simultaneously provisioned on chip node by chip
 *    service. The maximum supported value is 8, however, in most cases
 *    only two such keys will exist on device at any given point in time.
 *
 */
#ifndef CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS
#define CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS 4
#endif // CHIP_CONFIG_MAX_APPLICATION_EPOCH_KEYS

/**
 *  @def CHIP_CONFIG_MAX_APPLICATION_GROUPS
 *
 *  @brief
 *    Maximum number of simultaneously supported application groups.
 *    This define should be set to the number of chip application
 *    groups, in which associated chip node has membership.
 *
 */
#ifndef CHIP_CONFIG_MAX_APPLICATION_GROUPS
#define CHIP_CONFIG_MAX_APPLICATION_GROUPS 8
#endif // CHIP_CONFIG_MAX_APPLICATION_GROUPS

/**
 *  @def CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
 *
 *  @brief
 *    Enable (1) or disable (0) support for the application group keys
 *    used for chip message encryption.
 *
 */
#ifndef CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
#define CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC 1
#endif // CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

/**
 *  @def CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously cached chip message encryption
 *    application keys.
 *    Caching these keys speeds up message encoding/decoding processes
 *    and eliminates the need to retrieve constituent key material from
 *    the platform memory every time we derive these keys.
 *    This define can be set equal to the number of application groups
 *    (#CHIP_CONFIG_MAX_APPLICATION_GROUPS) supported by the chip node
 *    such that exactly one key can be cached for each application group.
 *    It might be a good idea to allocate few more entries in the key
 *    cache for the corner cases, where application group is having
 *    simultaneous conversations using an 'old' and a 'new' epoch key.
 *
 *  @note This configuration is only relevant when
 *        #CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC is set and
 *        ignored otherwise.
 *
 */
#ifndef CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS
#define CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS (CHIP_CONFIG_MAX_APPLICATION_GROUPS + 1)
#endif // CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS

#if !(CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS > 0 && CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS < 256)
#error "Please set CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS to a value greater than zero and smaller than 256."
#endif // !(CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS > 0 && CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS < 256)

/**
 *  @name chip Encrypted Passcode Configuration
 *
 *  @brief
 *    The following definitions enable (1) or disable (0) supported for
 *    chip encrypted passcode configurations. Each configuration
 *    uniquely specifies how chip passcode was encrypted, authenticated,
 *    and structured. chip supports the following passcode
 *    configurations:
 *
 *    * #CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
 *    * #CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG2
 *
 *    which are summarized in the table below:
 *
 *    | Configuration | Encryption | Authentication | Fingerprint | Notes                 |
 *    | :-----------: | :--------: | :------------: | :---------: | :-------------------- |
 *    | 1             | -          | SHA1 Hash      | SHA1 Hash   | Test-only             |
 *    | 2             | AES128-ECB | SHA1 HMAC      | SHA1 HMAC   | Default configuration |
 *
 *  @{
 *
 */

/**
 *  @def CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
 *
 *  @brief
 *    This chip passcode configuration does not encrypt the passcode
 *    and doesn't use secret keys to authenticate and uniquely identify
 *    (fingerprint) the passcode.
 *
 *  @note For this configuration the computational overhead of the
 *        cryptography has largely been disabled since the focus
 *        of this configuration is testing the overall passcode
 *        encryption/decryption protocol, independently of the
 *        cryptography.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
#define CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY 0
#endif // CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY

/**
 *  @def CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG2
 *
 *  @brief
 *    This chip passcode configuration uses AES128 algorithm in ECB
 *    mode to encrypt passcodes. It also uses SHA1 Hash-based Message
 *    Authentication Code (HMAC) to authenticate and uniquely identify
 *    (fingerprint) the passcode.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG2
#define CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG2 1
#endif // CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG2

/**
 *  @}
 */

/**
 *  @def CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT
 *
 *  @brief
 *    The default amount of time, in milliseconds, after which an in-progess
 *    session establishment will fail due to a timeout.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT
#define CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT 30000
#endif // CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT

/**
 *  @def CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT
 *
 *  @brief
 *    The default minimum amount of time, in milliseconds, that an unreserved and idle
 *    security session will be allowed to exist before being destroyed.  In practice,
 *    unreserved idle sessions can exist for up to twice this value.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT
#define CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT 15000
#endif // CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT

/**
 *  @def CHIP_CONFIG_NUM_MESSAGE_BUFS
 *
 *  @brief
 *    Total number of message buffers. Only used for the BSD sockets
 *    configuration.
 *
 */
#ifndef CHIP_CONFIG_NUM_MESSAGE_BUFS
#define CHIP_CONFIG_NUM_MESSAGE_BUFS 16
#endif // CHIP_CONFIG_NUM_MESSAGE_BUFS

/**
 *  @def CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
 *
 *  @brief
 *    Maximum number of simultaneously active unsolicited message
 *    handlers.
 *
 */
#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

/**
 *  @def CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
 *
 *  @brief
 *    Maximum number of simultaneously active exchange contexts.
 *
 */
#ifndef CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 16
#endif // CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS

/**
 *  @def CHIP_CONFIG_MAX_ACTIVE_CHANNELS
 *
 *  @brief
 *    Maximum number of simultaneously active channels
 */
#ifndef CHIP_CONFIG_MAX_ACTIVE_CHANNELS
#define CHIP_CONFIG_MAX_ACTIVE_CHANNELS 16
#endif // CHIP_CONFIG_MAX_ACTIVE_CHANNELS

/**
 *  @def CHIP_CONFIG_MAX_CHANNEL_HANDLES
 *
 *  @brief
 *    Maximum number of channel handles
 */
#ifndef CHIP_CONFIG_MAX_CHANNEL_HANDLES
#define CHIP_CONFIG_MAX_CHANNEL_HANDLES 32
#endif // CHIP_CONFIG_MAX_CHANNEL_HANDLES

/**
 *  @def CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS
 *
 *  @brief
 *    This is the default timeout for node addres resolve over mDNS
 *
 */
#ifndef CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS
#define CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS (5000)
#endif // CHIP_CONFIG_NODE_ADDRESS_RESOLVE_TIMEOUT_MSECS

/**
 *  @def CHIP_CONFIG_MCSP_RECEIVE_TABLE_SIZE
 *
 *  @brief
 *    Size of the receive table for message counter synchronization protocol
 *
 */
#ifndef CHIP_CONFIG_MCSP_RECEIVE_TABLE_SIZE
#define CHIP_CONFIG_MCSP_RECEIVE_TABLE_SIZE (CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS - 2)
#endif // CHIP_CONFIG_MCSP_RECEIVE_TABLE_SIZE

/**
 *  @def CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE
 *
 *  @brief
 *    Max number of messages behind message window can be accepted.
 *
 */
#ifndef CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE
#define CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE 32
#endif // CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE

/**
 *  @def CHIP_CONFIG_CONNECT_IP_ADDRS
 *
 *  @brief
 *    Maximum number of IP addresses tried when connecting to a
 *    hostname.
 *
 */
#ifndef CHIP_CONFIG_CONNECT_IP_ADDRS
#define CHIP_CONFIG_CONNECT_IP_ADDRS 4
#endif // CHIP_CONFIG_CONNECT_IP_ADDRS

/**
 *  @def CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE
 *
 *  @brief
 *    The default MTU size for an IPv6 datagram carrying UDP. This is useful
 *    for senders who want to send UDP chip messages that fit within a single
 *    IPv6 datagram.
 *
 *    1280 is the guaranteed minimum IPv6 MTU.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE
#define CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE 1280
#endif // CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE

/**
 *  @def CHIP_HEADER_RESERVE_SIZE
 *
 *  @brief
 *    The number of bytes to reserve in a network packet buffer to contain the
 *    chip message and exchange headers.
 *
 *    This number was calculated as follows:
 *
 *      chip Message Header:
 *
 *          2 -- Frame Length
 *          2 -- Message Header
 *          4 -- Message Id
 *          8 -- Source Node Id
 *          8 -- Destination Node Id
 *          2 -- Key Id
 *
 *      chip Exchange Header:
 *
 *          1 -- Application Version
 *          1 -- Message Type
 *          2 -- Exchange Id
 *          4 -- Profile Id
 *          4 -- Acknowleged Message Id
 *
 *    @note A number of these fields are optional or not presently used.
 *          So most headers will be considerably smaller than this.
 *
 */
#ifndef CHIP_HEADER_RESERVE_SIZE
#define CHIP_HEADER_RESERVE_SIZE 38
#endif // CHIP_HEADER_RESERVE_SIZE

/**
 *  @def CHIP_TRAILER_RESERVE_SIZE
 *
 *  @brief
 *    TODO
 *
 */
#ifndef CHIP_TRAILER_RESERVE_SIZE
#define CHIP_TRAILER_RESERVE_SIZE 20
#endif // CHIP_TRAILER_RESERVE_SIZE

/**
 *  @def CHIP_PORT
 *
 *  @brief
 *    chip TCP/UDP port for secured chip traffic.
 *
 */
#ifndef CHIP_PORT
#define CHIP_PORT 11097
#endif // CHIP_PORT

/**
 *  @def CHIP_UNSECURED_PORT
 *
 *  @brief
 *    chip TCP/UDP port for unsecured chip traffic.
 *
 */
#ifndef CHIP_UNSECURED_PORT
#define CHIP_UNSECURED_PORT 11096
#endif // CHIP_UNSECURED_PORT

/**
 *  @def CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
 *
 *  @brief
 *    Enable use of an ephemeral UDP source port for locally initiated chip exchanges.
 */
#ifndef CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
#define CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT 0
#endif // CHIP_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

/**
 *  @def CHIP_CONFIG_SECURITY_TEST_MODE
 *
 *  @brief
 *    Enable various features that make it easier to debug secure chip communication.
 *
 *  @note
 *    WARNING: This option makes it possible to circumvent basic chip security functionality,
 *    including message encryption. Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.
 */
#ifndef CHIP_CONFIG_SECURITY_TEST_MODE
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#endif // CHIP_CONFIG_SECURITY_TEST_MODE

/**
 *  @def CHIP_CONFIG_ENABLE_DNS_RESOLVER
 *
 *  @brief
 *    Enable support for resolving hostnames with a DNS resolver.
 */
#ifndef CHIP_CONFIG_ENABLE_DNS_RESOLVER
#define CHIP_CONFIG_ENABLE_DNS_RESOLVER (INET_CONFIG_ENABLE_DNS_RESOLVER)
#endif // CHIP_CONFIG_ENABLE_DNS_RESOLVER

/**
 *  @def CHIP_CONFIG_RESOLVE_IPADDR_LITERAL
 *
 *  @brief
 *    Enable support for resolving hostnames as literal IP addresses without a DNS resolver.
 *
 *    For historical reasons, the default is \c TRUE where \c CHIP_SYSTEM_CONFIG_USE_SOCKETS=1,
 *    and \c FALSE otherwise. The exception in the LwIP-only case was originally made to facilitate
 *    integration and change management with existing development lines. The default may
 *    change in the future to \c TRUE in all cases.
 */
#ifndef CHIP_CONFIG_RESOLVE_IPADDR_LITERAL
#define CHIP_CONFIG_RESOLVE_IPADDR_LITERAL (CHIP_SYSTEM_CONFIG_USE_SOCKETS)
#endif // CHIP_CONFIG_RESOLVE_IPADDR_LITERAL

/**
 *  @def CHIP_CONFIG_ENABLE_TARGETED_LISTEN
 *
 *  @brief
 *    Enable support for listening on particular addresses/interfaces.
 *
 *    This allows testing multiple instances of the chip stack
 *    running on a single host.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_TARGETED_LISTEN
#define CHIP_CONFIG_ENABLE_TARGETED_LISTEN (!CHIP_SYSTEM_CONFIG_USE_LWIP)
#endif // CHIP_CONFIG_ENABLE_TARGETED_LISTEN

/**
 *  @def CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
 *
 *  @brief
 *    Enable support for receiving TCP connections over an unsecured
 *    network layer (for example, from a device that is provisionally joined
 *    to a 6LowPAN network but does not possess the 802.15.4 network
 *    keys).
 *
 */
#ifndef CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
#define CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN 0
#endif // CHIP_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

/**
 *  @def CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES
 *
 *  @brief
 *    The maximum number of Relative Distinguished Name (RDN) attributes
 *    supported by the CHIP Certificate Distinguished Names (DN).
 *
 */
#ifndef CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES
#define CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES 5
#endif // CHIP_CONFIG_CERT_MAX_RDN_ATTRIBUTES

/**
 *  @def CHIP_CONFIG_DEBUG_CERT_VALIDATION
 *
 *  @brief
 *    Enable support for debugging output from certificate validation.
 *
 */
#ifndef CHIP_CONFIG_DEBUG_CERT_VALIDATION
#define CHIP_CONFIG_DEBUG_CERT_VALIDATION 1
#endif // CHIP_CONFIG_DEBUG_CERT_VALIDATION

/**
 *  @def CHIP_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID
 *
 *  @brief
 *    EC curve to be used to generate chip operational device certificate.
 *
 */
#ifndef CHIP_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID
#define CHIP_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID (chip::Profiles::Security::kChipCurveId_prime256v1)
#endif // CHIP_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID

/**
 *  @def CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE
 *
 *  @brief
 *    This is a packed valid date to be encoded in the chip
 *    operational device certificate. Any date before
 *    that date the certificate is considered invalid.
 *    The following functions can be used to calculate packed
 *    date/time: PackCertTime() and PackedCertTimeToDate().
 *    chip packed certificate dates are limited to representing
 *    dates that are on or after 2000/01/01.
 *    Mathematical expression to calculate packed date is:
 *        (((year - 2000) * 12 + (mon - 1)) * 31 + (day - 1))
 *    Currently encoded value corresponds to 2019/01/01.
 *
 */
#ifndef CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE
#define CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE 0x1B9C
#endif // CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE

/**
 *  @def CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER
 *
 *  @brief
 *    This is the valid date to be encoded in the chip
 *    operational device certificate. Any date after
 *    that date the certificate is considered invalid.
 *    The following functions can be used to calculate packed
 *    date/time: PackCertTime() and PackedCertTimeToDate().
 *    chip packed certificate dates are limited to representing
 *    dates that are on or after 2000/01/01.
 *    Mathematical expression to calculate packed date is:
 *        (((year - 2000) * 12 + (mon - 1)) * 31 + (day - 1))
 *    Currently encoded value corresponds to 2069/01/01.
 *
 */
#ifndef CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER
#define CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER 0x6444
#endif // CHIP_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER

/**
 *  @def CHIP_CONFIG_ENABLE_PASE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating PASE sessions.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_PASE_INITIATOR
#define CHIP_CONFIG_ENABLE_PASE_INITIATOR 1
#endif // CHIP_CONFIG_ENABLE_PASE_INITIATOR

/**
 *  @def CHIP_CONFIG_ENABLE_PASE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to PASE sessions initiated by
 *    other nodes.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_PASE_RESPONDER
#define CHIP_CONFIG_ENABLE_PASE_RESPONDER 1
#endif // CHIP_CONFIG_ENABLE_PASE_RESPONDER

/**
 *  @def CHIP_CONFIG_ENABLE_CASE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating CASE sessions.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_CASE_INITIATOR
#define CHIP_CONFIG_ENABLE_CASE_INITIATOR 1
#endif // CHIP_CONFIG_ENABLE_CASE_INITIATOR

/**
 *  @def CHIP_CONFIG_ENABLE_CASE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to CASE sessions initiated by other nodes.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_CASE_RESPONDER
#define CHIP_CONFIG_ENABLE_CASE_RESPONDER 1
#endif // CHIP_CONFIG_ENABLE_CASE_RESPONDER

/**
 *  @def CHIP_CONFIG_SUPPORT_CASE_CONFIG1
 *
 *  @brief
 *    Enable use of CASE protocol configuration 1.
 *
 *  @note CASE config 1 uses SHA-1 for message signatures, which is deprecated.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_CASE_CONFIG1
#define CHIP_CONFIG_SUPPORT_CASE_CONFIG1 1
#endif // CHIP_CONFIG_SUPPORT_CASE_CONFIG1

#ifndef CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER
#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER "GlobalMCTR"
#endif // CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER

/**
 *  @def CHIP_CONFIG_DEFAULT_CASE_CURVE_ID
 *
 *  @brief
 *    Default ECDH curve to be used when initiating a CASE session, if not overridden by the application.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_CASE_CURVE_ID
#if CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#define CHIP_CONFIG_DEFAULT_CASE_CURVE_ID (chip::Profiles::Security::kChipCurveId_secp224r1)
#elif CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define CHIP_CONFIG_DEFAULT_CASE_CURVE_ID (chip::Profiles::Security::kChipCurveId_prime256v1)
#elif CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#define CHIP_CONFIG_DEFAULT_CASE_CURVE_ID (chip::Profiles::Security::kChipCurveId_prime192v1)
#else
#define CHIP_CONFIG_DEFAULT_CASE_CURVE_ID (chip::Profiles::Security::kChipCurveId_secp160r1)
#endif
#endif // CHIP_CONFIG_DEFAULT_CASE_CURVE_ID

/**
 *  @def CHIP_CONFIG_DEFAULT_CASE_ALLOWED_CURVES
 *
 *  @brief
 *    Default set of ECDH curves allowed to be used in a CASE session (initiating or responding), if not overridden by the
 * application.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_CASE_ALLOWED_CURVES
#if CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1 || CHIP_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define CHIP_CONFIG_DEFAULT_CASE_ALLOWED_CURVES                                                                                    \
    (chip::Profiles::Security::kChipCurveSet_secp224r1 | chip::Profiles::Security::kChipCurveSet_prime256v1)
#else
#define CHIP_CONFIG_DEFAULT_CASE_ALLOWED_CURVES (chip::Profiles::Security::kChipCurveSet_All)
#endif
#endif // CHIP_CONFIG_DEFAULT_CASE_ALLOWED_CURVES

/**
 * @def CHIP_CONFIG_LEGACY_CASE_AUTH_DELEGATE
 *
 * @brief
 *   Enable use of the legacy chipCASEAuthDelegate interface.
 */
#ifndef CHIP_CONFIG_LEGACY_CASE_AUTH_DELEGATE
#define CHIP_CONFIG_LEGACY_CASE_AUTH_DELEGATE 1
#endif

/**
 *  @def CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES
 *
 *  @brief
 *    The maximum number of end nodes simultaneously supported
 *    for all active shared sessions.
 *
 */
#ifndef CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES
#define CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES 10
#endif // CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES

/**
 *  @def CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION
 *
 *  @brief
 *    The maximum number of end nodes simultaneously supported
 *    per active shared session.
 *
 */
#ifndef CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION
#define CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION 10
#endif // CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION

/**
 *  @def CHIP_CONFIG_ENABLE_TAKE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating TAKE sessions.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_TAKE_INITIATOR
#define CHIP_CONFIG_ENABLE_TAKE_INITIATOR 0
#endif // CHIP_CONFIG_ENABLE_TAKE_INITIATOR

/**
 *  @def CHIP_CONFIG_ENABLE_TAKE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to TAKE sessions initiated by other nodes.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_TAKE_RESPONDER
#define CHIP_CONFIG_ENABLE_TAKE_RESPONDER 0
#endif // CHIP_CONFIG_ENABLE_TAKE_RESPONDER

/**
 *  @def CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR
 *
 *  @brief
 *    Enable support for initiating key export request.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR
#define CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR 1
#endif // CHIP_CONFIG_ENABLE_KEY_EXPORT_INITIATOR

/**
 *  @def CHIP_CONFIG_ENABLE_KEY_EXPORT_RESPONDER
 *
 *  @brief
 *    Enable support for responding to key export request initiated by other nodes.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_KEY_EXPORT_RESPONDER
#define CHIP_CONFIG_ENABLE_KEY_EXPORT_RESPONDER 1
#endif // CHIP_CONFIG_ENABLE_KEY_EXPORT_RESPONDER

/**
 * @def CHIP_CONFIG_LEGACY_KEY_EXPORT_DELEGATE
 *
 * @brief
 *   Enable use of the legacy chipKeyExportDelegate interface.
 */
#ifndef CHIP_CONFIG_LEGACY_KEY_EXPORT_DELEGATE
#define CHIP_CONFIG_LEGACY_KEY_EXPORT_DELEGATE 1
#endif

/**
 *  @def CHIP_CONFIG_REQUIRE_AUTH
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests via an
 *    authenticated session.
 *
 *    This broadly controls whether or not a number of chip servers
 *    require client requests to be sent via an authenticated session
 *    and provides a default configuration value to these related
 *    definitions:
 *
 *      * #CHIP_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
 *      * #CHIP_CONFIG_REQUIRE_AUTH_FABRIC_PROV
 *      * #CHIP_CONFIG_REQUIRE_AUTH_NETWORK_PROV
 *      * #CHIP_CONFIG_REQUIRE_AUTH_SERVICE_PROV
 *
 *    @note These configurations shall be deasserted for development
 *          and testing purposes only. No chip-enabled device shall
 *          be certified without these asserted.
 *
 */
#ifndef CHIP_CONFIG_REQUIRE_AUTH
#define CHIP_CONFIG_REQUIRE_AUTH 1
#endif // CHIP_CONFIG_REQUIRE_AUTH

/**
 *  @def CHIP_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    chip Device Control server via an authenticated session. See
 *    also #CHIP_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No chip-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef CHIP_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
#define CHIP_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL CHIP_CONFIG_REQUIRE_AUTH
#endif // CHIP_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL

/**
 *  @def CHIP_CONFIG_REQUIRE_AUTH_FABRIC_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    chip Fabric Provisioning server via an authenticated
 *    session. See also #CHIP_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No chip-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef CHIP_CONFIG_REQUIRE_AUTH_FABRIC_PROV
#define CHIP_CONFIG_REQUIRE_AUTH_FABRIC_PROV CHIP_CONFIG_REQUIRE_AUTH
#endif // CHIP_CONFIG_REQUIRE_AUTH_FABRIC_PROV

/**
 *  @def CHIP_CONFIG_REQUIRE_AUTH_NETWORK_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    chip Network Provisioning server via an authenticated
 *    session. See also #CHIP_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No chip-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef CHIP_CONFIG_REQUIRE_AUTH_NETWORK_PROV
#define CHIP_CONFIG_REQUIRE_AUTH_NETWORK_PROV CHIP_CONFIG_REQUIRE_AUTH
#endif // CHIP_CONFIG_REQUIRE_AUTH_NETWORK_PROV

/**
 *  @def CHIP_CONFIG_REQUIRE_AUTH_SERVICE_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    chip Service Provisioning server via an authenticated
 *    session. See also #CHIP_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No chip-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef CHIP_CONFIG_REQUIRE_AUTH_SERVICE_PROV
#define CHIP_CONFIG_REQUIRE_AUTH_SERVICE_PROV CHIP_CONFIG_REQUIRE_AUTH
#endif // CHIP_CONFIG_REQUIRE_AUTH_SERVICE_PROV

/**
 *  @def CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT
 *
 *  @brief
 *    Enable (1) or disable (0) support for the handling of chip
 *    Provisioning Bundles.
 *
 *    chip Provisioning Bundles are a chip TLV payload containing
 *    the chip certificate, corresponding private key, and pairing
 *    code / entry key that a chip device would have otherwise
 *    received at its time of manufacture.
 *
 *    Enable this if your family of device needs to support in-field
 *    provisioning (IFP). IFP for chip devices is neither generally
 *    supported nor recommended.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT
#define CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT 1
#endif // CHIP_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT

/**
 *  @def CHIP_ERROR_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    chip::Logging::LogCategory::kLogCategory_Error category.
 *
 */
#ifndef CHIP_ERROR_LOGGING
#define CHIP_ERROR_LOGGING 1
#endif // CHIP_ERROR_LOGGING

/**
 *  @def CHIP_PROGRESS_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    chip::Logging::LogCategory::kLogCategory_Progress category.
 *
 */
#ifndef CHIP_PROGRESS_LOGGING
#define CHIP_PROGRESS_LOGGING 1
#endif // CHIP_PROGRESS_LOGGING

/**
 *  @def CHIP_DETAIL_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    chip::Logging::kLogCategory_Detail category.
 *
 */
#ifndef CHIP_DETAIL_LOGGING
#define CHIP_DETAIL_LOGGING 1
#endif // CHIP_DETAIL_LOGGING

/**
 * CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE
 *
 * The maximum size (in bytes) of a log message
 */
#ifndef CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE
#define CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE 256
#endif

/**
 *  @def CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of errors at function exit via the
 *    ChipLogFunctError() macro.
 */
#ifndef CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
#define CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 0
#endif // CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING

/**
 *  @def CHIP_CONFIG_ENABLE_CONDITION_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of failed conditions via the
 *    ChipLogIfFalse() macro.
 */
#ifndef CHIP_CONFIG_ENABLE_CONDITION_LOGGING
#define CHIP_CONFIG_ENABLE_CONDITION_LOGGING 0
#endif // CHIP_CONFIG_ENABLE_CONDITION_LOGGING

/**
 *  @def CHIP_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS
 *
 *  @brief
 *    This is the default timeout for the connect call to the
 *    directory server to wait for success or being notified
 *    of an error.
 *
 */
#ifndef CHIP_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS
#define CHIP_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS (10000)
#endif // CHIP_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS

/**
 *  @def CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT
 *
 *  @brief
 *    The maximum amount of time, in milliseconds, that an idle inbound
 *    chip connection will be allowed to exist before being closed.
 *
 *    This is a default value that can be overridden at runtime by the
 *    application.
 *
 *    A value of 0 disables automatic closing of idle connections.
 *
 */
#ifndef CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT
#define CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT 15000
#endif // CHIP_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT

/**
 *  @def CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT
 *
 *  @brief
 *    The amount of time (in milliseconds) which a peer is given
 *    to respond to a message counter synchronization request.
 *    Depending on when the request is sent, peers may
 *    actually have up to twice this time.
 *
 */
#ifndef CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT
#define CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT 2000
#endif // CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT

/**
 *  @def CHIP_CONFIG_TEST
 *
 *  @brief
 *    If asserted (1), enable APIs that help implement
 *    unit and integration tests.
 *
 */
#ifndef CHIP_CONFIG_TEST
#define CHIP_CONFIG_TEST 0
#endif // CHIP_CONFIG_TEST

/**
 *  @def CHIP_CONFIG_SHORT_ERROR_STR
 *
 *  @brief
 *    If asserted (1), produce shorter error strings that only carry a
 *    minimum of information.
 *
 */
#ifndef CHIP_CONFIG_SHORT_ERROR_STR
#define CHIP_CONFIG_SHORT_ERROR_STR 0
#endif // CHIP_CONFIG_SHORT_ERROR_STR

/**
 *  @def CHIP_CONFIG_ERROR_STR_SIZE
 *
 *  @brief
 *    This defines the size of the buffer to store a formatted error string.
 *    If the formatting of an error string exceeds this size it will be truncated.
 *
 *    The default size varies based on the CHIP_CONFIG_SHORT_ERROR_STR option.
 *
 *    When CHIP_CONFIG_SHORT_ERROR_STR is 0, a large default buffer size is used
 *    to accommodate descriptive text summarizing the cause of the error. E.g.:
 *
 *         "chip Error 4047 (0x00000FCF): Invalid Argument"
 *
 *    When CHIP_CONFIG_SHORT_ERROR_STR is 1, the buffer size is set to accommodate
 *    a minimal error string consisting of a 10 character subsystem name followed
 *    by an 8 character error number, plus boilerplate. E.g.:
 *
 *         "Error chip:0x00000FCF"
 *
 */
#ifndef CHIP_CONFIG_ERROR_STR_SIZE
#if CHIP_CONFIG_SHORT_ERROR_STR
#define CHIP_CONFIG_ERROR_STR_SIZE (5 + 1 + 10 + 3 + 8 + 1)
#else // CHIP_CONFIG_SHORT_ERROR_STR
#define CHIP_CONFIG_ERROR_STR_SIZE 256
#endif // CHIP_CONFIG_SHORT_ERROR_STR
#endif // CHIP_CONFIG_ERROR_STR_SIZE

/**
 *  @def CHIP_CONFIG_CUSTOM_ERROR_FORMATTER
 *
 *  @brief
 *    If asserted (1), suppress definition of the standard error formatting function
 *    (#FormatError()) allowing an application-specific implementation to be used.
 *
 */
#ifndef CHIP_CONFIG_CUSTOM_ERROR_FORMATTER
#define CHIP_CONFIG_CUSTOM_ERROR_FORMATTER 0
#endif // CHIP_CONFIG_CUSTOM_ERROR_FORMATTER

/**
 *  @def CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT
 *
 *  @brief
 *    The printf-style format string used to format error values.
 *
 *  On some platforms, the structure of error values makes them more convenient to
 *  read in either hex or decimal format.  This option can be used to override
 *  the default hex format.
 *
 *  Note that this option only affects short-form error strings (i.e. when
 *  CHIP_CONFIG_SHORT_ERROR_STR == 1).  Long form error strings always show both hex
 *  and decimal values
 */
#ifndef CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT
#define CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT "0x%08" PRIX32
#endif // CHIP_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT

/**
 *  @def CHIP_CONFIG_BLE_PKT_RESERVED_SIZE
 *
 *  @brief
 *    The number of bytes that chip should reserve at the front of
 *    every outgoing BLE packet for the sake of the underlying BLE
 *    stack.
 *
 */
#ifndef CHIP_CONFIG_BLE_PKT_RESERVED_SIZE
#define CHIP_CONFIG_BLE_PKT_RESERVED_SIZE 0
#endif // CHIP_CONFIG_BLE_PKT_RESERVED_SIZE

/**
 *  @def CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS
 *
 *  @brief
 *    Enable (1) or disable (0) support for utility functions for
 *    decoding and outputing information related to chip security.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS
#define CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS 1
#endif // CHIP_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS

/**
 *  @def CHIP_CONFIG_IsPlatformErrorNonCritical(CODE)
 *
 *  This macro checks if a platform generated error is critical and
 *  needs to be reported to the application/caller. The criticality
 *  of an error (in the context of that platform) is determined by how
 *  it impacts the logic flow, i.e., whether or not the current flow
 *  can continue despite the error or it needs to be reported back
 *  resulting in a potential stoppage.
 *
 *  @note
 *  This is a default set of platform errors which are configured as
 *  non-critical from the context of that platform. Any new error that
 *  the platforms deem as non-critical could be added by overriding
 *  this default macro definition after careful thought towards its
 *  implication in the logic flow in that platform.
 *
 *  @param[in]    CODE    The #CHIP_ERROR being checked for criticality.
 *
 *  @return    true if the error is non-critical; false otherwise.
 *
 */
#ifndef CHIP_CONFIG_IsPlatformErrorNonCritical
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define _CHIP_CONFIG_IsPlatformLwIPErrorNonCritical(CODE)                                                                          \
    ((CODE) == chip::System::MapErrorLwIP(ERR_RTE) || (CODE) == chip::System::MapErrorLwIP(ERR_MEM))
#else // !CHIP_SYSTEM_CONFIG_USE_LWIP
#define _CHIP_CONFIG_IsPlatformLwIPErrorNonCritical(CODE) 0
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#define _CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE)                                                                         \
    ((CODE) == chip::System::MapErrorPOSIX(EHOSTUNREACH) || (CODE) == chip::System::MapErrorPOSIX(ENETUNREACH) ||                  \
     (CODE) == chip::System::MapErrorPOSIX(EADDRNOTAVAIL) || (CODE) == chip::System::MapErrorPOSIX(EPIPE))
#else // !CHIP_SYSTEM_CONFIG_USE_SOCKETS
#define _CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE) 0
#endif // !CHIP_SYSTEM_CONFIG_USE_SOCKETS

#define CHIP_CONFIG_IsPlatformErrorNonCritical(CODE)                                                                               \
    (_CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE) || _CHIP_CONFIG_IsPlatformLwIPErrorNonCritical(CODE))
#endif // CHIP_CONFIG_IsPlatformErrorNonCritical

/**
 *  @def CHIP_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS
 *
 *  @brief
 *    Enable (1) or disable (0) replacing math functions
 *    which may not be available in the standard/intrinsic library,
 *    and hence require special support from the platform.
 *
 */
#ifndef CHIP_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS
#define CHIP_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS 0
#endif // CHIP_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS

/**
 * @def CHIP_CONFIG_SERIALIZATION_USE_MALLOC
 *
 * @brief If turned on, then schema event serialization and
 *   deserialization will use the stdlib implementations of malloc,
 *   free, and realloc by default (if no other implementations have
 *   been provided).  We will fail at compile time if the stdlib
 *   implementations are not present.
 */
#ifndef CHIP_CONFIG_SERIALIZATION_USE_MALLOC
#define CHIP_CONFIG_SERIALIZATION_USE_MALLOC 0
#endif

/**
 * @def CHIP_CONFIG_SERIALIZATION_DEBUG_LOGGING
 *
 * @brief Enable debug logging for the serialization/deserialization APIs.
 */
#ifndef CHIP_CONFIG_SERIALIZATION_DEBUG_LOGGING
#define CHIP_CONFIG_SERIALIZATION_DEBUG_LOGGING 0
#endif

/**
 * @def CHIP_CONFIG_SERIALIZATION_LOG_FLOATS
 *
 * @brief Enable debug logging of floats and doubles for the
 *   serialization/deserialization APIs.  Not all platforms
 *   support these types, and may not compile if there are
 *   any references to them.  Only matters if
 *   CHIP_CONFIG_SERIALIZATION_DEBUG_LOGGING is enabled.
 */
#ifndef CHIP_CONFIG_SERIALIZATION_LOG_FLOATS
#define CHIP_CONFIG_SERIALIZATION_LOG_FLOATS 1
#endif

/**
 * @def CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE
 *
 * @brief
 *   The data type used to represent the key of a persistedly-stored
 *   key/value pair.
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE
#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#endif

/**
 * @def CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY
 *
 * @brief
 *   Lifetime Counter Key.
 */
#ifndef CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY
#define CHIP_CONFIG_LIFETIIME_PERSISTED_COUNTER_KEY "life-count"
#endif

/**
 *  @def CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID
 *
 *  @brief
 *    The group key message counter persisted storage Id.
 *
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID "EncMsgCntr"
#endif // CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID

/**
 *  @def CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH
 *
 *  @brief
 *    The group key message counter persisted storage epoch.
 *
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH
#define CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH 0x1000
#endif // CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH

/**
 * @def CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *
 * @brief The maximum length of the key in a key/value pair
 *   stored in the platform's persistent storage.
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 16
#endif

/**
 * @def CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH
 *
 * @brief The maximum length of the value in a key/value pair
 *   stored in the platform's persistent storage.
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH 256
#endif

/**
 * @def CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
 *
 * @brief Enable debug logging for the PersistedCounter API.
 */
#ifndef CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
#define CHIP_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING 0
#endif

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
 *
 * @brief Enable verbose debug logging for the EventLogging API.
 * This setting is incompatible with platforms that route console
 * logs into event logging, as it would result in circular logic.
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
#define CHIP_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS 1
#endif

/**
 * @def CHIP_CONFIG_ENABLE_ARG_PARSER
 *
 * @brief Enable support functions for parsing command-line arguments
 */
#ifndef CHIP_CONFIG_ENABLE_ARG_PARSER
#define CHIP_CONFIG_ENABLE_ARG_PARSER 0
#endif

/**
 * @def CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
 *
 * @brief Enable santiy checking of command-line argument definitions.
 */
#ifndef CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
#define CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK 1
#endif

/**
 * @def CHIP_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT
 *
 * @brief
 *    The amount of time (in milliseconds) which the service is given
 *    to respond to a pair device to account request.
 */
#ifndef CHIP_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT
#define CHIP_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT 60000
#endif

/**
 *  @def CHIP_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE
 *
 *  @brief
 *    Enable (1) or disable (0) support for the depricated
 *    version of AddNetwork() message in the Network Provisioning
 *    profile.
 *    This option should be enabled to support pairing with CHIP
 *    legacy devices that don't have latest SW.
 *
 */
#ifndef CHIP_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE
#define CHIP_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE 1
#endif // CHIP_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE

/**
 *  @def CHIP_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE
 *
 *  @brief
 *    Enable (1) or disable (0) the exclusive use of the depricated
 *    version of AddNetwork() message in the Network Provisioning
 *    profile.
 *    This option should be enabled when exclusively pairing with CHIP
 *    legacy devices that don't have latest SW.
 *    This option requires that
 *    CHIP_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE is enabled.
 *
 */
#ifndef CHIP_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE
#define CHIP_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE 0
#endif // CHIP_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE

/**
 * @def CHIP_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN
 *
 * @brief Enable the Service Provisioning profile message
 * for notification of successful in-field joining of the
 * chip fabric.
 */
#ifndef CHIP_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN
#define CHIP_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN 0
#endif // CHIP_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN

/**
 * @def CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE
 *
 * @brief Define the size of the pool used for tracking CHIP
 * Peer connections. This defines maximum number of concurrent
 * device connections across all supported transports.
 */
#ifndef CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE
#define CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE 16
#endif // CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE

/**
 * @def CHIP_PEER_CONNECTION_TIMEOUT_MS
 *
 * @brief After what period of inactivity is a peer connection considered
 * expired.
 */
#ifndef CHIP_PEER_CONNECTION_TIMEOUT_MS
#define CHIP_PEER_CONNECTION_TIMEOUT_MS 120000
#endif // CHIP_PEER_CONNECTION_TIMEOUT_MS

/**
 * @def CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS
 *
 * @brief How frequent are peer connections checked for timeouts.
 */
#ifndef CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS
#define CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS 5000
#endif // CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS

/**
 *  @def CHIP_CONFIG_MAX_BINDINGS
 *
 *  @brief
 *    Maximum number of simultaneously active bindings per WeaveExchangeManager
 *    The new single source TimeSync client takes one binding.
 *    Every WDM one-way subscription takes one binding. Mutual subscription counts as two one-way subscriptions.
 *    A reserved slot is needed to take an incoming subscription request.
 *    For a device with 2 mutual subscriptions, and one single source time sync client, it needs 2 x 2 + 1 = 5 bindings at least.
 *    At least six is needed if it still wants to take new WDM subscriptions under this load.
 */
#ifndef CHIP_CONFIG_MAX_BINDINGS
#define CHIP_CONFIG_MAX_BINDINGS 6
#endif // CHIP_CONFIG_MAX_BINDINGS

/**
 *  @def CHIP_CONFIG_MAX_DEVICE_ADMINS
 *
 *  @brief
 *    Maximum number of administrators that can provision the device. Each admin
 *    can provision the device with their unique operational credentials and manage
 *    their access control lists.
 */
#ifndef CHIP_CONFIG_MAX_DEVICE_ADMINS
#define CHIP_CONFIG_MAX_DEVICE_ADMINS 16
#endif // CHIP_CONFIG_MAX_DEVICE_ADMINS

/**
 * @def CHIP_NON_PRODUCTION_MARKER
 *
 * @brief Defines the name of a mark symbol whose presence signals that the chip code
 * includes development/testing features that should never be used in production contexts.
 */
#ifndef CHIP_NON_PRODUCTION_MARKER
#if (CHIP_CONFIG_SECURITY_TEST_MODE || CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY ||                                               \
     CHIP_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY || (!CHIP_CONFIG_REQUIRE_AUTH) || CHIP_FUZZING_ENABLED)
#define CHIP_NON_PRODUCTION_MARKER WARNING__DO_NOT_SHIP__CONTAINS_NON_PRODUCTION_CHIP_CODE
#endif
#endif

#ifdef CHIP_NON_PRODUCTION_MARKER
extern const char CHIP_NON_PRODUCTION_MARKER[];
#endif

/**
 * @def CHIP_COMMISSIONING_HINT_TABLE
 *
 * @brief Defines the set of "pairing hint" values that can be set in
 * the PH key in commissionable node discovery response.
 */
#ifndef CHIP_COMMISSIONING_HINT_TABLE
#define CHIP_COMMISSIONING_HINT_TABLE
#define CHIP_COMMISSIONING_HINT_INDEX_POWER_CYCLE 0
#define CHIP_COMMISSIONING_HINT_INDEX_MANUFACTURER_URL 1
#define CHIP_COMMISSIONING_HINT_INDEX_SEE_ADMINISTRATOR_UX 2
#define CHIP_COMMISSIONING_HINT_INDEX_SEE_SETTINGS_MENU 3
#define CHIP_COMMISSIONING_HINT_INDEX_CUSTOM_INSTRUCTION 4
#define CHIP_COMMISSIONING_HINT_INDEX_SEE_MANUAL 5
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET 6
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET_WITH_POWER 7
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET_SECONDS 8
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET_UNTIL_BLINK 9
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET_SECONDS_WITH_POWER 10
#define CHIP_COMMISSIONING_HINT_INDEX_PRESS_RESET_UNTIL_BLINK_WITH_POWER 11
#endif
