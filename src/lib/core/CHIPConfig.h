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
 *    implementation of chip Security Manager memory-management
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
 *          4 -- Acknowledged Message Id
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
#define CHIP_PORT 5540
#endif // CHIP_PORT

/**
 *  @def CHIP_UDC_PORT
 *
 *  @brief
 *    chip TCP/UDP port for unsecured user-directed-commissioning traffic.
 *
 */
#ifndef CHIP_UDC_PORT
#define CHIP_UDC_PORT CHIP_PORT + 10
#endif // CHIP_UDC_PORT

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

#ifndef CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER
#define CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER "GlobalMCTR"
#endif // CHIP_CONFIG_PERSISTED_STORAGE_KEY_GLOBAL_MESSAGE_COUNTER

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
    ((CODE) == CHIP_ERROR_POSIX(EHOSTUNREACH) || (CODE) == CHIP_ERROR_POSIX(ENETUNREACH) ||                                        \
     (CODE) == CHIP_ERROR_POSIX(EADDRNOTAVAIL) || (CODE) == CHIP_ERROR_POSIX(EPIPE))
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
 * @def CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE
 *
 * @brief Define the size of the pool used for tracking CHIP unauthenticated
 * states. The entries in the pool are automatically rotated by LRU. The size
 * of the pool limits how many PASE and CASE pairing sessions can be processed
 * simultaneously.
 */
#ifndef CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE
#define CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE 4
#endif // CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE

/**
 * @def CHIP_CONFIG_GROUP_CONNECTION_POOL_SIZE
 *
 * @brief Define the size of the pool used for tracking CHIP groups.
 *        Given the ephemeral nature of groups session, no need to support
 *        a large pool size.
 */
#ifndef CHIP_CONFIG_GROUP_CONNECTION_POOL_SIZE
#define CHIP_CONFIG_GROUP_CONNECTION_POOL_SIZE 4
#endif // CHIP_CONFIG_GROUP_CONNECTION_POOL_SIZE

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
 *  @def CHIP_CONFIG_MAX_GROUP_DATA_PEERS
 *
 *  @brief
 *    Maximum number of Peer within a fabric that can send group data message to a device.
 *
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
 * @def CHIP_NON_PRODUCTION_MARKER
 *
 * @brief Defines the name of a mark symbol whose presence signals that the chip code
 * includes development/testing features that should never be used in production contexts.
 */
#ifndef CHIP_NON_PRODUCTION_MARKER
#if (CHIP_CONFIG_SECURITY_TEST_MODE || (!CHIP_CONFIG_REQUIRE_AUTH) || CHIP_FUZZING_ENABLED)
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
 * @def CHIP_CONFIG_MDNS_CACHE_SIZE
 *
 * @brief
 *      Define the size of the MDNS cache
 *
 *      If CHIP_CONFIG_MDNS_CACHE_SIZE is 0, the builtin cache is not used.
 *
 */
#ifndef CHIP_CONFIG_MDNS_CACHE_SIZE
#define CHIP_CONFIG_MDNS_CACHE_SIZE 20
#endif
/**
 *  @name Interaction Model object pool configuration.
 *
 *  @brief
 *    The following definitions sets the maximum number of corresponding interaction model object pool size.
 *
 *      * #CHIP_IM_MAX_NUM_COMMAND_HANDLER
 *      * #CHIP_IM_MAX_NUM_READ_HANDLER
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
 * @def CHIP_IM_MAX_NUM_READ_HANDLER
 *
 * @brief Defines the maximum number of ReadHandler, limits the number of active read transactions on server.
 */
#ifndef CHIP_IM_MAX_NUM_READ_HANDLER
#define CHIP_IM_MAX_NUM_READ_HANDLER 4
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
 * @def CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS
 *
 * @brief Defines the maximum number of path objects, limits the number of attributes being read or subscribed at the same time.
 */
#ifndef CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS
#define CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS 8
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
 * @def CONFIG_IM_BUILD_FOR_UNIT_TEST
 *
 * @brief Defines whether we're currently building the IM for unit testing, which enables a set of features
 *        that are only utilized in those tests.
 */
#ifndef CONFIG_IM_BUILD_FOR_UNIT_TEST
#define CONFIG_IM_BUILD_FOR_UNIT_TEST 0
#endif

/**
 * @def CHIP_CONFIG_LAMBDA_EVENT_SIZE
 *
 * @brief The maximum size of the lambda which can be post into system event queue.
 */
#ifndef CHIP_CONFIG_LAMBDA_EVENT_SIZE
#define CHIP_CONFIG_LAMBDA_EVENT_SIZE (16)
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
 * @brief Number of devices an end device can be simultaneously connected to
 */
#ifndef CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES
#define CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES 4
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
 *
 * @brief Defines the number of groups supported per fabric, see Group Key Management Cluster in specification.
 *
 * Binds to number of GroupState entries to support per fabric
 */
#ifndef CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUPS_PER_FABRIC 2
#endif

/**
 * @def CHIP_CONFIG_MAX_GROUPS_PER_FABRIC
 *
 * @brief Defines the number of groups supported per fabric, see Group Key Management Cluster in specification.
 *
 * Binds to number of GroupState entries to support per fabric
 */
#ifndef CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC
#define CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC 2
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
#define CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC 3
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
 * @def CHIP_CONFIG_MAX_SESSION_RELEASE_DELEGATES
 *
 * @brief Defines the max number of SessionReleaseDelegate
 */
#ifndef CHIP_CONFIG_MAX_SESSION_RELEASE_DELEGATES
#define CHIP_CONFIG_MAX_SESSION_RELEASE_DELEGATES 4
#endif

/**
 * @def CHIP_CONFIG_MAX_SESSION_RECOVERY_DELEGATES
 *
 * @brief Defines the max number of SessionRecoveryDelegate
 */
#ifndef CHIP_CONFIG_MAX_SESSION_RECOVERY_DELEGATES
#define CHIP_CONFIG_MAX_SESSION_RECOVERY_DELEGATES 4
#endif

/**
 * @def CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE
 *
 * @brief
 *   Maximum number of CASE sessions that a device caches, that can be resumed
 */
#ifndef CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE
#define CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE 4
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

/**
 * @}
 */
