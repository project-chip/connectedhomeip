/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <inet/InetConfig.h>

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
 *    implementation of Matter memory-management functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_MEMORY_MGMT_MALLOC.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_MGMT_PLATFORM
#define CHIP_CONFIG_MEMORY_MGMT_PLATFORM 0
#endif // CHIP_CONFIG_MEMORY_MGMT_PLATFORM

/**
 *  @def CHIP_CONFIG_MEMORY_MGMT_MALLOC
 *
 *  @brief
 *    Enable (1) or disable (0) support for a chip-provided
 *    implementation of Matter memory-management
 *    functions based on the C Standard Library malloc / free
 *    functions.
 *
 *  @note This configuration is mutual exclusive with
 *        #CHIP_CONFIG_MEMORY_MGMT_PLATFORM.
 *
 */
#ifndef CHIP_CONFIG_MEMORY_MGMT_MALLOC
#define CHIP_CONFIG_MEMORY_MGMT_MALLOC 1
#endif // CHIP_CONFIG_MEMORY_MGMT_MALLOC

/**
 *  @}
 */

#if ((CHIP_CONFIG_MEMORY_MGMT_PLATFORM + CHIP_CONFIG_MEMORY_MGMT_MALLOC) != 1)
#error "Please assert exactly one of CHIP_CONFIG_MEMORY_MGMT_PLATFORM or CHIP_CONFIG_MEMORY_MGMT_MALLOC."
#endif // ((CHIP_CONFIG_MEMORY_MGMT_PLATFORM + CHIP_CONFIG_MEMORY_MGMT_MALLOC) != 1)

#if !CHIP_CONFIG_MEMORY_MGMT_MALLOC && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#error "!CHIP_CONFIG_MEMORY_MGMT_MALLOC but getifaddrs() uses malloc()"
#endif

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
 *  @def CHIP_CONFIG_GLOBALS_LAZY_INIT
 *
 *  @brief
 *    Whether to perform chip::Global initialization lazily (1) or eagerly (0).
 *
 *    The default is standard (eager) C++ global initialization behavior.
 */
#ifndef CHIP_CONFIG_GLOBALS_LAZY_INIT
#define CHIP_CONFIG_GLOBALS_LAZY_INIT 0
#endif // CHIP_CONFIG_GLOBALS_LAZY_INIT

/**
 *  @def CHIP_CONFIG_GLOBALS_NO_DESTRUCT
 *
 *  @brief
 *    Whether to omit calling destructors for chip::Global objects.
 *
 *    The default is to call destructors.
 */
#ifndef CHIP_CONFIG_GLOBALS_NO_DESTRUCT
#define CHIP_CONFIG_GLOBALS_NO_DESTRUCT 0
#endif // CHIP_CONFIG_GLOBALS_NO_DESTRUCT

/**
 *  @def CHIP_CONFIG_SHA256_CONTEXT_SIZE
 *
 *  @brief
 *    Size of the statically allocated context for SHA256 operations in CryptoPAL
 *
 *    The default size is based on the Worst software implementation, OpenSSL. A
 *    static assert will tell us if we are wrong, since `typedef SHA_LONG unsigned
 *    int` is default.
 *      SHA_LONG h[8];
 *      SHA_LONG Nl, Nh;
 *      SHA_LONG data[SHA_LBLOCK]; // SHA_LBLOCK is 16 for SHA256
 *      unsigned int num, md_len;
 *
 *    We also have to account for possibly some custom extensions on some targets,
 *    especially for mbedTLS, so an extra sizeof(uint64_t) is added to account.
 *
 */
#ifndef CHIP_CONFIG_SHA256_CONTEXT_SIZE
#define CHIP_CONFIG_SHA256_CONTEXT_SIZE ((sizeof(unsigned int) * (8 + 2 + 16 + 2)) + sizeof(uint64_t))
#endif // CHIP_CONFIG_SHA256_CONTEXT_SIZE

/**
 *  @def CHIP_CONFIG_SHA256_CONTEXT_ALIGN
 *
 * @brief The alignment of SHA256 context buffer.
 */
#ifndef CHIP_CONFIG_SHA256_CONTEXT_ALIGN
#define CHIP_CONFIG_SHA256_CONTEXT_ALIGN size_t
#endif // CHIP_CONFIG_SHA256_CONTEXT_ALIGN

/**
 *  @def CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE
 *
 *  @brief
 *    Size of the statically allocated context for the HKDF key handle in CryptoPAL.
 *
 *  The default size is selected so that the key handle is able to fit 256-bit raw key
 *  material along with the size information.
 */
#ifndef CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE
#define CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE (32 + 1)
#endif // CHIP_CONFIG_HKDF_KEY_HANDLE_CONTEXT_SIZE

/**
 * @def CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
 *
 * @brief
 *   Base of the PSA key identifier range used by Matter.
 *
 * Cryptographic keys stored in the PSA Internal Trusted Storage must have
 * a user-assigned identifer from the range PSA_KEY_ID_USER_MIN to
 * PSA_KEY_ID_USER_MAX. This option allows to override the base used to derive
 * key identifiers used by Matter to avoid overlapping with other firmware
 * components that also use PSA crypto API. The default value was selected
 * not to interfere with OpenThread's default base that is 0x20000.
 *
 * Note that volatile keys like ephemeral keys used for ECDH have identifiers
 * auto-assigned by the PSA backend.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE
#define CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE 0x30000
#endif // CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE

/**
 * @def CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END
 *
 * @brief
 *   End of the PSA key identifier range used by Matter.
 *
 * This setting establishes the maximum limit for the key range specific to Matter, in order to
 * prevent any overlap with other firmware components that also employ the PSA crypto API.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END
#define CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END 0x3FFFF
#endif // CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END

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
 *  @def CHIP_PORT
 *
 *  @brief
 *    chip TCP/UDP port for secured chip traffic.
 *
 */
#ifndef CHIP_PORT
#define CHIP_PORT 5540
#endif // CHIP_PORT

/**
 *  @def CHIP_UDC_PORT
 *
 *  @brief
 *    chip TCP/UDP port on commissioner for unsecured user-directed-commissioning traffic.
 *
 */
#ifndef CHIP_UDC_PORT
#define CHIP_UDC_PORT CHIP_PORT + 10
#endif // CHIP_UDC_PORT

/**
 *  @def CHIP_UDC_COMMISSIONEE_PORT
 *
 *  @brief
 *    chip TCP/UDP port on commisionee for unsecured user-directed-commissioning traffic.
 *
 */
#ifndef CHIP_UDC_COMMISSIONEE_PORT
#define CHIP_UDC_COMMISSIONEE_PORT CHIP_UDC_PORT + 10
#endif // CHIP_UDC_COMMISSIONEE_PORT

/**
 *  @def CHIP_CONFIG_SECURITY_TEST_MODE
 *
 *  @brief
 *    Enable various features that make it easier to debug secure chip communication.
 *
 *  @note
 *    WARNING: This option makes it possible to circumvent basic chip security functionality,
 *    including message encryption. Because of this it SHOULD NEVER BE ENABLED IN PRODUCTION BUILDS.
 *
 *    To build with this flag, pass 'treat_warnings_as_errors=false' to gn/ninja.
 */
#ifndef CHIP_CONFIG_SECURITY_TEST_MODE
#define CHIP_CONFIG_SECURITY_TEST_MODE 0
#endif // CHIP_CONFIG_SECURITY_TEST_MODE

/**
 *  @def CHIP_CONFIG_TEST_SHARED_SECRET_VALUE
 *
 *  @brief
 *    Shared secret to use for unit tests or when CHIP_CONFIG_SECURITY_TEST_MODE is enabled.
 *
 *    This parameter is 32 bytes to maximize entropy passed to the CryptoContext::InitWithSecret KDF,
 *    and can be initialized either as a raw string or array of bytes. The default test secret of
 *    "Test secret for key derivation." results in the following encryption keys:
 *
 *              5E DE D2 44 E5 53 2B 3C DC 23 40 9D BA D0 52 D2
 *              A9 E0 11 B1 73 7C 6D 4B 70 E4 C0 A2 FE 66 04 76
 */
#ifndef CHIP_CONFIG_TEST_SHARED_SECRET_VALUE
#define CHIP_CONFIG_TEST_SHARED_SECRET_VALUE "Test secret for key derivation."
#endif // CHIP_CONFIG_TEST_SHARED_SECRET_VALUE

/**
 *  @def CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH
 *
 *  @brief
 *    Length of the shared secret to use for unit tests or when CHIP_CONFIG_SECURITY_TEST_MODE is enabled.
 *
 *    Note that the default value of 32 includes the null terminator.
 *    WARNING: `strlen(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE)` will result in different keys
 *              than expected and give unexpected results for shared secrets that contain '\x00'.
 */
#ifndef CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH
#define CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH 32
#endif // CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH

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
 *  @def CHIP_AUTOMATION_LOGGING
 *
 *  @brief
 *    If asserted (1), enable logging of all messages in the
 *    chip::Logging::kLogCategory_Automation category.
 *
 */
#ifndef CHIP_AUTOMATION_LOGGING
#define CHIP_AUTOMATION_LOGGING 1
#endif // CHIP_AUTOMATION_LOGGING

/**
 *  @def CHIP_LOG_FILTERING
 *
 *  @brief
 *    If asserted (1), enable runtime log level configuration.
 */
#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 1
#endif

/**
 * CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE
 *
 * The maximum size (in bytes) of a log message
 */
#ifndef CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE
#define CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE 256
#endif

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
 *  @def CHIP_CONFIG_ERROR_SOURCE
 *
 *  If asserted (1), then CHIP_ERROR constants will include the source location of their expansion.
 */
#ifndef CHIP_CONFIG_ERROR_SOURCE
#define CHIP_CONFIG_ERROR_SOURCE 0
#endif // CHIP_CONFIG_ERROR_SOURCE

/**
 *  @def CHIP_CONFIG_ERROR_SOURCE_NO_ERROR
 *
 *  If asserted (1) along with CHIP_CONFIG_ERROR_SOURCE, then instances of CHIP_NO_ERROR will also include
 *  the source location of their expansion. Otherwise, CHIP_NO_ERROR is excluded from source tracking.
 */
#ifndef CHIP_CONFIG_ERROR_SOURCE_NO_ERROR
#define CHIP_CONFIG_ERROR_SOURCE_NO_ERROR 1
#endif // CHIP_CONFIG_ERROR_SOURCE

/**
 *  @def CHIP_CONFIG_ERROR_FORMAT_AS_STRING
 *
 *  If 0, then ChipError::Format() returns an integer (ChipError::StorageType).
 *  If 1, then ChipError::Format() returns a const char *, from chip::ErrorStr().
 *  In either case, the macro CHIP_ERROR_FORMAT expands to a suitable printf format.
 */

#ifndef CHIP_CONFIG_ERROR_FORMAT_AS_STRING
#define CHIP_CONFIG_ERROR_FORMAT_AS_STRING 0
#endif // CHIP_CONFIG_ERROR_FORMAT_AS_STRING

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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#define _CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE)                                                                         \
    ((CODE) == CHIP_ERROR_POSIX(EHOSTUNREACH) || (CODE) == CHIP_ERROR_POSIX(ENETUNREACH) ||                                        \
     (CODE) == CHIP_ERROR_POSIX(EADDRNOTAVAIL) || (CODE) == CHIP_ERROR_POSIX(EPIPE))
#else // !(CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)
#define _CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE) 0
#endif // !(CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK)

#define CHIP_CONFIG_IsPlatformErrorNonCritical(CODE)                                                                               \
    (_CHIP_CONFIG_IsPlatformPOSIXErrorNonCritical(CODE) || _CHIP_CONFIG_IsPlatformLwIPErrorNonCritical(CODE))
#endif // CHIP_CONFIG_IsPlatformErrorNonCritical

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
 * @def CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *
 * @brief The maximum length of the key in a key/value pair
 *   stored in the platform's persistent storage.
 */
#ifndef CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
#define CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH 16
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
 * @def CHIP_CONFIG_ENABLE_ARG_PARSER_VALIDITY_CHECKS
 *
 * @brief Enable validity checking of command-line argument definitions.
 *
 * // TODO: Determine why we wouldn't need this
 */
#ifndef CHIP_CONFIG_ENABLE_ARG_PARSER_VALIDITY_CHECKS
#define CHIP_CONFIG_ENABLE_ARG_PARSER_VALIDITY_CHECKS 1
#endif

/**
 * @def CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE
 *
 * @brief Define the size of the pool used for tracking Matter unauthenticated
 * message states. The entries in the pool are automatically rotated by LRU. The size
 * of the pool limits how many PASE and CASE pairing sessions can be processed
 * simultaneously.
 */
#ifndef CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE
#define CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE 4
#endif // CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE

/**
 * @def CHIP_CONFIG_SECURE_SESSION_REFCOUNT_LOGGING
 *
 * @brief This enables logging of changes to the underlying reference count of
 * SecureSession objects.
 *
 */
#ifndef CHIP_CONFIG_SECURE_SESSION_REFCOUNT_LOGGING
#define CHIP_CONFIG_SECURE_SESSION_REFCOUNT_LOGGING 0
#endif

/**
 *  @def CHIP_CONFIG_MAX_FABRICS
 *
 *  @brief
 *    Maximum number of fabrics the device can participate in.  Each fabric can
 *    provision the device with its unique operational credentials and manage
 *    its own access control lists.
 */
#ifndef CHIP_CONFIG_MAX_FABRICS
#define CHIP_CONFIG_MAX_FABRICS 16
#endif // CHIP_CONFIG_MAX_FABRICS

/**
 * @def CHIP_CONFIG_SECURE_SESSION_POOL_SIZE
 *
 * @brief Defines the size of the pool used for tracking the state of
 * secure sessions. This controls the maximum number of concurrent
 * established secure sessions across all supported transports.
 *
 * This is sized by default to cover the sum of the following:
 *  - At least 3 CASE sessions / fabric (Spec Ref: 4.13.2.8)
 *  - 1 reserved slot for CASEServer as a responder.
 *  - 1 reserved slot for PASE.
 *
 *  NOTE: On heap-based platforms, there is no pre-allocation of the pool.
 *  Due to the use of an LRU-scheme to manage sessions, the actual active
 *  size of the pool will grow up to the value of this define,
 *  after which, it will remain at or around this size indefinitely.
 *
 */
#ifndef CHIP_CONFIG_SECURE_SESSION_POOL_SIZE
#define CHIP_CONFIG_SECURE_SESSION_POOL_SIZE (CHIP_CONFIG_MAX_FABRICS * 3 + 2)
#endif // CHIP_CONFIG_SECURE_SESSION_POOL_SIZE

/**
 *  @def CHIP_CONFIG_MAX_GROUP_DATA_PEERS
 *
 *  @brief
 *    Maximum number of Peer within a fabric that can send group data message to a device.
 *
 *  // TODO: Determine a better value for this
 */
#ifndef CHIP_CONFIG_MAX_GROUP_DATA_PEERS
#define CHIP_CONFIG_MAX_GROUP_DATA_PEERS 15
#endif // CHIP_CONFIG_MAX_GROUP_DATA_PEERS

/**
 *  @def CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS
 *
 *  @brief
 *   Maximum number of Peer within a fabric that can send group control message to a device.
 */
#ifndef CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS
#define CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS 2
#endif // CHIP_CONFIG_MAX_GROUP_CONTROL_PEER

/**
 *  @def CHIP_CONFIG_SLOW_CRYPTO
 *
 *  @brief
 *   When enabled, CASE and PASE setup will proactively send standalone acknowledgements
 *   prior to engaging in crypto operations.
 */
#ifndef CHIP_CONFIG_SLOW_CRYPTO
#define CHIP_CONFIG_SLOW_CRYPTO 1
#endif // CHIP_CONFIG_SLOW_CRYPTO

/**
 * @def CHIP_NON_PRODUCTION_MARKER
 *
 * @brief Defines the name of a mark symbol whose presence signals that the chip code
 * includes development/testing features that should never be used in production contexts.
 */
#ifndef CHIP_NON_PRODUCTION_MARKER
#if (CHIP_CONFIG_SECURITY_TEST_MODE || CHIP_FUZZING_ENABLED)
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

/**
 *  @name Interaction Model object pool configuration.
 *
 *  @brief
 *    The following definitions sets the maximum number of corresponding interaction model object pool size.
 *
 *      * #CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *      * #CHIP_IM_MAX_NUM_READS
 *      * #CHIP_IM_MAX_NUM_SUBSCRIPTIONS
 *      * #CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS
 *      * #CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS
 *      * #CHIP_IM_MAX_REPORTS_IN_FLIGHT
 *      * #CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS
 *      * #CHIP_IM_SERVER_MAX_NUM_DIRTY_SET
 *      * #CHIP_IM_MAX_NUM_WRITE_HANDLER
 *      * #CHIP_IM_MAX_NUM_WRITE_CLIENT
 *      * #CHIP_IM_MAX_NUM_TIMED_HANDLER
 *
 *  @{
 */

/**
 * @def CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *
 * @brief Defines the maximum number of CommandHandler, limits the number of active commands transactions on server.
 */
#ifndef CHIP_IM_MAX_NUM_COMMAND_HANDLER
#define CHIP_IM_MAX_NUM_COMMAND_HANDLER 4
#endif

/**
 * @def CHIP_IM_MAX_NUM_SUBSCRIPTIONS
 *
 * @brief Defines the maximum number of ReadHandler for subscriptions, limits the number of active subscription transactions on
 * server.
 *
 * The default value comes from 3sub per fabric * max number of fabrics.
 *
 */
#ifndef CHIP_IM_MAX_NUM_SUBSCRIPTIONS
#define CHIP_IM_MAX_NUM_SUBSCRIPTIONS (CHIP_CONFIG_MAX_FABRICS * 3)
#endif

/**
 * @def CHIP_IM_MAX_NUM_READS
 *
 * @brief Defines the maximum number of ReadHandler for read transactions, limits the number of active read transactions on
 * server.
 *
 * The default value is one per fabric * max number of fabrics.
 */
#ifndef CHIP_IM_MAX_NUM_READS
#define CHIP_IM_MAX_NUM_READS (CHIP_CONFIG_MAX_FABRICS)
#endif

/**
 * @def CHIP_IM_MAX_REPORTS_IN_FLIGHT
 *
 * @brief Defines the maximum number of Reports, limits the traffic of read and subscription transactions.
 */
#ifndef CHIP_IM_MAX_REPORTS_IN_FLIGHT
#define CHIP_IM_MAX_REPORTS_IN_FLIGHT 4
#endif

/**
 * @def CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS
 *
 * @brief The maximum number of path objects for subscriptions, limits the number of attributes being subscribed at the same time.
 */
#ifndef CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS
#define CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS (CHIP_IM_MAX_NUM_SUBSCRIPTIONS * 3)
#endif

/**
 * @def CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS
 *
 * @brief Defines the maximum number of path objects for read requests.
 */
#ifndef CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS
#define CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS (CHIP_IM_MAX_NUM_READS * 9)
#endif

/**
 * @def CHIP_IM_SERVER_MAX_NUM_DIRTY_SET
 *
 * @brief Defines the maximum number of dirty set, limits the number of attributes being read or subscribed at the same time.
 */
#ifndef CHIP_IM_SERVER_MAX_NUM_DIRTY_SET
#define CHIP_IM_SERVER_MAX_NUM_DIRTY_SET 8
#endif

/**
 * @def CHIP_IM_MAX_NUM_WRITE_HANDLER
 *
 * @brief Defines the maximum number of WriteHandler, limits the number of active write transactions on server.
 */
#ifndef CHIP_IM_MAX_NUM_WRITE_HANDLER
#define CHIP_IM_MAX_NUM_WRITE_HANDLER 4
#endif

/**
 * @def CHIP_IM_MAX_NUM_WRITE_CLIENT
 *
 * @brief Defines the maximum number of WriteClient, limits the number of active write transactions on client.
 */
#ifndef CHIP_IM_MAX_NUM_WRITE_CLIENT
#define CHIP_IM_MAX_NUM_WRITE_CLIENT 4
#endif

/**
 * @def CHIP_IM_MAX_NUM_TIMED_HANDLER
 *
 * @brief Defines the maximum number of TimedHandler, limits the number of
 *        active timed interactions waiting for the Invoke or Write.
 */
#ifndef CHIP_IM_MAX_NUM_TIMED_HANDLER
#define CHIP_IM_MAX_NUM_TIMED_HANDLER 8
#endif

/**
 * @}
 */

/**
 * @def CONFIG_BUILD_FOR_HOST_UNIT_TEST
 *
 * @brief Defines whether we're currently building for unit testing, which enables a set of features
 *        that are only utilized in those tests. This flag should not be enabled on devices. If you have a test
 *        that uses this flag, either appropriately conditionalize the entire test on this flag, or to exclude
 *        the compliation of that test source file entirely.
 */
#ifndef CONFIG_BUILD_FOR_HOST_UNIT_TEST
#define CONFIG_BUILD_FOR_HOST_UNIT_TEST 0
#endif

/**
 * @def CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES
 *
 * @brief Defines whether encoding the "not a known enum value" enum values is
 *        allowed.  Should only be enabled in certain test applications.  This
 *        flag must not be enabled on actual devices.
 */
#ifndef CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES
#define CHIP_CONFIG_IM_ENABLE_ENCODING_SENTINEL_ENUM_VALUES 0
#endif

/**
 * @def CHIP_CONFIG_LAMBDA_EVENT_SIZE
 *
 * @brief The maximum size of the lambda which can be post into system event queue.
 */
#ifndef CHIP_CONFIG_LAMBDA_EVENT_SIZE
#define CHIP_CONFIG_LAMBDA_EVENT_SIZE (24)
#endif

/**
 * @def CHIP_CONFIG_LAMBDA_EVENT_ALIGN
 *
 * @brief The maximum alignment of the lambda which can be post into system event queue.
 */
#ifndef CHIP_CONFIG_LAMBDA_EVENT_ALIGN
#define CHIP_CONFIG_LAMBDA_EVENT_ALIGN (sizeof(void *))
#endif

/**
 * @def CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
 *
 * @brief If true, VerifyOrDie() calls with no message will use an
 *        automatically generated message that makes it clear what failed.
 */
#ifndef CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
#define CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE 0
#endif

/**
 * @def CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE_NO_COND
 *
 * @brief If true, VerifyOrDie() built with @c CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE
 *        generates a short message that includes only the source code location,
 *        without the condition that fails.
 */
#ifndef CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE_NO_COND
#define CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE_NO_COND 0
#endif

/**
 * @def CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES
 *
 * @brief Number of devices a controller can be simultaneously connected to
 */
#ifndef CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES
#define CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES 64
#endif

/**
 * @def CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_CASE_CLIENTS
 *
 * @brief Number of outgoing CASE sessions can be simutaneously negotiated.
 */
#ifndef CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_CASE_CLIENTS
#define CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_CASE_CLIENTS 16
#endif

/**
 * @def CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS
 *
 * @brief Number of outgoing CASE sessions can be simutaneously negotiated on an end device.
 */
#ifndef CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS
#define CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS 2
#endif

/**
 * @def CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES
 *
 * @brief Number of devices to which the `Server` implementers will be able to
 *        concurrently connect over CASE and interact with. Relates to OTA
 *        requestor queries and bindings that can be operated. This is *global*,
 *        not *per fabric*.
 *
 * TODO: Determine how to better explain this.
 */
#ifndef CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES
#define CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES 4
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC
 *
 * @brief Defines the number of "endpoint->controlling group" mappings per fabric.
 *
 * Binds to number of GroupMapping entries per fabric
 */
#ifndef CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC 1
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
 *
 * @brief Defines the number of groups supported per fabric, see Group Key Management Cluster in specification.
 *
 * Binds to number of GroupState entries to support per fabric
 */
#ifndef CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUPS_PER_FABRIC (4 * CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC)
#endif

#if CHIP_CONFIG_MAX_GROUPS_PER_FABRIC < (4 * CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC)
#error "Please ensure CHIP_CONFIG_MAX_GROUPS_PER_FABRIC meets minimum requirements. See Group Limits in the specification."
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC
 *
 * @brief Defines the number of groups key sets supported per fabric, see Group Key Management Cluster in specification.
 *
 * Binds to number of KeySet entries to support per fabric (Need at least 1 for Identity Protection Key)
 */
#ifndef CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC 3
#endif

#if CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC < 1
#error "Please ensure CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC > 0 to support at least the IPK."
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS
 *
 * @brief Defines the number of simultaneous Group iterators that can be allocated
 *
 * Number of iterator instances that can be allocated at any one time
 */
#ifndef CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS
#define CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS 2
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUP_NAME_LENGTH
 *
 * @brief Defines the maximum length of the group names
 */
#ifndef CHIP_CONFIG_MAX_GROUP_NAME_LENGTH
#define CHIP_CONFIG_MAX_GROUP_NAME_LENGTH 16
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC
 *
 * Defines the number of access control entries supported per fabric in the
 * example access control code.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC 4
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY
 *
 * Defines the number of access control subjects supported per entry in the
 * example access control code.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY 4
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY
 *
 * Defines the number of access control targets supported per entry in the
 * example access control code.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY 3
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_STORAGE_POOL_SIZE
 *
 * Defines the entry storage pool size in the example access control code.
 * It's possible to get by with only one.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_STORAGE_POOL_SIZE
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_STORAGE_POOL_SIZE 1
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_DELEGATE_POOL_SIZE
 *
 * Defines the entry delegate pool size in the example access control code.
 * It's possible to get by with only one.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_DELEGATE_POOL_SIZE
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_DELEGATE_POOL_SIZE 1
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_ITERATOR_DELEGATE_POOL_SIZE
 *
 * Defines the entry iterator delegate pool size in the example access control code.
 * It's possible to get by with only one.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_ITERATOR_DELEGATE_POOL_SIZE
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_ITERATOR_DELEGATE_POOL_SIZE 1
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT
 *
 * Support fast copy in the example access control implementation.
 *
 * At least one of "fast" or "flexible" copy support must be enabled.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT 1
#endif

/**
 * @def CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT
 *
 * Support flexible copy in the example access control implementation.
 *
 * Only needed if mixing the example access control implementation with other
 * non-example access control delegate implementations; omitting it saves space.
 *
 * At least one of "fast" or "flexible" copy support must be enabled.
 */
#ifndef CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT 0
#endif

#if !CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT && !CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT
#error                                                                                                                             \
    "Please enable at least one of CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT or CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT"
#endif

/**
 * @def CHIP_CONFIG_ACCESS_RESTRICTION_MAX_ENTRIES_PER_FABRIC
 *
 * Defines the maximum number of access restriction list entries per
 * fabric in the access control code's ARL attribute.
 */
#ifndef CHIP_CONFIG_ACCESS_RESTRICTION_MAX_ENTRIES_PER_FABRIC
#define CHIP_CONFIG_ACCESS_RESTRICTION_MAX_ENTRIES_PER_FABRIC 10
#endif

/**
 * @def CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY
 *
 * Defines the maximum number of access restrictions for each entry
 * in the ARL attribute (each entry is for a specific cluster on an
 * endpoint on a fabric).
 */
#ifndef CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY
#define CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY 10
#endif

/**
 * @def CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE
 *
 * @brief
 *   Maximum number of CASE sessions that a device caches, that can be resumed
 */
#ifndef CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE
#define CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE (3 * CHIP_CONFIG_MAX_FABRICS)
#endif

/**
 * @def CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
 *
 * @brief The number of bytes written to the event logging system that
 *   will trigger Report Delivery.
 *
 * The configuration captures the number of bytes written to the event
 * logging subsystem needed to trigger a report. For example, if an application wants to offload all DEBUG events
 * reliably, the threshold should be set to less than the size of the
 * DEBUG buffer (plus a slop factor to account for events generated
 * during the scheduling and event offload).  Similarly, if the
 * application does not want to drop INFO events, the threshold should
 * be set to the sum of DEBUG and INFO buffers (with the same
 * correction).
 *
 */
#ifndef CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD
#define CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD 512
#endif /* CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD */

/**
 * @def CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
 *
 * @brief Enable Interaction model Event support in server
 */
#ifndef CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
#define CHIP_CONFIG_ENABLE_SERVER_IM_EVENT 1
#endif

/**
 * Accepts receipt of invalid privacy flag usage that affected some early SVE2 test event implementations.
 * When SVE2 started, group messages would be sent with the privacy flag enabled, but without privacy encrypting the message header.
 * The issue was subsequently corrected in master, the 1.0 branch, and the SVE2 branch.
 * This is a temporary workaround for interoperability with those erroneous early-SVE2 implementations.
 * The cost of this compatibity mode is twice as many decryption steps per received group message.
 *
 * TODO(#24573): Remove this workaround once interoperability with legacy pre-SVE2 is no longer required.
 */
#ifndef CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2
#define CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2 1
#endif // CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2

/**
 *  @def CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS
 *
 *  @brief
 *    If auto resubscribe is enabled & default resubscription policy is used,
 *    specify the max wait time.
 *    This value was chosen so that the average wait time is 3600000
 *    ((100 - CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP) % of CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS) / 2 +
 *    (CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP % of CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS) = average wait is
 * 3600000
 */
#ifndef CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS
#define CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS 5538000
#endif

/**
 *  @def CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX
 *
 *  @brief
 *    If auto resubscribe is enabled & default resubscription policy is used,
 *    specify the max fibonacci step index.
 *    This index must satisfy below conditions:
 *    1 . Fibonacci(CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX + 1) * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS >
 * CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS 2 . Fibonacci(CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX) *
 * CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS < CHIP_RESUBSCRIBE_MAX_RETRY_WAIT_INTERVAL_MS
 *
 */
#ifndef CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX
#define CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX 14
#endif

/**
 *  @def CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP
 *
 *  @brief
 *    If auto resubscribe is enabled & default resubscription policy is used,
 *    specify the minimum wait
 *    time as a percentage of the max wait interval for that step.
 *
 */
#ifndef CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP
#define CHIP_RESUBSCRIBE_MIN_WAIT_TIME_INTERVAL_PERCENT_PER_STEP 30
#endif

/**
 *  @def CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS
 *
 *  @brief
 *    If auto resubscribe is enabled & default resubscription policy is used,
 *    specify the multiplier that multiplies the result of a fibonacci computation
 *    based on a specific index to provide a max wait time for
 *    a step.
 *
 */
#ifndef CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS
#define CHIP_RESUBSCRIBE_WAIT_TIME_MULTIPLIER_MS 10000
#endif

/*
 * @def CHIP_CONFIG_MAX_ATTRIBUTE_STORE_ELEMENT_SIZE
 *
 * @brief Safety limit to ensure that we don't end up with a
 * larger-than-expected buffer for temporary attribute storage (on the stack or
 * in .bss).  The SDK will fail to compile if this value is set below the value
 * it thinks it needs for a buffer size that can store any simple (not list or
 * struct) attribute value.
 */
#ifndef CHIP_CONFIG_MAX_ATTRIBUTE_STORE_ELEMENT_SIZE
// I can't figure out how to get all-clusters-app to sanely use a different
// value here, and that app includes TestCluster, which has very large string
// attributes (1000 octets, leading to a 1003 octet buffer).
#define CHIP_CONFIG_MAX_ATTRIBUTE_STORE_ELEMENT_SIZE 1003
#endif // CHIP_CONFIG_MAX_ATTRIBUTE_STORE_ELEMENT_SIZE

/*
 * @def CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST
 *
 * @brief Enables usage of heap in the minmdns DNSSD implementation
 *        for tracking active operational responder lists.
 *
 *        When this is not set, CHIP_CONFIG_MAX_FABRICS is used to determine
 *        and statically allocate pointers needed to track active
 *        operational responder lists.
 */
#ifndef CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST
#define CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST 0
#endif // CHIP_CONFIG_MINMDNS_DYNAMIC_OPERATIONAL_RESPONDER_LIST

/*
 * @def CHIP_CONFIG_MINMDNS_MAX_PARALLEL_RESOLVES
 *
 * @brief Determines the maximum number of SRV records that can be processed in parallel.
 *        Affects maximum number of results received for browse requests
 *        (where a single packet may contain multiple SRV entries)
 *        or number of pending resolves that still require a AAAA IP record
 *        to be resolved.
 */
#ifndef CHIP_CONFIG_MINMDNS_MAX_PARALLEL_RESOLVES
#define CHIP_CONFIG_MINMDNS_MAX_PARALLEL_RESOLVES 2
#endif // CHIP_CONFIG_MINMDNS_MAX_PARALLEL_RESOLVES

/**
 * def CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS
 *
 * @brief Determines the maximum number of node resolve results (PeerAddresses) to keep
 *        for establishing an operational session.
 *
 */
#ifndef CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS
#define CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS 1
#endif // CHIP_CONFIG_MDNS_RESOLVE_LOOKUP_RESULTS

/**
 * @def CHIP_CONFIG_ADDRESS_RESOLVE_MIN_LOOKUP_TIME_MS
 *
 * @brief Default minimum lookup time to wait during address resolve for
 *        additional DNSSD queries even if a reply has already been received, or
 *        to allow for additional heuristics regarding node choice to succeed, in
 *        milliseconds
 */
#ifndef CHIP_CONFIG_ADDRESS_RESOLVE_MIN_LOOKUP_TIME_MS
#define CHIP_CONFIG_ADDRESS_RESOLVE_MIN_LOOKUP_TIME_MS 200
#endif // CHIP_CONFIG_ADDRESS_RESOLVE_MIN_LOOKUP_TIME_MS

/**
 * @def CHIP_CONFIG_ADDRESS_RESOLVE_MAX_LOOKUP_TIME_MS
 *
 * @brief Default maximum lookup time to wait during address resolve before
 *        a TIMEOUT error, in milliseconds
 */
#ifndef CHIP_CONFIG_ADDRESS_RESOLVE_MAX_LOOKUP_TIME_MS
#define CHIP_CONFIG_ADDRESS_RESOLVE_MAX_LOOKUP_TIME_MS 45000
#endif // CHIP_CONFIG_ADDRESS_RESOLVE_MAX_LOOKUP_TIME_MS

/*
 * @def CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
 *
 * @brief This buffer will used for holding debug text when handling synchronous invokes (AddOrUpdate / Reorder / Remove). Since we
 * don't set this value for most cases and on most platforms, this value can be 0 to always ignore such field.
 */
#ifndef CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE
#define CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE 64
#endif // CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE

/**
 *  @def CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
 *
 *  If 1, IM status codes, when logged, will be formatted as "0xNN (NameOfCode)"
 *  If 0, IM status codes, when logged, will be formatted as "0xNN" In either
 *  case, the macro ChipLogFormatIMStatus expands to a suitable printf format
 *  string, which already includes the '%' in it, to be used with
 *  ChipLogValueIMStatus(status).
 */

#ifndef CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
#define CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT 0
#endif // CHIP_CONFIG_ERROR_FORMAT_AS_STRING

/**
 *  @def CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS
 *
 *  @brief
 *    This is the default timeout for the discovery of devices by
 *    the setup code pairer.
 *
 */
#ifndef CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS
#define CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS 30
#endif // CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS

/**
 * @def CHIP_CONFIG_NUM_CD_KEY_SLOTS
 *
 * @brief Number of custom CD signing keys supported by default CD keystore
 *
 */
#ifndef CHIP_CONFIG_NUM_CD_KEY_SLOTS
#define CHIP_CONFIG_NUM_CD_KEY_SLOTS 5
#endif // CHIP_CONFIG_NUM_CD_KEY_SLOTS

/**
 * @def CHIP_CONFIG_MAX_SUBSCRIPTION_RESUMPTION_STORAGE_CONCURRENT_ITERATORS
 *
 * @brief Defines the number of simultaneous subscription resumption iterators that can be allocated
 *
 * Number of iterator instances that can be allocated at any one time
 */
#ifndef CHIP_CONFIG_MAX_SUBSCRIPTION_RESUMPTION_STORAGE_CONCURRENT_ITERATORS
#define CHIP_CONFIG_MAX_SUBSCRIPTION_RESUMPTION_STORAGE_CONCURRENT_ITERATORS 2
#endif

/**
 * @brief Maximum length of Scene names
 */
#ifndef CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH
#define CHIP_CONFIG_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH 16
#endif

/**
 * @brief The maximum number of attribute value pairs in an extension field set.
 */
#ifndef CHIP_CONFIG_SCENES_MAX_AV_PAIRS_EFS
#define CHIP_CONFIG_SCENES_MAX_AV_PAIRS_EFS 15
#endif

/**
 * @brief The maximum number of clusters per scene, we recommend using 4 for a typical use case (onOff + level control + color
 * control cluster + mode selec cluster). Needs to be changed in case a greater number of clusters is chosen. In the event the
 * device does not need to support the mode select cluster, the maximum number of clusters per scene should be set to 3.
 */
#ifndef CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE
#define CHIP_CONFIG_SCENES_MAX_CLUSTERS_PER_SCENE 4
#endif

/**
 * @brief The maximum size of a single extension field set for a single cluster
 */
#ifndef CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER
#define CHIP_CONFIG_SCENES_MAX_EXTENSION_FIELDSET_SIZE_PER_CLUSTER 128
#endif

/**
 * @brief The maximum number bytes taken by a scene. This needs to be increased if the number of clusters per scene is increased.
 * @note The default number (256) is based on the assumption that the maximum number of clusters per scene is 3 and that those
 * clusers are onOff, level control and color control cluster.
 * @warning Changing this value will not only affect the RAM usage of a scene but also the size of the scene table in the flash.
 *  A scene's size can be calculated based on the following structure:
 *  Scene TLV (struct)
 *  {
 *  	2 bytes GroupID,
 *  	1 byte SceneID,
 *  	0 - 16 bytes SceneName,
 *  	4 bytes Transition time in miliseconds,
 *
 *  	Extension field sets TLV (array)
 *  	[
 *  		EFS TLV (struct)
 *  		{
 *  			4 bytes for the cluster ID,
 *  			Attribute Value List TLV (array)
 *  			[
 *  				AttributeValue Pair TLV (struct)
 *  				{
 *  					Attribute ID
 *  					4 bytes attributeID,
 *  					AttributeValue
 *  					1 - 8 bytes AttributeValue,
 *  				},
 *  				.
 *  				.
 *  				.
 *
 *  			],
 *
 *  		},
 *  		.
 *  		.
 *  		.
 *  	],
 *  }
 *
 *  Including all the TLV fields, the following values can help estimate the needed size for a scenes given a number of clusters:
 *  Empty EFS Scene Max name size: 37 bytes
 *  Scene Max name size + OnOff : 55 bytes
 *  Scene Max name size + LevelControl : 64 bytes
 *  Scene Max name size + ColorControl : 130 bytes
 *  Scene Max name size + OnOff + LevelControl + ColoControl : 175 bytes
 *
 *  Cluster Sizes:
 *  OnOff Cluster Max Size: 21 bytes
 *  LevelControl Cluster Max Size: 30 bytes
 *  Color Control Cluster Max Size: 96 bytes
 * */
#ifndef CHIP_CONFIG_SCENES_MAX_SERIALIZED_SCENE_SIZE_BYTES
#define CHIP_CONFIG_SCENES_MAX_SERIALIZED_SCENE_SIZE_BYTES 256
#endif

/**
 * @def CHIP_CONFIG_MAX_SCENES_CONCURRENT_ITERATORS
 *
 * @brief Defines the number of simultaneous Scenes iterators that can be allocated
 *
 * Number of iterator instances that can be allocated at any one time
 */
#ifndef CHIP_CONFIG_MAX_SCENES_CONCURRENT_ITERATORS
#define CHIP_CONFIG_MAX_SCENES_CONCURRENT_ITERATORS 2
#endif

/**
 * @def CHIP_CONFIG_MAX_SCENES_TABLE_SIZE
 *
 * @brief This defines how many scenes a single endpoint is allowed to allocate in flash memory. This value MUST at least 16
 * per spec and MUST be increased to allow for configuring a greater scene table size from Zap.
 */
#ifndef CHIP_CONFIG_MAX_SCENES_TABLE_SIZE
#if CHIP_CONFIG_TEST
#define CHIP_CONFIG_MAX_SCENES_TABLE_SIZE 24
#else
#define CHIP_CONFIG_MAX_SCENES_TABLE_SIZE 16
#endif // CHIP_CONFIG_TEST
#endif // CHIP_CONFIG_MAX_SCENES_TABLE_SIZE

/**
 * @def CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
 *
 * @brief This define enables the automatic registration of the default scene handlers in the scene table for each sceneable
 * cluster. If a user wants to use their own scene handlers, they can disable this flag and implement their own handlers. They can
 * use ScenesServer::Instance().RegisterSceneHandler() to have their handlers called when a scene is recalled or stored.
 */
#ifndef CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS
#define CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS 1
#endif // CHIP_CONFIG_SCENES_USE_DEFAULT_HANDLERS

/**
 * @def CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE
 *
 * Defines the size of the time zone list
 */
#ifndef CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE
#define CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE 2
#endif

#if (CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE < 1 || CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE > 2)
#error "Please ensure CHIP_CONFIG_TIME_ZONE_LIST_MAX_SIZE meets minimum and maximum requirements."
#endif

/**
 * @def CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE
 *
 * Defines the size of the DSTOffset list
 */
#ifndef CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE
#define CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE 2
#endif

#if (CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE < 1)
#error "Please ensure CHIP_CONFIG_DST_OFFSET_LIST_MAX_SIZE meets minimum requirements."
#endif

/**
 * @def CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES
 *
 * @brief Controls whether core data model code will try to include app-specific generated headers.
 *
 * If this is set to true, data model code will be compiled with no client or
 * server clusters enabled and all required access control levels set to their
 * defaults: (view for all attribute/event reads, operate for all writes and
 * invokes).
 */
#ifndef CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES
#define CHIP_CONFIG_SKIP_APP_SPECIFIC_GENERATED_HEADER_INCLUDES 0
#endif

/**
 * @def CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC
 *
 * @brief Default value for the ICD Management cluster IdleModeDuration attribute, in seconds
 */
#ifndef CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC
#define CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC 300
#endif

/**
 * @def CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS
 *
 * @brief Default value for the ICD Management cluster ActiveModeDuration attribute, in milliseconds
 */
#ifndef CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS
#define CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS 300
#endif

/**
 * @def CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS
 *
 * @brief Default value for the ICD Management cluster ActiveModeThreshold attribute, in milliseconds
 */
#ifndef CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS
#define CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS 5000
#endif

/**
 * @def CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC
 *
 * @brief Default value for the ICD Management cluster ClientsSupportedPerFabric attribute
 */
#ifndef CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC
#define CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC 2
#endif

/**
 * @def CHIP_CONFIG_CRYPTO_PSA_ICD_MAX_CLIENTS
 *
 * @brief
 *   Maximum number of ICD clients. Based on this number, platforms that utilize the
 *   PSA Crypto API should reserve key slot range.
 *
 * @note
 *   For platforms that utilize the PSA Crypto API, this configuration is used to
 *   compute the number of PSA key slots. It should remain unchanged during the device's lifetime,
 *   as alterations may lead to issues with backwards compatibility.
 */
#ifndef CHIP_CONFIG_CRYPTO_PSA_ICD_MAX_CLIENTS
#define CHIP_CONFIG_CRYPTO_PSA_ICD_MAX_CLIENTS 256
#endif

/**
 * @def CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF
 *
 * @brief Default value for the ICD Management cluster MaximumCheckInBackoff attribute, in seconds
 */
#ifndef CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF_SEC
#define CHIP_CONFIG_ICD_MAXIMUM_CHECK_IN_BACKOFF_SEC CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC
#endif

/**
 *  @name Configuation for resuming subscriptions that timed out
 *
 *  @brief
 *    The following definitions sets the parameters for subscription resumption in the case of subscription timeout.
 *      * #CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX
 *      * #CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS
 *      * #CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS
 *      * #CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS
 *
 *  @{
 */

/**
 *  @def CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX
 *
 *  @brief
 *    If subscription timeout resumption is enabled, specify the max fibonacci step index.
 *
 *    This index must satisfy below conditions (for readability "CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_" prefix is omitted):
 *      * MIN_RETRY_INTERVAL_SECS + Fibonacci(MAX_FIBONACCI_STEP_INDEX + 1) * WAIT_TIME_MULTIPLIER_SECS > MAX_RETRY_INTERVAL_SECS
 *      * MIN_RETRY_INTERVAL_SECS + Fibonacci(MAX_FIBONACCI_STEP_INDEX) * WAIT_TIME_MULTIPLIER_SECS < MAX_RETRY_INTERVAL_SECS
 *
 */
#ifndef CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX
#define CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX 10
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX

/**
 *  @def CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS
 *
 *  @brief The minimum interval before resuming a subsciption that timed out.
 */
#ifndef CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS
#define CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS 300
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS
 *
 *  @brief The multiplier per step in the calculation of retry interval.
 */
#ifndef CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS
#define CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS 300
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS

/**
 *  @def CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS
 *
 *  @brief The maximum interval before resuming a subsciption that timed out.
 */
#ifndef CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS
#define CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS (3600 * 6)
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS

/**
 * @def CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
 *
 * @brief Controls whether the synchronized report scheduler is used.
 *
 * The use of the synchronous reports feature aims to reduce the number of times an ICD needs to wake up to emit reports to its
 * various subscribers.
 */
#ifndef CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED
#define CHIP_CONFIG_SYNCHRONOUS_REPORTS_ENABLED 0
#endif

/**
 * @def CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS
 *
 * @brief Defines the number of simultaneous ICD Clients info iterators that can be allocated
 *
 * Number of iterator instances that can be allocated at any one time
 */
#ifndef CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS
#define CHIP_CONFIG_MAX_ICD_CLIENTS_INFO_STORAGE_CONCURRENT_ITERATORS 1
#endif

/**
 * @def CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CAPACITY
 *
 * Defines the number of networks the default ThreadNetworkDirectoryStorage implementation will store.
 */
#ifndef CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CAPACITY
#define CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CAPACITY (CHIP_CONFIG_MAX_FABRICS * 2)
#endif

/**
 * @def CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CONCURRENT_ITERATORS
 *
 * Defines the number of ThreadNetworkDirectoryStorage iterators that can be allocated at any one time.
 */
#ifndef CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CONCURRENT_ITERATORS
#define CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CONCURRENT_ITERATORS 1
#endif

/**
 * @def CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
 *
 * @brief CommandSender will use built-in support to enable sending batch commands in a single Invoke Request Message.
 *
 * **Important:** This feature is code and RAM intensive. Enable only on platforms where these
 * resources are not constrained.
 */
#ifndef CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
#define CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS 0
#endif

/**
 * @def CHIP_CONFIG_MAX_PATHS_PER_INVOKE
 *
 * @brief The maximum number of elements in the InvokeRequests list that the Node is able to process.
 */
#ifndef CHIP_CONFIG_MAX_PATHS_PER_INVOKE
#define CHIP_CONFIG_MAX_PATHS_PER_INVOKE 1
#endif

#if CHIP_CONFIG_MAX_PATHS_PER_INVOKE < 1 || CHIP_CONFIG_MAX_PATHS_PER_INVOKE > 65535
#error "CHIP_CONFIG_MAX_PATHS_PER_INVOKE is not allowed to be a number less than 1 or greater than 65535"
#endif

/**
 * @def CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE
 *
 * @brief Defines the entry iterator delegate pool size of the ICDObserver object pool in ICDManager.h.
 *        Two are used in the default implementation. Users can increase it to register more observers.
 */
#ifndef CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE
#define CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE 2
#endif

/**
 * @def CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
 *
 * @brief Enables support for diagnostic logs transfer using the BDX protocol
 *
 */
#ifndef CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
#define CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER 0
#endif

/**
 *  @def CHIP_CONFIG_MAX_BDX_LOG_TRANSFERS
 *
 *  @brief
 *    Maximum number of simultaneously active bdx log transfers.
 *
 */
#ifndef CHIP_CONFIG_MAX_BDX_LOG_TRANSFERS
#define CHIP_CONFIG_MAX_BDX_LOG_TRANSFERS 5
#endif // CHIP_CONFIG_MAX_BDX_LOG_TRANSFERS

/**
 *  @def CHIP_CONFIG_TEST_GOOGLETEST
 *
 *  @brief
 *    If asserted (1), enable APIs that support unit tests built with the GoogleTest framework
 *
 */
#ifndef CHIP_CONFIG_TEST_GOOGLETEST
#define CHIP_CONFIG_TEST_GOOGLETEST 0
#endif // CHIP_CONFIG_TEST_GOOGLETEST

/**
 *  @def CHIP_CONFIG_MRP_ANALYTICS_ENABLED
 *
 *  @brief
 *    Enables code for collecting and sending analytic related events for MRP
 *
 * The purpose of this macro is to prevent compiling code related to MRP analytics
 * for devices that are not interested interested to save on flash.
 */

#ifndef CHIP_CONFIG_MRP_ANALYTICS_ENABLED
#define CHIP_CONFIG_MRP_ANALYTICS_ENABLED 0
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

/**
 * @}
 */
