/*
 *
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
 *    All rights reserved.
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
 *      for Nest Weave.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific WeaveProjectConfig.h header and then assert
 *      HAVE_WEAVEPROJECTCONFIG_H via the package configuration tool
 *      via --with-weave-project-includes=DIR where DIR is the
 *      directory that contains the header.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#ifndef WEAVE_CONFIG_H_
#define WEAVE_CONFIG_H_

#include <SystemLayer/SystemConfig.h>

/* COMING SOON: making the INET Layer optional entails making this inclusion optional. */
#include <InetLayer/InetConfig.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL <= 0
#error "Weave SDK requires INET_TCP_IDLE_CHECK_INTERVAL > 0"
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL <= 0

/* Include a project-specific configuration file, if defined.
 *
 * An application or module that incorporates Weave can define a project configuration
 * file to override standard Weave configuration with application-specific values.
 * The WeaveProjectConfig.h file is typically located outside the OpenWeave source tree,
 * alongside the source code for the application.
 */
#ifdef WEAVE_PROJECT_CONFIG_INCLUDE
#include WEAVE_PROJECT_CONFIG_INCLUDE
#endif

/* Include a platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard Weave configuration
 * that are specific to the platform or OS on which Weave is running.  It is typically
 * provided as apart of an adaptation layer that adapts OpenWeave to the target
 * environment.  This adaptation layer may be included in the OpenWeave source tree
 * itself or implemented externally.
 */
#ifdef WEAVE_PLATFORM_CONFIG_INCLUDE
#include WEAVE_PLATFORM_CONFIG_INCLUDE
#endif

// clang-format off

/**
 *  @def WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
 *
 *  @brief
 *    This boolean configuration option is (1) if the obsolescent interfaces
 *    of the Nest Weave layer are still available for transitional purposes.
 *
 */
#ifndef WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#define WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES         0
#endif //  WEAVE_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

// Profile-specific Configuration Headers

#include "WeaveBDXConfig.h"

#include "WeaveDMConfig.h"

#include "WeaveTimeConfig.h"

#include "WeaveTunnelConfig.h"

#include "WeaveEventLoggingConfig.h"

#include "WeaveWRMPConfig.h"

/**
 *  @def WEAVE_CONFIG_ERROR_TYPE
 *
 *  @brief
 *    This defines the data type used to represent errors for Weave.
 *
 */
#ifndef WEAVE_CONFIG_ERROR_TYPE
#include <stdint.h>

#define WEAVE_CONFIG_ERROR_TYPE                             int32_t
#endif // WEAVE_CONFIG_ERROR_TYPE

/**
 *  @def WEAVE_CONFIG_NO_ERROR
 *
 *  @brief
 *    This defines the Weave error code for no error or success.
 *
 */
#ifndef WEAVE_CONFIG_NO_ERROR
#define WEAVE_CONFIG_NO_ERROR                               0
#endif // WEAVE_CONFIG_NO_ERROR

/**
 *  @def WEAVE_CONFIG_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum Weave error number range.
 *
 */
#ifndef WEAVE_CONFIG_ERROR_MIN
#define WEAVE_CONFIG_ERROR_MIN                              4000
#endif // WEAVE_CONFIG_ERROR_MIN

/**
 *  @def WEAVE_CONFIG_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum Weave error number range.
 *
 */
#ifndef WEAVE_CONFIG_ERROR_MAX
#define WEAVE_CONFIG_ERROR_MAX                              4999
#endif // WEAVE_CONFIG_ERROR_MAX

/**
 *  @def _WEAVE_CONFIG_ERROR
 *
 *  @brief
 *    This defines a mapping function for Weave errors that allows
 *    mapping such errors into a platform- or system-specific manner.
 *
 */
#ifndef _WEAVE_CONFIG_ERROR
#define _WEAVE_CONFIG_ERROR(e)                              (WEAVE_ERROR_MIN + (e))
#endif // _WEAVE_CONFIG_ERROR

/**
 *  @def WEAVE_CONFIG_USE_OPENSSL_ECC
 *
 *  @brief
 *    Use the OpenSSL implementation of the elliptic curve primitives
 *    for Weave communication.
 *
 *    Note that this option is mutually exclusive with
 *    #WEAVE_CONFIG_USE_MICRO_ECC.
 */
#ifndef WEAVE_CONFIG_USE_OPENSSL_ECC
#define WEAVE_CONFIG_USE_OPENSSL_ECC                        1
#endif // WEAVE_CONFIG_USE_OPENSSL_ECC

/**
 *  @def WEAVE_CONFIG_USE_MICRO_ECC
 *
 *  @brief
 *    Use the Micro ECC implementation of the elliptic curve primitives
 *    for Weave communication.
 *
 *    Note that this option is mutually exclusive with
 *    #WEAVE_CONFIG_USE_OPENSSL_ECC.
 *
 */
#ifndef WEAVE_CONFIG_USE_MICRO_ECC
#define WEAVE_CONFIG_USE_MICRO_ECC                          0
#endif // WEAVE_CONFIG_USE_MICRO_ECC

#if WEAVE_CONFIG_USE_MICRO_ECC && WEAVE_CONFIG_USE_OPENSSL_ECC
#error "Please assert one of either WEAVE_CONFIG_USE_MICRO_ECC or WEAVE_CONFIG_USE_OPENSSL_ECC, but not both."
#endif // WEAVE_CONFIG_USE_MICRO_ECC && WEAVE_CONFIG_USE_OPENSSL_ECC

/**
 *  @name Weave Elliptic Curve Security Configuration
 *
 *  @brief
 *    The following definitions enable one or more of four potential
 *    elliptic curves:
 *
 *      * #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
 *      * #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
 *      * #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
 *      * #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp160r1 elliptic curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
#define WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1       0
#endif // WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1

/**
 *  @def WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp192r1 elliptic curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#define WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1       1
#endif // WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1

/**
 *  @def WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp224r1 / National
 *    Institute of Standards (NIST) P-224 elliptic curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#define WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1       1
#endif // WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1

/**
 *  @def WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
 *
 *  @brief
 *    Enable (1) or disable (0) support for the Standards for
 *    Efficient Cryptography Group (SECG) secp256r1 / American
 *    National Standards Institute (ANSI) prime256v1 / National
 *    Institute of Standards (NIST) P-256 elliptic curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1       1
#endif // WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1

/**
 *  @}
 */

/**
 *  @name Weave Password Authenticated Session Establishment (PASE) Configuration
 *
 *  @brief
 *    The following definitions define the configurations supported
 *    for Weave's Password Authenticated Session Establishment (PASE)
 *    protocol.
 *
 *    This protocol is used primarily for establishing a secure
 *    session for provisioning. Weave supports the following PASE
 *    configurations:
 *
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG1
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG2
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG3
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG4
 *    * #WEAVE_CONFIG_SUPPORT_PASE_CONFIG5
 *
 *    which are summarized in the table below:
 *
 *    | Configuration  |  J-PAKE Style   |   Curve   |  Test Only  | Notes                                |
 *    | :------------: | :-------------- | :-------: | :---------: | :----------------------------------- |
 *    | 0              | -               | -         | Y           | Test-only                            |
 *    | 1              | Finite Field    | -         | N           | Original Weave default configuration |
 *    | 2              | Elliptic Curve  | secp160r1 | N           |                                      |
 *    | 3              | Elliptic Curve  | secp192r1 | N           |                                      |
 *    | 4              | Elliptic Curve  | secp224r1 | N           | Future Weave default configuration   |
 *    | 5              | Elliptic Curve  | secp256r1 | N           |                                      |
 *
 *  @{
 *
 */

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
 *
 *  @brief
 *    This Weave PASE configuration does not use the J-PAKE algorithm
 *    and sends deterministic messages over the communications
 *    channel. The size and structure of the messages are similar to
 *    #WEAVE_CONFIG_SUPPORT_PASE_CONFIG5.
 *
 *  @note The results of this configuration are insecure because the
 *        computational overhead of the cryptography has largely been
 *        disabled since the focus of this configuration is testing
 *        the overall PASE protocol exchange, independently of the
 *        cryptography.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY         0
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG1
 *
 *  @brief
 *    This Weave PASE configuration uses Finite Field J-PAKE and is
 *    the original, default Weave PASE configuration.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG1
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG1                   1
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG1

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG2
 *
 *  @brief
 *    This Weave PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp160r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1)
 *          should also be enabled.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG2
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG2                   0
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG2

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG3
 *
 *  @brief
 *    This Weave PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp192r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1)
 *          should also be enabled.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG3
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG3                   0
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG3

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG4
 *
 *  @brief
 *    This Weave PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp224r1 curve and will be the new, default Weave PASE
 *    configuration.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1)
 *          should also be enabled.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG4
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG4                   1
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG4

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASE_CONFIG5
 *
 *  @brief
 *    This Weave PASE configuration uses Elliptic Curve J-PAKE with a
 *    SECG secp256r1 curve.
 *
 *    @note When this PASE configuration is enabled, the corresponding
 *          elliptic curve (i.e. #WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1)
 *          should also be enabled.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASE_CONFIG5
#define WEAVE_CONFIG_SUPPORT_PASE_CONFIG5                   0
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG5

/**
 *  @}
 */

#if WEAVE_CONFIG_SUPPORT_PASE_CONFIG2 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1
#error "Please assert WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1 when WEAVE_CONFIG_SUPPORT_PASE_CONFIG2 is asserted"
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG2 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP160R1

#if WEAVE_CONFIG_SUPPORT_PASE_CONFIG3 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#error "Please assert WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1 when WEAVE_CONFIG_SUPPORT_PASE_CONFIG3 is asserted"
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG3 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1

#if WEAVE_CONFIG_SUPPORT_PASE_CONFIG4 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#error "Please assert WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1 when WEAVE_CONFIG_SUPPORT_PASE_CONFIG4 is asserted"
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG4 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1

#if WEAVE_CONFIG_SUPPORT_PASE_CONFIG5 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#error "Please assert WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1 when WEAVE_CONFIG_SUPPORT_PASE_CONFIG5 is asserted"
#endif // WEAVE_CONFIG_SUPPORT_PASE_CONFIG5 && !WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1

/**
 *  @def WEAVE_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT
 *
 *  @brief
 *    Align payload on 4-byte boundary for PASE messages.
 *    Currently, payload alignment is required only when micro-ecc
 *    library is used and it is compiled with ARM assembly.
 *    If implementation guarantees that payload is always 4-byte
 *    aligned this option should stay deasserted to save code size.
 *
 */
#ifndef WEAVE_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT
#if WEAVE_CONFIG_USE_MICRO_ECC
#define WEAVE_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT         1
#else
#define WEAVE_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT         0
#endif // WEAVE_CONFIG_USE_MICRO_ECC
#endif // WEAVE_CONFIG_PASE_MESSAGE_PAYLOAD_ALIGNMENT

/**
 *  @def WEAVE_CONFIG_PASE_RATE_LIMITER_TIMEOUT
 *
 *  @brief
 *    The amount of time (in milliseconds) in which the Security Manager
 *    is allowed to have maximum #WEAVE_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
 *    counted PASE attempts.
 *
 */
#ifndef WEAVE_CONFIG_PASE_RATE_LIMITER_TIMEOUT
#define WEAVE_CONFIG_PASE_RATE_LIMITER_TIMEOUT              15000
#endif // WEAVE_CONFIG_PASE_RATE_LIMITER_TIMEOUT

/**
 *  @def WEAVE_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
 *
 *  @brief
 *    The maximum number of PASE attempts after which the
 *    next PASE session establishment attempt will be allowed
 *    only after #WEAVE_CONFIG_PASE_RATE_LIMITER_TIMEOUT expires.
 *     * For PASE negotiations with key confirmation option enabled:
 *       only attempts that failed with key confirmation error are counted.
 *       Successful PASE negotiations do not reset the rate limiter.
 *     * For PASE negotiations with key confirmation option disabled:
 *       every PASE negotiation, successful or otherwise, is added
 *       to the rate limiter.
 *
 */
#ifndef WEAVE_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS
#define WEAVE_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS         3
#endif // WEAVE_CONFIG_PASE_RATE_LIMITER_MAX_ATTEMPTS

/**
 *  @name Weave Security Manager Memory Management Configuration
 *
 *  @brief
 *    The following definitions enable one of three potential Weave
 *    Security Manager memory-management options:
 *
 *      * #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM
 *      * #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE
 *      * #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC
 *
 *    Note that these options are mutually exclusive and only one
 *    of these options should be set.
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of Weave Security Manager memory-management
 *    functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE and
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC.
 *
 */
#ifndef WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM
#define WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM      0
#endif // WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM

/**
 *  @def WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE
 *
 *  @brief
 *    Enable (1) or disable (0) support for a Weave-provided
 *    implementation of Weave Security Manager memory-management
 *    functions based on temporary network buffer allocation /
 *    release.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM and
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC.
 *
 */
#ifndef WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE
#define WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE        0
#endif // WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE

/**
 *  @def WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC
 *
 *  @brief
 *    Enable (1) or disable (0) support for a Weave-provided
 *    implementation of Weave Security Manager memory-management
 *    functions based on the C Standard Library malloc / free
 *    functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM and
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE.
 *
 */
#ifndef WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC
#define WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC        1
#endif // WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC

/**
 *  @}
 */

#if ((WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM + WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE + WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC) != 1)
#error "Please assert exactly one of WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM, WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE, or WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC."
#endif // ((WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_PLATFORM + WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE + WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_MALLOC) != 1)

/**
 *  @def WEAVE_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS
 *
 *  @brief
 *    Enable (1) or disable (0) simple memory allocator support
 *    for small size network buffers. When enabled, this configuration
 *    requires 4 network buffers with minimum available payload size of
 *    600 bytes.
 *
 *  @note This configuration is only relevant when
 *        #WEAVE_CONFIG_SECURITY_MGR_MEMORY_MGMT_SIMPLE is set and
 *        ignored otherwise.
 *
 */
#ifndef WEAVE_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS
#define WEAVE_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS     0
#endif // WEAVE_CONFIG_SIMPLE_ALLOCATOR_USE_SMALL_BUFFERS

/**
 *  @name Weave Security Manager Time-Consuming Crypto Alerts.
 *
 *  @brief
 *    The following definitions enable one of two potential Weave
 *    Security Manager time-consuming crypto alerts implementations:
 *
 *      * #WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
 *      * #WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
 *
 *    Note that these options are mutually exclusive and only one
 *    of these options should be set.
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
 *
 *  @brief
 *    Enable (1) or disable (0) support for Weave-provided dummy
 *    implementation of Weave security manager time-consuming
 *    crypto alerts functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM.
 *
 */
#ifndef WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY
#define WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY         1
#endif // WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY

/**
 *  @def WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for a platform-specific
 *    implementation of Weave security manager time-consuming
 *    crypto alerts functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY.
 *
 */
#ifndef WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM
#define WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM      0
#endif // WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM

/**
 *  @}
 */

#if ((WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY + WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM) != 1)
#error "Please assert exactly one of WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY or WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM."
#endif // ((WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY + WEAVE_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM) != 1)

/**
 *  @name Weave Random Number Generator (RNG) Implementation Configuration
 *
 *  @brief
 *    The following definitions enable one of three potential Weave
 *    RNG implementation options:
 *
 *      * #WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM
 *      * #WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG
 *      * #WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the Weave Random Number Generator.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG and
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL.
 *
 */
#ifndef WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM
#define WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM            0
#endif // WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM

/**
 *  @def WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG
 *
 *  @brief
 *    Enable (1) or disable (0) support for a Weave-provided
 *    implementation of the Weave Random Number Generator.
 *    This implementation is based on AES-CTR DRBG as
 *    specified in the NIST SP800-90A document.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM and
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL.
 *
 */
#ifndef WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG
#define WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG            0
#endif // WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG

/**
 *  @def WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for a standard OpenSSL
 *    implementation of the Weave Random Number Generator.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM and
 *        #WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG.
 *
 */
#ifndef WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL
#define WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL             1
#endif // WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL

/**
 *  @}
 */

#if ((WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM + WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG + WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL) != 1)
#error "Please assert exactly one of WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM, WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG, or WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL."
#endif // ((WEAVE_CONFIG_RNG_IMPLEMENTATION_PLATFORM + WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG + WEAVE_CONFIG_RNG_IMPLEMENTATION_OPENSSL) != 1)


/**
 *  @def WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED
 *
 *  @brief
 *    Enable (1) or disable (0) a function for seeding the DRBG with
 *    entropy from the /dev/(u)random device.
 *
 *  @note When enabled along with #WEAVE_CONFIG_RNG_IMPLEMENTATION_NESTDRBG
 *        this function becomes the default seeding function for the DRBG if
 *        another isn't specified at initialization time.
 *
 */
#ifndef WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED
#define WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED              0
#endif // WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED

/**
 *  @def WEAVE_CONFIG_DEV_RANDOM_DEVICE_NAME
 *
 *  @brief
 *    The device name used by the dev random entropy function.
 *
 *  @note Only meaningful when #WEAVE_CONFIG_DEV_RANDOM_DRBG_SEED is enabled.
 *
 */
#ifndef WEAVE_CONFIG_DEV_RANDOM_DEVICE_NAME
#define WEAVE_CONFIG_DEV_RANDOM_DEVICE_NAME                 "/dev/urandom"
#endif // WEAVE_CONFIG_DEV_RANDOM_DEVICE_NAME



/**
 *  @name Weave AES Block Cipher Algorithm Implementation Configuration.
 *
 *  @brief
 *    The following definitions enable one of the potential Weave
 *    AES implementation options:
 *
 *      * #WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM
 *      * #WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the Weave AES functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL and
 *        #WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI
 *
 */
#ifndef WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM
#define WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM            0
#endif // WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM

/**
 *  @def WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for the OpenSSL
 *    implementation of the Weave AES functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL
#define WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL             1
#endif // WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL

/**
 *  @def WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI
 *
 *  @brief
 *    Enable (1) or disable (0) support for an implementation
 *    of the Weave AES functions using Intel AES-NI intrinsics.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI
#define WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI              0
#endif // WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI

/**
 *  @def WEAVE_CONFIG_AES_IMPLEMENTATION_MBEDTLS
 *
 *  @brief
 *    Enable (1) or disable (0) support the mbed TLS
 *    implementation of the Weave AES functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_AES_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_AES_IMPLEMENTATION_MBEDTLS
#define WEAVE_CONFIG_AES_IMPLEMENTATION_MBEDTLS              0
#endif // WEAVE_CONFIG_AES_IMPLEMENTATION_MBEDTLS

/**
 *  @}
 */

#if ((WEAVE_CONFIG_AES_IMPLEMENTATION_PLATFORM + \
      WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL + \
      WEAVE_CONFIG_AES_IMPLEMENTATION_AESNI + \
      WEAVE_CONFIG_AES_IMPLEMENTATION_MBEDTLS) != 1)
#error "Please assert exactly one WEAVE_CONFIG_AES_IMPLEMENTATION_... option."
#endif

/**
 *  @def WEAVE_CONFIG_AES_USE_EXPANDED_KEY
 *
 *  @brief
 *    Defines whether AES key is used in its expanded (1) or native (0) form.
 *
 *  @note OpenSSL AES implementation uses its own AES key declaration
 *        and this configuration option is ignored when
 *        #WEAVE_CONFIG_AES_IMPLEMENTATION_OPENSSL is set.
 *
 */
#ifndef WEAVE_CONFIG_AES_USE_EXPANDED_KEY
#define WEAVE_CONFIG_AES_USE_EXPANDED_KEY                   0
#endif // WEAVE_CONFIG_AES_USE_EXPANDED_KEY


/**
 *  @name Weave SHA1 and SHA256 Hash Algorithms Implementation Configuration.
 *
 *  @brief
 *    The following definitions enable one of three potential Weave
 *    hash implementation options:
 *
 *      * #WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM
 *      * #WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
 *      * #WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL
 *      * #WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
 *
 *    Note that these options are mutually exclusive and only one of
 *    these options should be set.
 *
 *  @{
 */

/**
 *  @def WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM
 *
 *  @brief
 *    Enable (1) or disable (0) support for platform-specific
 *    implementation of the Weave SHA1 and SHA256 hashes.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM           0
#endif // WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM

/**
 *  @def WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
 *
 *  @brief
 *    Enable (1) or disable (0) support for a Weave-provided
 *    implementation of the Weave SHA1 and SHA256 hash functions.
 *    This implementation is using sha1 and sha256 engines from
 *    mincrypt library of Android core.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT           0
#endif // WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT

/**
 *  @def WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL
 *
 *  @brief
 *    Enable (1) or disable (0) support for the OpenSSL
 *    implementation of the Weave SHA1 and SHA256 hash functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL            1
#endif // WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL

/**
 *  @def WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
 *
 *  @brief
 *    Enable (1) or disable (0) support for the mbedTLS
 *    implementation of the Weave SHA1 and SHA256 hash functions.
 *
 *  @note This configuration is mutual exclusive with other
 *        WEAVE_CONFIG_HASH_IMPLEMENTATION options.
 *
 */
#ifndef WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS
#define WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS            0
#endif // WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS

/**
 *  @}
 */

#if ((WEAVE_CONFIG_HASH_IMPLEMENTATION_PLATFORM + \
      WEAVE_CONFIG_HASH_IMPLEMENTATION_MINCRYPT + \
      WEAVE_CONFIG_HASH_IMPLEMENTATION_OPENSSL  + \
      WEAVE_CONFIG_HASH_IMPLEMENTATION_MBEDTLS) != 1)
#error "Please assert exactly one WEAVE_CONFIG_HASH_IMPLEMENTATION_... option."
#endif


/**
 *  @name Weave key export protocol configuration.
 *
 *  @brief
 *    The following definitions define the configurations supported
 *    for Weave's key export protocol.
 *
 *    This protocol is used to export secret key material from Weave device.
 *    Weave supports the following protocol configurations:
 *
 *    * #WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
 *    * #WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
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
 *  @def WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
 *
 *  @brief
 *    This Weave key export protocol configuration uses secp224r1
 *    Elliptic Curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1
#define WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1            1
#endif // WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG1

/**
 *  @def WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
 *
 *  @brief
 *    This Weave key export protocol configuration uses secp256r1
 *    Elliptic Curve.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2
#define WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2            1
#endif // WEAVE_CONFIG_SUPPORT_KEY_EXPORT_CONFIG2

/**
 *  @}
 */


/**
 *  @def WEAVE_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES
 *
 *  @brief
 *    Allow the use of elliptic curves beyond the standard ones
 *    supported by Weave.
 *
 */
#ifndef WEAVE_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES
#define WEAVE_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES     0
#endif // WEAVE_CONFIG_ALLOW_NON_STANDARD_ELLIPTIC_CURVES

/**
 *  @def WEAVE_CONFIG_MAX_EC_BITS
 *
 *  @brief
 *    The maximum size elliptic curve supported, in bits.
 *
 */
#ifndef WEAVE_CONFIG_MAX_EC_BITS
#define WEAVE_CONFIG_MAX_EC_BITS                            256
#endif // WEAVE_CONFIG_MAX_EC_BITS

/**
 *  @def WEAVE_CONFIG_MAX_RSA_BITS
 *
 *  @brief
 *    The maximum size RSA modulus supported, in bits.
 *
 */
#ifndef WEAVE_CONFIG_MAX_RSA_BITS
#define WEAVE_CONFIG_MAX_RSA_BITS                           4096
#endif // WEAVE_CONFIG_MAX_RSA_BITS

/**
 *  @def WEAVE_CONFIG_MAX_PEER_NODES
 *
 *  @brief
 *    Maximum number of peer nodes that the local node can communicate
 *    with.
 *
 */
#ifndef WEAVE_CONFIG_MAX_PEER_NODES
#define WEAVE_CONFIG_MAX_PEER_NODES                         128
#endif // WEAVE_CONFIG_MAX_PEER_NODES

/**
 *  @def WEAVE_CONFIG_MAX_CONNECTIONS
 *
 *  @brief
 *    Maximum number of simultaneously active connections.
 *
 */
#ifndef WEAVE_CONFIG_MAX_CONNECTIONS
#define WEAVE_CONFIG_MAX_CONNECTIONS                        INET_CONFIG_NUM_TCP_ENDPOINTS
#endif // WEAVE_CONFIG_MAX_CONNECTIONS

/**
 *  @def WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
 *
 *  @brief
 *    Maximum number of simultaneously active inbound TCP connections.
 *
 *    Regardless of what #WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
 *    is set to, the total number of inbound connections cannot exceed
 *    #WEAVE_CONFIG_MAX_CONNECTIONS, which is the overall limit for
 *    inbound and outbound connections.
 */
#ifndef WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS
#define WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS           (WEAVE_CONFIG_MAX_CONNECTIONS * 4 / 5)
#endif // WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS

/**
 *  @def WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
 *
 *  @brief
 *    Maximum number of simultaneously active inbound TCP connections
 *    from the single IP address.
 *
 *    Regardless of what #WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
 *    is set to, the total number of inbound connections from a single IP
 *    address cannot exceed #WEAVE_CONFIG_MAX_CONNECTIONS or
 *    #WEAVE_CONFIG_MAX_INCOMING_TCP_CONNECTIONS.
 */
#ifndef WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP
#define WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP    2
#endif // WEAVE_CONFIG_MAX_INCOMING_TCP_CON_FROM_SINGLE_IP

/**
 *  @def WEAVE_CONFIG_MAX_TUNNELS
 *
 *  @brief
 *    Maximum number of simultaneously active connection tunnels.
 *
 */
#ifndef WEAVE_CONFIG_MAX_TUNNELS
#define WEAVE_CONFIG_MAX_TUNNELS                            1
#endif // WEAVE_CONFIG_MAX_TUNNELS

/**
 *  @def WEAVE_CONFIG_MAX_SESSION_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously active session keys.
 *
 */
#ifndef WEAVE_CONFIG_MAX_SESSION_KEYS
#define WEAVE_CONFIG_MAX_SESSION_KEYS                       WEAVE_CONFIG_MAX_CONNECTIONS
#endif // WEAVE_CONFIG_MAX_SESSION_KEYS

/**
 *  @def WEAVE_CONFIG_MAX_APPLICATION_EPOCH_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously supported application epoch keys.
 *    This define should be set to the maximum number of epoch keys
 *    that can be simultaneously provisioned on Weave node by Weave
 *    service. The maximum supported value is 8, however, in most cases
 *    only two such keys will exist on device at any given point in time.
 *
 */
#ifndef WEAVE_CONFIG_MAX_APPLICATION_EPOCH_KEYS
#define WEAVE_CONFIG_MAX_APPLICATION_EPOCH_KEYS             4
#endif // WEAVE_CONFIG_MAX_APPLICATION_EPOCH_KEYS

/**
 *  @def WEAVE_CONFIG_MAX_APPLICATION_GROUPS
 *
 *  @brief
 *    Maximum number of simultaneously supported application groups.
 *    This define should be set to the number of Weave application
 *    groups, in which associated Weave node has membership.
 *
 */
#ifndef WEAVE_CONFIG_MAX_APPLICATION_GROUPS
#define WEAVE_CONFIG_MAX_APPLICATION_GROUPS                 8
#endif // WEAVE_CONFIG_MAX_APPLICATION_GROUPS

/**
 *  @def WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
 *
 *  @brief
 *    Enable (1) or disable (0) support for the application group keys
 *    used for Weave message encryption.
 *
 */
#ifndef WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
#define WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC         1
#endif // WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

/**
 *  @def WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS
 *
 *  @brief
 *    Maximum number of simultaneously cached Weave message encryption
 *    application keys.
 *    Caching these keys speeds up message encoding/decoding processes
 *    and eliminates the need to retrieve constituent key material from
 *    the platform memory every time we derive these keys.
 *    This define can be set equal to the number of application groups
 *    (#WEAVE_CONFIG_MAX_APPLICATION_GROUPS) supported by the Weave node
 *    such that exactly one key can be cached for each application group.
 *    It might be a good idea to allocate few more entries in the key
 *    cache for the corner cases, where application group is having
 *    simultaneous conversations using an 'old' and a 'new' epoch key.
 *
 *  @note This configuration is only relevant when
 *        #WEAVE_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC is set and
 *        ignored otherwise.
 *
 */
#ifndef WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS
#define WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS            (WEAVE_CONFIG_MAX_APPLICATION_GROUPS + 1)
#endif // WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS

#if !(WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS > 0 && WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS < 256)
#error "Please set WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS to a value greater than zero and smaller than 256."
#endif // !(WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS > 0 && WEAVE_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS < 256)

/**
 *  @name Weave Encrypted Passcode Configuration
 *
 *  @brief
 *    The following definitions enable (1) or disable (0) supported for
 *    Weave encrypted passcode configurations. Each configuration
 *    uniquely specifies how Weave passcode was encrypted, authenticated,
 *    and structured. Weave supports the following passcode
 *    configurations:
 *
 *    * #WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
 *    * #WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG2
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
 *  @def WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
 *
 *  @brief
 *    This Weave passcode configuration does not encrypt the passcode
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
#ifndef WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY
#define WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY     0
#endif // WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY

/**
 *  @def WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG2
 *
 *  @brief
 *    This Weave passcode configuration uses AES128 algorithm in ECB
 *    mode to encrypt passcodes. It also uses SHA1 Hash-based Message
 *    Authentication Code (HMAC) to authenticate and uniquely identify
 *    (fingerprint) the passcode.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG2
#define WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG2               1
#endif // WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG2

/**
 *  @}
 */

/**
 *  @def WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT
 *
 *  @brief
 *    The default amount of time, in milliseconds, after which an in-progess
 *    session establishment will fail due to a timeout.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT
#define WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT  30000
#endif // WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT

/**
 *  @def WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT
 *
 *  @brief
 *    The default minimum amount of time, in milliseconds, that an unreserved and idle
 *    security session will be allowed to exist before being destroyed.  In practice,
 *    unreserved idle sessions can exist for up to twice this value.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT
#define WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT           15000
#endif // WEAVE_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT

/**
 *  @def WEAVE_CONFIG_NUM_MESSAGE_BUFS
 *
 *  @brief
 *    Total number of message buffers. Only used for the BSD sockets
 *    configuration.
 *
 */
#ifndef WEAVE_CONFIG_NUM_MESSAGE_BUFS
#define WEAVE_CONFIG_NUM_MESSAGE_BUFS                       16
#endif // WEAVE_CONFIG_NUM_MESSAGE_BUFS

/**
 *  @def WEAVE_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
 *
 *  @brief
 *    Maximum number of simultaneously active unsolicited message
 *    handlers.
 *
 */
#ifndef WEAVE_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define WEAVE_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS       32
#endif // WEAVE_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

/**
 *  @def WEAVE_CONFIG_MAX_EXCHANGE_CONTEXTS
 *
 *  @brief
 *    Maximum number of simultaneously active exchange contexts.
 *
 */
#ifndef WEAVE_CONFIG_MAX_EXCHANGE_CONTEXTS
#define WEAVE_CONFIG_MAX_EXCHANGE_CONTEXTS                  16
#endif // WEAVE_CONFIG_MAX_EXCHANGE_CONTEXTS

/**
 *  @def WEAVE_CONFIG_MAX_BINDINGS
 *
 *  @brief
 *    Maximum number of simultaneously active bindings per WeaveExchangeManager
 *    The new single source TimeSync client takes one binding.
 *    Every WDM one-way subscription takes one binding. Mutual subscription counts as two one-way subscriptions.
 *    A reserved slot is needed to take an incoming subscription request.
 *    For a device with 2 mutual subscriptions, and one single source time sync client, it needs 2 x 2 + 1 = 5 bindings at least.
 *    At least six is needed if it still wants to take new WDM subscriptions under this load.
 */
#ifndef WEAVE_CONFIG_MAX_BINDINGS
#define WEAVE_CONFIG_MAX_BINDINGS                           6
#endif // WEAVE_CONFIG_MAX_BINDINGS

/**
 *  @def WEAVE_CONFIG_CONNECT_IP_ADDRS
 *
 *  @brief
 *    Maximum number of IP addresses tried when connecting to a
 *    hostname.
 *
 */
#ifndef WEAVE_CONFIG_CONNECT_IP_ADDRS
#define WEAVE_CONFIG_CONNECT_IP_ADDRS                       4
#endif // WEAVE_CONFIG_CONNECT_IP_ADDRS

/**
 *  @def WEAVE_CONFIG_DEFAULT_UDP_MTU_SIZE
 *
 *  @brief
 *    The default MTU size for an IPv6 datagram carrying UDP. This is useful
 *    for senders who want to send UDP Weave messages that fit within a single
 *    IPv6 datagram.
 *
 *    1280 is the guaranteed minimum IPv6 MTU.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_UDP_MTU_SIZE
#define WEAVE_CONFIG_DEFAULT_UDP_MTU_SIZE                   1280
#endif // WEAVE_CONFIG_DEFAULT_UDP_MTU_SIZE

/**
 *  @def WEAVE_HEADER_RESERVE_SIZE
 *
 *  @brief
 *    The number of bytes to reserve in a network packet buffer to contain the
 *    Weave message and exchange headers.
 *
 *    This number was calculated as follows:
 *
 *      Weave Message Header:
 *
 *          2 -- Frame Length
 *          2 -- Message Header
 *          4 -- Message Id
 *          8 -- Source Node Id
 *          8 -- Destination Node Id
 *          2 -- Key Id
 *
 *      Weave Exchange Header:
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
#ifndef WEAVE_HEADER_RESERVE_SIZE
#define WEAVE_HEADER_RESERVE_SIZE                           38
#endif // WEAVE_HEADER_RESERVE_SIZE

/**
 *  @def WEAVE_TRAILER_RESERVE_SIZE
 *
 *  @brief
 *    TODO
 *
 */
#ifndef WEAVE_TRAILER_RESERVE_SIZE
#define WEAVE_TRAILER_RESERVE_SIZE                          20
#endif // WEAVE_TRAILER_RESERVE_SIZE

/**
 *  @def WEAVE_PORT
 *
 *  @brief
 *    Weave TCP/UDP port for secured Weave traffic.
 *
 */
#ifndef WEAVE_PORT
#define WEAVE_PORT                                          11095
#endif // WEAVE_PORT

/**
 *  @def WEAVE_UNSECURED_PORT
 *
 *  @brief
 *    Weave TCP/UDP port for unsecured Weave traffic.
 *
 */
#ifndef WEAVE_UNSECURED_PORT
#define WEAVE_UNSECURED_PORT                                11096
#endif // WEAVE_UNSECURED_PORT

/**
 *  @def WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
 *
 *  @brief
 *    Enable use of an ephemeral UDP source port for locally initiated Weave exchanges.
 */
#ifndef WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT
#define WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT              0
#endif // WEAVE_CONFIG_ENABLE_EPHEMERAL_UDP_PORT

/**
 *  @def WEAVE_CONFIG_SECURITY_TEST_MODE
 *
 *  @brief
 *    Enable various features that make it easier to debug secure Weave communication.
 *
 *  @note
 *    WARNING: This option makes it possible to circumvent basic Weave security functionality,
 *    including message encryption. Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.
 */
#ifndef WEAVE_CONFIG_SECURITY_TEST_MODE
#define WEAVE_CONFIG_SECURITY_TEST_MODE                     0
#endif // WEAVE_CONFIG_SECURITY_TEST_MODE

/**
 *  @def WEAVE_CONFIG_ENABLE_DNS_RESOLVER
 *
 *  @brief
 *    Enable support for resolving hostnames with a DNS resolver.
 */
#ifndef WEAVE_CONFIG_ENABLE_DNS_RESOLVER
#define WEAVE_CONFIG_ENABLE_DNS_RESOLVER                    (INET_CONFIG_ENABLE_DNS_RESOLVER)
#endif // WEAVE_CONFIG_ENABLE_DNS_RESOLVER

/**
 *  @def WEAVE_CONFIG_RESOLVE_IPADDR_LITERAL
 *
 *  @brief
 *    Enable support for resolving hostnames as literal IP addresses without a DNS resolver.
 *
 *    For historical reasons, the default is \c TRUE where \c WEAVE_SYSTEM_CONFIG_USE_SOCKETS=1,
 *    and \c FALSE otherwise. The exception in the LwIP-only case was originally made to facilitate
 *    integration and change management with existing development lines. The default may
 *    change in the future to \c TRUE in all cases.
 */
#ifndef WEAVE_CONFIG_RESOLVE_IPADDR_LITERAL
#define WEAVE_CONFIG_RESOLVE_IPADDR_LITERAL                 (WEAVE_SYSTEM_CONFIG_USE_SOCKETS)
#endif // WEAVE_CONFIG_RESOLVE_IPADDR_LITERAL

/**
 *  @def WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
 *
 *  @brief
 *    Enable support for listening on particular addresses/interfaces.
 *
 *    This allows testing multiple instances of the Weave stack
 *    running on a single host.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_TARGETED_LISTEN
#define WEAVE_CONFIG_ENABLE_TARGETED_LISTEN                 (!WEAVE_SYSTEM_CONFIG_USE_LWIP)
#endif // WEAVE_CONFIG_ENABLE_TARGETED_LISTEN

/**
 *  @def WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
 *
 *  @brief
 *    Enable support for receiving TCP connections over an unsecured
 *    network layer (for example, from a device that is provisionally joined
 *    to a 6LowPAN network but does not possess the 802.15.4 network
 *    keys).
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN
#define WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN            0
#endif // WEAVE_CONFIG_ENABLE_UNSECURED_TCP_LISTEN

/**
 *  @def WEAVE_CONFIG_DEBUG_CERT_VALIDATION
 *
 *  @brief
 *    Enable support for debugging output from certificate validation.
 *
 */
#ifndef WEAVE_CONFIG_DEBUG_CERT_VALIDATION
#define WEAVE_CONFIG_DEBUG_CERT_VALIDATION                  1
#endif // WEAVE_CONFIG_DEBUG_CERT_VALIDATION

/**
 *  @def WEAVE_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID
 *
 *  @brief
 *    EC curve to be used to generate Weave operational device certificate.
 *
 */
#ifndef WEAVE_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID
#define WEAVE_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID       (nl::Weave::Profiles::Security::kWeaveCurveId_prime256v1)
#endif // WEAVE_CONFIG_OPERATIONAL_DEVICE_CERT_CURVE_ID

/**
 *  @def WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE
 *
 *  @brief
 *    This is a packed valid date to be encoded in the Weave
 *    operational device certificate. Any date before
 *    that date the certificate is considered invalid.
 *    The following functions can be used to calculate packed
 *    date/time: PackCertTime() and PackedCertTimeToDate().
 *    Weave packed certificate dates are limited to representing
 *    dates that are on or after 2000/01/01.
 *    Mathematical expression to calculate packed date is:
 *        (((year - 2000) * 12 + (mon - 1)) * 31 + (day - 1))
 *    Currently encoded value corresponds to 2019/01/01.
 *
 */
#ifndef WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE
#define WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE   0x1B9C
#endif // WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_BEFORE

/**
 *  @def WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER
 *
 *  @brief
 *    This is the valid date to be encoded in the Weave
 *    operational device certificate. Any date after
 *    that date the certificate is considered invalid.
 *    The following functions can be used to calculate packed
 *    date/time: PackCertTime() and PackedCertTimeToDate().
 *    Weave packed certificate dates are limited to representing
 *    dates that are on or after 2000/01/01.
 *    Mathematical expression to calculate packed date is:
 *        (((year - 2000) * 12 + (mon - 1)) * 31 + (day - 1))
 *    Currently encoded value corresponds to 2069/01/01.
 *
 */
#ifndef WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER
#define WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER    0x6444
#endif // WEAVE_CONFIG_OP_DEVICE_CERT_VALID_DATE_NOT_AFTER

/**
 *  @def WEAVE_CONFIG_ENABLE_PASE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating PASE sessions.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_PASE_INITIATOR
#define WEAVE_CONFIG_ENABLE_PASE_INITIATOR                  1
#endif // WEAVE_CONFIG_ENABLE_PASE_INITIATOR

/**
 *  @def WEAVE_CONFIG_ENABLE_PASE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to PASE sessions initiated by
 *    other nodes.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_PASE_RESPONDER
#define WEAVE_CONFIG_ENABLE_PASE_RESPONDER                  1
#endif // WEAVE_CONFIG_ENABLE_PASE_RESPONDER

/**
 *  @def WEAVE_CONFIG_ENABLE_CASE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating CASE sessions.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_CASE_INITIATOR
#define WEAVE_CONFIG_ENABLE_CASE_INITIATOR                  1
#endif // WEAVE_CONFIG_ENABLE_CASE_INITIATOR

/**
 *  @def WEAVE_CONFIG_ENABLE_CASE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to CASE sessions initiated by other nodes.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_CASE_RESPONDER
#define WEAVE_CONFIG_ENABLE_CASE_RESPONDER                  1
#endif // WEAVE_CONFIG_ENABLE_CASE_RESPONDER

/**
 *  @def WEAVE_CONFIG_SUPPORT_CASE_CONFIG1
 *
 *  @brief
 *    Enable use of CASE protocol configuration 1.
 *
 *  @note CASE config 1 uses SHA-1 for message signatures, which is deprecated.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_CASE_CONFIG1
#define WEAVE_CONFIG_SUPPORT_CASE_CONFIG1                   1
#endif // WEAVE_CONFIG_SUPPORT_CASE_CONFIG1

/**
 *  @def WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID
 *
 *  @brief
 *    Default ECDH curve to be used when initiating a CASE session, if not overridden by the application.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID
#if WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1
#define WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID                  (nl::Weave::Profiles::Security::kWeaveCurveId_secp224r1)
#elif WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID                  (nl::Weave::Profiles::Security::kWeaveCurveId_prime256v1)
#elif WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP192R1
#define WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID                  (nl::Weave::Profiles::Security::kWeaveCurveId_prime192v1)
#else
#define WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID                  (nl::Weave::Profiles::Security::kWeaveCurveId_secp160r1)
#endif
#endif // WEAVE_CONFIG_DEFAULT_CASE_CURVE_ID

/**
 *  @def WEAVE_CONFIG_DEFAULT_CASE_ALLOWED_CURVES
 *
 *  @brief
 *    Default set of ECDH curves allowed to be used in a CASE session (initiating or responding), if not overridden by the application.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_CASE_ALLOWED_CURVES
#if WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP224R1 || WEAVE_CONFIG_SUPPORT_ELLIPTIC_CURVE_SECP256R1
#define WEAVE_CONFIG_DEFAULT_CASE_ALLOWED_CURVES            (nl::Weave::Profiles::Security::kWeaveCurveSet_secp224r1|nl::Weave::Profiles::Security::kWeaveCurveSet_prime256v1)
#else
#define WEAVE_CONFIG_DEFAULT_CASE_ALLOWED_CURVES            (nl::Weave::Profiles::Security::kWeaveCurveSet_All)
#endif
#endif // WEAVE_CONFIG_DEFAULT_CASE_ALLOWED_CURVES

/**
 * @def WEAVE_CONFIG_LEGACY_CASE_AUTH_DELEGATE
 *
 * @brief
 *   Enable use of the legacy WeaveCASEAuthDelegate interface.
 */
#ifndef WEAVE_CONFIG_LEGACY_CASE_AUTH_DELEGATE
#define WEAVE_CONFIG_LEGACY_CASE_AUTH_DELEGATE 1
#endif

/**
 *  @def WEAVE_CONFIG_MAX_SHARED_SESSIONS_END_NODES
 *
 *  @brief
 *    The maximum number of end nodes simultaneously supported
 *    for all active shared sessions.
 *
 */
#ifndef WEAVE_CONFIG_MAX_SHARED_SESSIONS_END_NODES
#define WEAVE_CONFIG_MAX_SHARED_SESSIONS_END_NODES          10
#endif // WEAVE_CONFIG_MAX_SHARED_SESSIONS_END_NODES

/**
 *  @def WEAVE_CONFIG_MAX_END_NODES_PER_SHARED_SESSION
 *
 *  @brief
 *    The maximum number of end nodes simultaneously supported
 *    per active shared session.
 *
 */
#ifndef WEAVE_CONFIG_MAX_END_NODES_PER_SHARED_SESSION
#define WEAVE_CONFIG_MAX_END_NODES_PER_SHARED_SESSION       10
#endif // WEAVE_CONFIG_MAX_END_NODES_PER_SHARED_SESSION

/**
 *  @def WEAVE_CONFIG_ENABLE_TAKE_INITIATOR
 *
 *  @brief
 *    Enable support for initiating TAKE sessions.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_TAKE_INITIATOR
#define WEAVE_CONFIG_ENABLE_TAKE_INITIATOR                  0
#endif // WEAVE_CONFIG_ENABLE_TAKE_INITIATOR

/**
 *  @def WEAVE_CONFIG_ENABLE_TAKE_RESPONDER
 *
 *  @brief
 *    Enable support for responding to TAKE sessions initiated by other nodes.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_TAKE_RESPONDER
#define WEAVE_CONFIG_ENABLE_TAKE_RESPONDER                  0
#endif // WEAVE_CONFIG_ENABLE_TAKE_RESPONDER

/**
 *  @def WEAVE_CONFIG_ENABLE_KEY_EXPORT_INITIATOR
 *
 *  @brief
 *    Enable support for initiating key export request.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_KEY_EXPORT_INITIATOR
#define WEAVE_CONFIG_ENABLE_KEY_EXPORT_INITIATOR            1
#endif // WEAVE_CONFIG_ENABLE_KEY_EXPORT_INITIATOR

/**
 *  @def WEAVE_CONFIG_ENABLE_KEY_EXPORT_RESPONDER
 *
 *  @brief
 *    Enable support for responding to key export request initiated by other nodes.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_KEY_EXPORT_RESPONDER
#define WEAVE_CONFIG_ENABLE_KEY_EXPORT_RESPONDER            1
#endif // WEAVE_CONFIG_ENABLE_KEY_EXPORT_RESPONDER

/**
 * @def WEAVE_CONFIG_LEGACY_KEY_EXPORT_DELEGATE
 *
 * @brief
 *   Enable use of the legacy WeaveKeyExportDelegate interface.
 */
#ifndef WEAVE_CONFIG_LEGACY_KEY_EXPORT_DELEGATE
#define WEAVE_CONFIG_LEGACY_KEY_EXPORT_DELEGATE 1
#endif

/**
 *  @def WEAVE_CONFIG_REQUIRE_AUTH
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests via an
 *    authenticated session.
 *
 *    This broadly controls whether or not a number of Weave servers
 *    require client requests to be sent via an authenticated session
 *    and provides a default configuration value to these related
 *    definitions:
 *
 *      * #WEAVE_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
 *      * #WEAVE_CONFIG_REQUIRE_AUTH_FABRIC_PROV
 *      * #WEAVE_CONFIG_REQUIRE_AUTH_NETWORK_PROV
 *      * #WEAVE_CONFIG_REQUIRE_AUTH_SERVICE_PROV
 *
 *    @note These configurations shall be deasserted for development
 *          and testing purposes only. No Weave-enabled device shall
 *          be certified without these asserted.
 *
 */
#ifndef WEAVE_CONFIG_REQUIRE_AUTH
#define WEAVE_CONFIG_REQUIRE_AUTH                           1
#endif // WEAVE_CONFIG_REQUIRE_AUTH

/**
 *  @def WEAVE_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    Weave Device Control server via an authenticated session. See
 *    also #WEAVE_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No Weave-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef WEAVE_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL
#define WEAVE_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL            WEAVE_CONFIG_REQUIRE_AUTH
#endif // WEAVE_CONFIG_REQUIRE_AUTH_DEVICE_CONTROL

/**
 *  @def WEAVE_CONFIG_REQUIRE_AUTH_FABRIC_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    Weave Fabric Provisioning server via an authenticated
 *    session. See also #WEAVE_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No Weave-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef WEAVE_CONFIG_REQUIRE_AUTH_FABRIC_PROV
#define WEAVE_CONFIG_REQUIRE_AUTH_FABRIC_PROV               WEAVE_CONFIG_REQUIRE_AUTH
#endif // WEAVE_CONFIG_REQUIRE_AUTH_FABRIC_PROV

/**
 *  @def WEAVE_CONFIG_REQUIRE_AUTH_NETWORK_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    Weave Network Provisioning server via an authenticated
 *    session. See also #WEAVE_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No Weave-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef WEAVE_CONFIG_REQUIRE_AUTH_NETWORK_PROV
#define WEAVE_CONFIG_REQUIRE_AUTH_NETWORK_PROV              WEAVE_CONFIG_REQUIRE_AUTH
#endif // WEAVE_CONFIG_REQUIRE_AUTH_NETWORK_PROV

/**
 *  @def WEAVE_CONFIG_REQUIRE_AUTH_SERVICE_PROV
 *
 *  @brief
 *    Enable (1) or disable (0) support for client requests to the
 *    Weave Service Provisioning server via an authenticated
 *    session. See also #WEAVE_CONFIG_REQUIRE_AUTH.
 *
 *    @note This configuration shall be deasserted for development
 *          and testing purposes only. No Weave-enabled device shall
 *          be certified without this asserted.
 *
 */
#ifndef WEAVE_CONFIG_REQUIRE_AUTH_SERVICE_PROV
#define WEAVE_CONFIG_REQUIRE_AUTH_SERVICE_PROV              WEAVE_CONFIG_REQUIRE_AUTH
#endif // WEAVE_CONFIG_REQUIRE_AUTH_SERVICE_PROV

/**
 *  @def WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT
 *
 *  @brief
 *    Enable (1) or disable (0) support for the handling of Weave
 *    Provisioning Bundles.
 *
 *    Weave Provisioning Bundles are a Weave TLV payload containing
 *    the Weave certificate, corresponding private key, and pairing
 *    code / entry key that a Weave device would have otherwise
 *    received at its time of manufacture.
 *
 *    Enable this if your family of device needs to support in-field
 *    provisioning (IFP). IFP for Weave devices is neither generally
 *    supported nor recommended.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT
#define WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT     1
#endif // WEAVE_CONFIG_ENABLE_PROVISIONING_BUNDLE_SUPPORT

/**
 *  @def WEAVE_ERROR_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    nl::Weave::Logging::LogCategory::kLogCategory_Error category.
 *
 */
#ifndef WEAVE_ERROR_LOGGING
#define WEAVE_ERROR_LOGGING                                 1
#endif // WEAVE_ERROR_LOGGING

/**
 *  @def WEAVE_PROGRESS_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    nl::Weave::Logging::LogCategory::kLogCategory_Progress category.
 *
 */
#ifndef WEAVE_PROGRESS_LOGGING
#define WEAVE_PROGRESS_LOGGING                              1
#endif // WEAVE_PROGRESS_LOGGING

/**
 *  @def WEAVE_DETAIL_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    nl::Weave::Logging::kLogCategory_Detail category.
 *
 */
#ifndef WEAVE_DETAIL_LOGGING
#define WEAVE_DETAIL_LOGGING                                1
#endif // WEAVE_DETAIL_LOGGING

/**
 *  @def WEAVE_RETAIN_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    nl::Weave::Logging::LogCategory::kLogCategory_Retain category.
 *    If not defined by the application, by default WEAVE_RETAIN_LOGGING is
 *    remapped to WEAVE_PROGRESS_LOGGING
 *
 */


/**
 *  @def WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of errors at function exit via the
 *    WeaveLogFunctError() macro.
 */
#ifndef WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
#define WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING 0
#endif // WEAVE_CONFIG_ENABLE_FUNCT_ERROR_LOGGING


/**
 *  @def WEAVE_CONFIG_ENABLE_CONDITION_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of failed conditions via the
 *    WeaveLogIfFalse() macro.
 */
#ifndef WEAVE_CONFIG_ENABLE_CONDITION_LOGGING
#define WEAVE_CONFIG_ENABLE_CONDITION_LOGGING 0
#endif // WEAVE_CONFIG_ENABLE_CONDITION_LOGGING


/**
 *  @def WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
 *
 *  @brief
 *    If set to (1), use of the ServiceDirectory implementation
 *    is enabled. Default value is (1) or enabled.
 *
 *  @note
 *    Enabling this profile allows applications using Weave to
 *    request a connection to a particular Weave service using
 *    a predefined service endpoint. It is relevant for
 *    applications that run on devices that interact with the
 *    Service over a direct TCP/IPv4 connection rather than those
 *    that use the Weave Tunnel through a gateway device. For
 *    devices of the latter category, the Service Directory
 *    profile can be disabled via this compilation switch.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
#define WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY               1
#endif // WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY

/**
 *  @def WEAVE_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS
 *
 *  @brief
 *    This is the default timeout for the connect call to the
 *    directory server to wait for success or being notified
 *    of an error.
 *
 */
#ifndef WEAVE_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS
#define WEAVE_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS      (10000)
#endif // WEAVE_CONFIG_SERVICE_DIR_CONNECT_TIMEOUT_MSECS

/**
 *  @def WEAVE_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT
 *
 *  @brief
 *    The maximum amount of time, in milliseconds, that an idle inbound
 *    Weave connection will be allowed to exist before being closed.
 *
 *    This is a default value that can be overridden at runtime by the
 *    application.
 *
 *    A value of 0 disables automatic closing of idle connections.
 *
 */
#ifndef WEAVE_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT
#define WEAVE_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT        15000
#endif // WEAVE_CONFIG_DEFAULT_INCOMING_CONNECTION_IDLE_TIMEOUT

/**
 *  @def WEAVE_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT
 *
 *  @brief
 *    The amount of time (in milliseconds) which a peer is given
 *    to respond to a message counter synchronization request.
 *    Depending on when the request is sent, peers may
 *    actually have up to twice this time.
 *
 */
#ifndef WEAVE_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT
#define WEAVE_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT          2000
#endif // WEAVE_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT

/**
 *  @def WEAVE_CONFIG_TEST
 *
 *  @brief
 *    If asserted (1), enable APIs that help implement
 *    unit and integration tests.
 *
 */
#ifndef WEAVE_CONFIG_TEST
#define WEAVE_CONFIG_TEST                                   0
#endif // WEAVE_CONFIG_TEST

/**
 *  @def WEAVE_CONFIG_SHORT_ERROR_STR
 *
 *  @brief
 *    If asserted (1), produce shorter error strings that only carry a
 *    minimum of information.
 *
 */
#ifndef WEAVE_CONFIG_SHORT_ERROR_STR
#define WEAVE_CONFIG_SHORT_ERROR_STR                        0
#endif // WEAVE_CONFIG_SHORT_ERROR_STR

/**
 *  @def WEAVE_CONFIG_ERROR_STR_SIZE
 *
 *  @brief
 *    This defines the size of the buffer to store a formatted error string.
 *    If the formatting of an error string exceeds this size it will be truncated.
 *
 *    The default size varies based on the WEAVE_CONFIG_SHORT_ERROR_STR option.
 *
 *    When WEAVE_CONFIG_SHORT_ERROR_STR is 0, a large default buffer size is used
 *    to accommodate descriptive text summarizing the cause of the error. E.g.:
 *
 *         "Weave Error 4047 (0x00000FCF): Invalid Argument"
 *
 *    When WEAVE_CONFIG_SHORT_ERROR_STR is 1, the buffer size is set to accommodate
 *    a minimal error string consisting of a 10 character subsystem name followed
 *    by an 8 character error number, plus boilerplate. E.g.:
 *
 *         "Error Weave:0x00000FCF"
 *
 */
#ifndef WEAVE_CONFIG_ERROR_STR_SIZE
#if WEAVE_CONFIG_SHORT_ERROR_STR
#define WEAVE_CONFIG_ERROR_STR_SIZE                         (5 + 1 + 10 + 3 + 8 + 1)
#else // WEAVE_CONFIG_SHORT_ERROR_STR
#define WEAVE_CONFIG_ERROR_STR_SIZE                         256
#endif // WEAVE_CONFIG_SHORT_ERROR_STR
#endif // WEAVE_CONFIG_ERROR_STR_SIZE

/**
 *  @def WEAVE_CONFIG_CUSTOM_ERROR_FORMATTER
 *
 *  @brief
 *    If asserted (1), suppress definition of the standard error formatting function
 *    (#nl::FormatError()) allowing an application-specific implementation to be used.
 *
 */
#ifndef WEAVE_CONFIG_CUSTOM_ERROR_FORMATTER
#define WEAVE_CONFIG_CUSTOM_ERROR_FORMATTER                 0
#endif // WEAVE_CONFIG_CUSTOM_ERROR_FORMATTER

/**
 *  @def WEAVE_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT
 *
 *  @brief
 *    The printf-style format string used to format error values.
 *
 *  On some platforms, the structure of error values makes them more convenient to
 *  read in either hex or decimal format.  This option can be used to override
 *  the default hex format.
 *
 *  Note that this option only affects short-form error strings (i.e. when
 *  WEAVE_CONFIG_SHORT_ERROR_STR == 1).  Long form error strings always show both hex
 *  and decimal values
 */
#ifndef WEAVE_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT
#define WEAVE_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT          "0x%08" PRIX32
#endif // WEAVE_CONFIG_SHORT_FORM_ERROR_VALUE_FORMAT

/**
 *  @def WEAVE_CONFIG_BLE_PKT_RESERVED_SIZE
 *
 *  @brief
 *    The number of bytes that Weave should reserve at the front of
 *    every outgoing BLE packet for the sake of the underlying BLE
 *    stack.
 *
 */
#ifndef WEAVE_CONFIG_BLE_PKT_RESERVED_SIZE
#define WEAVE_CONFIG_BLE_PKT_RESERVED_SIZE                  0
#endif // WEAVE_CONFIG_BLE_PKT_RESERVED_SIZE

/**
 *  @def WEAVE_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS
 *
 *  @brief
 *    Enable (1) or disable (0) support for utility functions for
 *    decoding and outputing information related to Weave security.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS
#define WEAVE_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS            1
#endif // WEAVE_CONFIG_ENABLE_SECURITY_DEBUG_FUNCS

/**
 *  @def WEAVE_CONFIG_IsPlatformErrorNonCritical(CODE)
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
 *  @param[in]    CODE    The #WEAVE_ERROR being checked for criticality.
 *
 *  @return    true if the error is non-critical; false otherwise.
 *
 */
#ifndef WEAVE_CONFIG_IsPlatformErrorNonCritical
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#define _WEAVE_CONFIG_IsPlatformLwIPErrorNonCritical(CODE)         \
    ((CODE) == nl::Weave::System::MapErrorLwIP(ERR_RTE)         || \
     (CODE) == nl::Weave::System::MapErrorLwIP(ERR_MEM))
#else // !WEAVE_SYSTEM_CONFIG_USE_LWIP
#define _WEAVE_CONFIG_IsPlatformLwIPErrorNonCritical(CODE)  0
#endif // !WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#define _WEAVE_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE)        \
    ((CODE) == nl::Weave::System::MapErrorPOSIX(EHOSTUNREACH)   || \
     (CODE) == nl::Weave::System::MapErrorPOSIX(ENETUNREACH)    || \
     (CODE) == nl::Weave::System::MapErrorPOSIX(EADDRNOTAVAIL)  || \
     (CODE) == nl::Weave::System::MapErrorPOSIX(EPIPE))
#else // !WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#define _WEAVE_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE)  0
#endif // !WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#define WEAVE_CONFIG_IsPlatformErrorNonCritical(CODE)              \
    (_WEAVE_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE)        || \
     _WEAVE_CONFIG_IsPlatformLwIPErrorNonCritical(CODE))
#endif // WEAVE_CONFIG_IsPlatformErrorNonCritical

/**
 *  @def WEAVE_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS
 *
 *  @brief
 *    Enable (1) or disable (0) replacing math functions
 *    which may not be available in the standard/intrinsic library,
 *    and hence require special support from the platform.
 *
 */
#ifndef WEAVE_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS
#define WEAVE_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS   0
#endif // WEAVE_CONFIG_WILL_OVERRIDE_PLATFORM_MATH_FUNCS

/**
 * @def WEAVE_CONFIG_SERIALIZATION_USE_MALLOC
 *
 * @brief If turned on, then schema event serialization and
 *   deserialization will use the stdlib implementations of malloc,
 *   free, and realloc by default (if no other implementations have
 *   been provided).  We will fail at compile time if the stdlib
 *   implementations are not present.
 */
#ifndef WEAVE_CONFIG_SERIALIZATION_USE_MALLOC
#define WEAVE_CONFIG_SERIALIZATION_USE_MALLOC 0
#endif

/**
 * @def WEAVE_CONFIG_SERIALIZATION_DEBUG_LOGGING
 *
 * @brief Enable debug logging for the serialization/deserialization APIs.
 */
#ifndef WEAVE_CONFIG_SERIALIZATION_DEBUG_LOGGING
#define WEAVE_CONFIG_SERIALIZATION_DEBUG_LOGGING 0
#endif

/**
 * @def WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
 *
 * @brief Enable deserialization as well as serialization APIs.  We
 *   make deserialization configurable because it requires some extra
 *   memory that a highly resource-constrained platform could preserve
 *   if it doesn't consume WDM events or otherwise has no need to
 *   deserialize.
 */
#ifndef WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION
#define WEAVE_CONFIG_SERIALIZATION_ENABLE_DESERIALIZATION 1
#endif

/**
 * @def WEAVE_CONFIG_SERIALIZATION_LOG_FLOATS
 *
 * @brief Enable debug logging of floats and doubles for the
 *   serialization/deserialization APIs.  Not all platforms
 *   support these types, and may not compile if there are
 *   any references to them.  Only matters if
 *   WEAVE_CONFIG_SERIALIZATION_DEBUG_LOGGING is enabled.
 */
#ifndef WEAVE_CONFIG_SERIALIZATION_LOG_FLOATS
#define WEAVE_CONFIG_SERIALIZATION_LOG_FLOATS 1
#endif

/**
 * @def WEAVE_CONFIG_PERSISTED_STORAGE_KEY_TYPE
 *
 * @brief
 *   The data type used to represent the key of a persistedly-stored
 *   key/value pair.
 */
#ifndef WEAVE_CONFIG_PERSISTED_STORAGE_KEY_TYPE
#define WEAVE_CONFIG_PERSISTED_STORAGE_KEY_TYPE const char *
#endif

/**
 *  @def WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID
 *
 *  @brief
 *    The group key message counter persisted storage Id.
 *
 */
#ifndef WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID
#define WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID      "EncMsgCntr"
#endif // WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID

/**
 *  @def WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH
 *
 *  @brief
 *    The group key message counter persisted storage epoch.
 *
 */
#ifndef WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH
#define WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH   0x1000
#endif // WEAVE_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH

/**
 * @def WEAVE_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *
 * @brief The maximum length of the key in a key/value pair
 *   stored in the platform's persistent storage.
 */
#ifndef WEAVE_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
#define WEAVE_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 16
#endif

/**
 * @def WEAVE_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH
 *
 * @brief The maximum length of the value in a key/value pair
 *   stored in the platform's persistent storage.
 */
#ifndef WEAVE_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH
#define WEAVE_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH 256
#endif

/**
 * @def WEAVE_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
 *
 * @brief Enable debug logging for the PersistedCounter API.
 */
#ifndef WEAVE_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING
#define WEAVE_CONFIG_PERSISTED_COUNTER_DEBUG_LOGGING 0
#endif

/**
 * @def WEAVE_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
 *
 * @brief Enable verbose debug logging for the EventLogging API.
 * This setting is incompatible with platforms that route console
 * logs into event logging, as it would result in circular logic.
 */
#ifndef WEAVE_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS
#define WEAVE_CONFIG_EVENT_LOGGING_VERBOSE_DEBUG_LOGS 1
#endif

/**
 * @def WEAVE_CONFIG_ENABLE_ARG_PARSER
 *
 * @brief Enable support functions for parsing command-line arguments
 */
#ifndef WEAVE_CONFIG_ENABLE_ARG_PARSER
#define WEAVE_CONFIG_ENABLE_ARG_PARSER 0
#endif

/**
 * @def WEAVE_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
 *
 * @brief Enable santiy checking of command-line argument definitions.
 */
#ifndef WEAVE_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
#define WEAVE_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK 1
#endif

/**
 * @def WEAVE_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT
 *
 * @brief
 *    The amount of time (in milliseconds) which the service is given
 *    to respond to a pair device to account request.
 */
#ifndef WEAVE_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT
#define WEAVE_CONFIG_SERVICE_PROV_RESPONSE_TIMEOUT 60000
#endif

/**
 *  @def WEAVE_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE
 *
 *  @brief
 *    Enable (1) or disable (0) support for the depricated
 *    version of AddNetwork() message in the Network Provisioning
 *    profile.
 *    This option should be enabled to support pairing with Nest
 *    legacy devices that don't have latest SW.
 *
 */
#ifndef WEAVE_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE
#define WEAVE_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE     1
#endif // WEAVE_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE

/**
 *  @def WEAVE_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE
 *
 *  @brief
 *    Enable (1) or disable (0) the exclusive use of the depricated
 *    version of AddNetwork() message in the Network Provisioning
 *    profile.
 *    This option should be enabled when exclusively pairing with Nest
 *    legacy devices that don't have latest SW.
 *    This option requires that
 *    WEAVE_CONFIG_SUPPORT_LEGACY_ADD_NETWORK_MESSAGE is enabled.
 *
 */
#ifndef WEAVE_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE
#define WEAVE_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE     0
#endif // WEAVE_CONFIG_ALWAYS_USE_LEGACY_ADD_NETWORK_MESSAGE

/**
 * @def WEAVE_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN
 *
 * @brief Enable the Service Provisioning profile message
 * for notification of successful in-field joining of the
 * Weave fabric.
 */
#ifndef WEAVE_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN
#define WEAVE_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN         0
#endif // WEAVE_CONFIG_ENABLE_IFJ_SERVICE_FABRIC_JOIN

/**
 * @def WEAVE_NON_PRODUCTION_MARKER
 *
 * @brief Defines the name of a mark symbol whose presence signals that the Weave code
 * includes development/testing features that should never be used in production contexts.
 */
#ifndef WEAVE_NON_PRODUCTION_MARKER
#if (WEAVE_CONFIG_SECURITY_TEST_MODE || \
     WEAVE_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY || \
     WEAVE_CONFIG_SUPPORT_PASSCODE_CONFIG1_TEST_ONLY || \
     (!WEAVE_CONFIG_REQUIRE_AUTH) || \
     WEAVE_FUZZING_ENABLED)
#define WEAVE_NON_PRODUCTION_MARKER WARNING__DO_NOT_SHIP__CONTAINS_NON_PRODUCTION_WEAVE_CODE
#endif
#endif

#ifdef WEAVE_NON_PRODUCTION_MARKER
extern const char WEAVE_NON_PRODUCTION_MARKER[];
#endif

// clang-format on

#endif /* WEAVE_CONFIG_H_ */
