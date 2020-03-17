/*
 *
 *    <COPYRIGHT>
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
 *      for the CHIP InetLayer, an Internet Protocol communications
 *      abstraction layer.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific InetProjectConfig.h header and then assert
 *      HAVE_INETPROJECTCONFIG_H via the package configuration tool
 *      via --with-weave-inet-project-includes=DIR where DIR is the
 *      directory that contains the header.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#ifndef INETCONFIG_H
#define INETCONFIG_H

/*--- Include configuration headers ---*/
#include <SystemConfig.h>

/*
 * If the CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION option is not applicable, then the "InetProjectConfig.h"
 * header was not included by <SystemLayer/SystemConfig.h> and therefore it must be included here.
 */
#if !CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

/* Include a project-specific configuration file, if defined.
 *
 * An application or module that incorporates chip can define a project configuration
 * file to override standard Inet Layer configuration with application-specific values.
 * The project config file is typically located outside the Openchip source tree,
 * alongside the source code for the application.
 */
#ifdef INET_PROJECT_CONFIG_INCLUDE
#include INET_PROJECT_CONFIG_INCLUDE
#endif // INET_PROJECT_CONFIG_INCLUDE

#endif // !CHIP_SYSTEM_CONFIG_TRANSFER_INETLAYER_PROJECT_CONFIGURATION

/* Include a platform-specific configuration file, if defined.
 *
 * A platform configuration file contains overrides to standard Inet Layer configuration
 * that are specific to the platform or OS on which chip is running.  It is typically
 * provided as apart of an adaptation layer that adapts Openchip to the target
 * environment.  This adaptation layer may be included in the Openchip source tree
 * itself or implemented externally.
 */
#ifdef INET_PLATFORM_CONFIG_INCLUDE
#include INET_PLATFORM_CONFIG_INCLUDE
#endif

// clang-format off

/**
 *  @def INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
 *
 *  @brief
 *    This boolean configuration option is (1) if the obsolescent interfaces
 *    of the INET layer that now reside elsewhere, for example, in the chip System
 *    Layer are aliased for transitional purposes.
 *
 */
#ifndef INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#define INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES          0
#endif //  INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES && !CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
#error "REQUIRED: if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES then CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES!"
#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES && !CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  @def INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE
 *
 *  @brief
 *    The maximum space required for IPv6 and UDP headers.
 *    Useful when ensuring a chip message will not exceed a UDP MTU.
 *
 */
#ifndef INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE
#define INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE              (40 + 8)
#endif // INET_CONFIG_MAX_IP_AND_UDP_HEADER_SIZE

/**
 *  @def INET_CONFIG_ERROR_TYPE
 *
 *  @brief
 *    This defines the data type used to represent errors for the
 *    InetLayer subsystem.
 *
 *  @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_ERROR_TYPE.
 *
 */
#ifndef INET_CONFIG_ERROR_TYPE
#define INET_CONFIG_ERROR_TYPE                              CHIP_SYSTEM_CONFIG_ERROR_TYPE
#endif // !defined(INET_CONFIG_ERROR_TYPE)

/**
 *  @def INET_CONFIG_NO_ERROR
 *
 *  @brief
 *    This defines the InetLayer error code for no error or success.
 *
 *  @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_NO_ERROR.
 *
 */
#ifndef INET_CONFIG_NO_ERROR
#define INET_CONFIG_NO_ERROR                                CHIP_SYSTEM_CONFIG_NO_ERROR
#endif // !defined(INET_CONFIG_NO_ERROR)

/**
 *  @def INET_CONFIG_ERROR_MIN
 *
 *  @brief
 *    This defines the base or minimum InetLayer error number range.
 *
 */
#ifndef INET_CONFIG_ERROR_MIN
#define INET_CONFIG_ERROR_MIN                               1000
#endif // INET_CONFIG_ERROR_MIN

/**
 *  @def INET_CONFIG_ERROR_MAX
 *
 *  @brief
 *    This defines the top or maximum InetLayer error number range.
 *
 */
#ifndef INET_CONFIG_ERROR_MAX
#define INET_CONFIG_ERROR_MAX                               1999
#endif // INET_CONFIG_ERROR_MAX

/**
 *  @def _INET_CONFIG_ERROR
 *
 *  @brief
 *    This defines a mapping function for InetLayer errors that allows
 *    mapping such errors into a platform- or system-specific range.
 *
 */
#ifndef _INET_CONFIG_ERROR
#define _INET_CONFIG_ERROR(e)                               (INET_ERROR_MIN + (e))
#endif // _INET_CONFIG_ERROR

/**
 *  @def INET_CONFIG_WILL_OVERRIDE_OS_ERROR_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) your platform will override
 *    the platform- and system-specific INET_MapOSError,
 *    INET_DescribeOSError, and INET_IsOSError functions.
 *
 */
#ifndef INET_CONFIG_WILL_OVERRIDE_OS_ERROR_FUNCS
#define INET_CONFIG_WILL_OVERRIDE_OS_ERROR_FUNCS            0
#endif // INET_CONFIG_WILL_OVERRIDE_OS_ERROR_FUNCS

/**
 *  @def INET_CONFIG_WILL_OVERRIDE_LWIP_ERROR_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) your platform will override
 *    the platform- and system-specific INET_MapLwIPError,
 *    INET_DescribeLwIPError, and INET_IsLwIPError functions.
 *
 */
#ifndef INET_CONFIG_WILL_OVERRIDE_LWIP_ERROR_FUNCS
#define INET_CONFIG_WILL_OVERRIDE_LWIP_ERROR_FUNCS          0
#endif // INET_CONFIG_WILL_OVERRIDE_LWIP_ERROR_FUNCS

/**
 *  @def INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) your platform will override
 *    the platform- and system-specific InetLayer WillInit, DidInit,
 *    WillShutdown, and DidShutdown.
 *
 */
#ifndef INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS
#define INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS       0
#endif // INET_CONFIG_WILL_OVERRIDE_PLATFORM_XTOR_FUNCS

/**
 *  @def INET_CONFIG_MAX_DROPPABLE_EVENTS
 *
 *  @brief
 *    This is the maximum number of UDP or raw network transport
 *    packet events / messages that may be dropped due to packet
 *    buffer starvation.
 *
 *    In some implementations, there may be a shared event / message
 *    queue for the InetLayer used by other system events / messages.
 *
 *    If the length of that queue is considerably longer than the
 *    number of packet buffers available, it may lead to buffer
 *    exhaustion. As a result, using the queue itself to implement
 *    backpressure is insufficient, and we need an external mechanism
 *    to prevent buffer starvation in the rest of the system and
 *    getting into deadlock situations.
 *
 *    For both UDP and raw network transport traffic we can easily
 *    drop incoming packets without impacting the correctness of
 *    higher level protocols.
 *
 */
#ifndef INET_CONFIG_MAX_DROPPABLE_EVENTS
#define INET_CONFIG_MAX_DROPPABLE_EVENTS                    0
#endif // INET_CONFIG_MAX_DROPPABLE_EVENTS

/**
 *  @def INET_CONFIG_NUM_RAW_ENDPOINTS
 *
 *  @brief
 *    This is the total number of "raw" (direct-IP, non-TCP/-UDP) end
 *    point context structures.
 *
 *    Up to this many outstanding "raw" communication flows may be in
 *    use.
 *
 */
#ifndef INET_CONFIG_NUM_RAW_ENDPOINTS
#define INET_CONFIG_NUM_RAW_ENDPOINTS                       8
#endif // INET_CONFIG_NUM_RAW_ENDPOINTS

/**
 *  @def INET_CONFIG_NUM_TCP_ENDPOINTS
 *
 *  @brief
 *    This is the total number of TCP end point context structures.
 *
 *    Up to this many outstanding TCP communication flows may be in
 *    use.
 *
 */
#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS                       64
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

/**
 *  @def INET_CONFIG_NUM_UDP_ENDPOINTS
 *
 *  @brief
 *    This is the total number of UDP end point context structures.
 *
 *    Up to this many outstanding UDP communication flows may be in
 *    use.
 *
 */
#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS                       64
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS

/**
 *  @def INET_CONFIG_NUM_TUN_ENDPOINTS
 *
 *  @brief
 *    This is the total number of TUN end point context structures.
 *
 *    Up to this many outstanding TUN endpoints may be instantiated.
 *
 */
#ifndef INET_CONFIG_NUM_TUN_ENDPOINTS
#define INET_CONFIG_NUM_TUN_ENDPOINTS                       64
#endif // INET_CONFIG_NUM_TUN_ENDPOINTS

/**
 *  @def INET_CONFIG_NUM_DNS_RESOLVERS
 *
 *  @brief
 *    This is the total number of outstanding DNS resolution request
 *    contexts.
 *
 *    Up to this many DNS resolution requests may be in in use.
 *
 */
#ifndef INET_CONFIG_NUM_DNS_RESOLVERS
#define INET_CONFIG_NUM_DNS_RESOLVERS                       4
#endif // INET_CONFIG_NUM_DNS_RESOLVERS

/**
 *  @def INET_CONFIG_MAX_DNS_ADDRS
 *
 *  @brief
 *    This is the maximum allowable number of addresses that may
 *    be returned in a single DNS hostname lookup.
 *
 */
#ifndef INET_CONFIG_MAX_DNS_ADDRS
#define INET_CONFIG_MAX_DNS_ADDRS                           16
#endif // INET_CONFIG_MAX_DNS_ADDRS

/**
 *  @def INET_TCP_IDLE_CHECK_INTERVAL
 *
 *  @brief
 *    This is the interval, in milliseconds, at which checks are made
 *    to detect idle TCP connections.
 *
 */
#ifndef INET_TCP_IDLE_CHECK_INTERVAL
#define INET_TCP_IDLE_CHECK_INTERVAL                        100
#endif // INET_TCP_IDLE_CHECK_INTERVAL

/**
 *  @def INET_CONFIG_ENABLE_DNS_RESOLVER
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable the ability
 *    to instantiate the DNS resolver.
 *
 */
#ifndef INET_CONFIG_ENABLE_DNS_RESOLVER
#define INET_CONFIG_ENABLE_DNS_RESOLVER                     0
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

/**
 *  @def INET_CONFIG_ENABLE_RAW_ENDPOINT
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable the ability
 *    to instantiate a Raw endpoint.
 *
 */
#ifndef INET_CONFIG_ENABLE_RAW_ENDPOINT
#define INET_CONFIG_ENABLE_RAW_ENDPOINT                     0
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

/**
 *  @def INET_CONFIG_ENABLE_TCP_ENDPOINT
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable the ability
 *    to instantiate a TCP endpoint.
 *
 */
#ifndef INET_CONFIG_ENABLE_TCP_ENDPOINT
#define INET_CONFIG_ENABLE_TCP_ENDPOINT                     0
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

/**
 *  @def INET_CONFIG_ENABLE_TUN_ENDPOINT
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable the ability
 *    to instantiate a Tunnel endpoint for tun interfaces.
 *    By default, set when LWIP is enabled or being built
 *    on a Linux platform.
 *
 */
#ifndef INET_CONFIG_ENABLE_TUN_ENDPOINT
#define INET_CONFIG_ENABLE_TUN_ENDPOINT                     0
#endif // INET_CONFIG_ENABLE_TUN_ENDPOINT


/**
 *  @def INET_CONFIG_ENABLE_UDP_ENDPOINT
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable the ability
 *    to instantiate a UDP endpoint.
 *
 */
#ifndef INET_CONFIG_ENABLE_UDP_ENDPOINT
#define INET_CONFIG_ENABLE_UDP_ENDPOINT                     0
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

/**
 *  @def INET_CONFIG_EVENT_RESERVED
 *
 *  @brief
 *      This defines the first number in the default chip System Layer event code space reserved for use by the Inet Layer.
 *      Event codes used by each layer must not overlap.
 */
#ifndef INET_CONFIG_EVENT_RESERVED
#define INET_CONFIG_EVENT_RESERVED                          1000
#endif /* INET_CONFIG_EVENT_RESERVED */

/**
 *  @def _INET_CONFIG_EVENT
 *
 *  @brief
 *    This defines a mapping function for InetLayer event types that allows
 *    mapping such event types into a platform- or system-specific range.
 *
 *  @note
 *    By default, this definition is a copy of _CHIP_SYSTEM_CONFIG_LWIP_EVENT.
 *
 */
#ifndef _INET_CONFIG_EVENT
#define _INET_CONFIG_EVENT(e)                               _CHIP_SYSTEM_CONFIG_LWIP_EVENT(INET_CONFIG_EVENT_RESERVED + (e))
#endif // _INET_CONFIG_EVENT

/*
 * NOTE WELL: the following configuration parameters and macro definitions are
 * obsolescent. They are provided here to facilitate transition.
 */
#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  @def INET_LWIP
 *
 *  @brief
 *      Use LwIP.
 *
 *  @note
 *      By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_USE_LWIP.
 */
#ifndef INET_LWIP
#define INET_LWIP                                           CHIP_SYSTEM_CONFIG_USE_LWIP
#endif // !defined(INET_LWIP)

/**
 *  @def INET_SOCKETS
 *
 *  @brief
 *      Use BSD sockets.
 *
 *  @note
 *      By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_USE_SOCKETS.
 */
#ifndef INET_SOCKETS
#define INET_SOCKETS                                        CHIP_SYSTEM_CONFIG_USE_SOCKETS
#endif // !defined(INET_SOCKETS)

/**
 *  @def INET_CONFIG_POSIX_LOCKING
 *
 *  @brief
 *    Use POSIX locking. This is enabled by default when not compiling
 *    for BSD sockets.
 *
 *    Unless you are simulating an LwIP-based system on a Unix-style
 *    host, this value should be left at its default.
 *
 *  @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_POSIX_LOCKING.
 *
 */
#ifndef INET_CONFIG_POSIX_LOCKING
#define INET_CONFIG_POSIX_LOCKING                           CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#endif // !defined(INET_CONFIG_POSIX_LOCKING)

/**
 *  @def INET_CONFIG_FREERTOS_LOCKING
 *
 *  @brief
 *    Use FreeRTOS locking.
 *
 *    This should be generally asserted (1) for FreeRTOS + LwIP-based
 *    systems and deasserted (0) for BSD sockets-based systems.
 *
 *    However, if you are simulating an LwIP-based system atop POSIX
 *    threads and BSD sockets, this should also be deasserted (0).
 *
 *  @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING.
 *
 */
#ifndef INET_CONFIG_FREERTOS_LOCKING
#define INET_CONFIG_FREERTOS_LOCKING                        CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#endif // !defined(INET_CONFIG_FREERTOS_LOCKING)

/**
 *  @def INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) your platform will override
 *    the platform- and system-specific PostEvent, DispatchEvents, and
 *    DispatchEvent functions.
 *
 */
#ifndef INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS
#define INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS      0
#endif // INET_CONFIG_WILL_OVERRIDE_PLATFORM_EVENT_FUNCS

/**
 *  @def INET_CONFIG_EVENT_TYPE
 *
 *  @brief
 *     This defines the type for InetEvent types, typically
 *     an integral type.
 *
 *  @note
 *     By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE.
 *
 */
#ifndef INET_CONFIG_EVENT_TYPE
#define INET_CONFIG_EVENT_TYPE                              CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE
#endif // INET_CONFIG_EVENT_TYPE

/**
 *  @def INET_CONFIG_EVENT_OBJECT_TYPE
 *
 *  @brief
 *     This defines the type of InetEvent objects or "messages".
 *
 *     Such types are not directly used by the InetLayer but are
 *     "passed through". Consequently a forward declaration and a
 *     const pointer or reference are appropriate.
 *
 *  @note
 *     By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE.
 *
 */
#ifndef INET_CONFIG_EVENT_OBJECT_TYPE
#define INET_CONFIG_EVENT_OBJECT_TYPE                       CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE
#endif // INET_CONFIG_EVENT_OBJECT_TYPE

/**
 *  @def INET_CONFIG_NUM_BUFS
 *
 *  @brief
 *    This is the total number of packet buffers for the BSD sockets
 *    configuration.
 *
 *    This may be set to zero (0) to enable unbounded dynamic
 *    allocation using malloc.
 *
 * @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC.
 *
 */
#ifndef INET_CONFIG_NUM_BUFS
#define INET_CONFIG_NUM_BUFS                                CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC
#endif // INET_CONFIG_NUM_BUFS

/**
 *  @def INET_CONFIG_NUM_TIMERS
 *
 *  @brief
 *    This is the total number of available timers.
 *
 *  @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_NUM_TIMERS.
 *
 */
#ifndef INET_CONFIG_NUM_TIMERS
#define INET_CONFIG_NUM_TIMERS                              CHIP_SYSTEM_CONFIG_NUM_TIMERS
#endif // INET_CONFIG_NUM_TIMERS

/**
 * @def INET_CONFIG_HEADER_RESERVE_SIZE
 *
 * @brief
 *  The number of bytes to reserve in a network packet buffer to contain
 *  all the possible protocol encapsulation headers before the application
 *  message text.
 *
 * @note
 *    By default, this parameter is a copy of CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE.
 *
 */
#ifndef INET_CONFIG_HEADER_RESERVE_SIZE
#define INET_CONFIG_HEADER_RESERVE_SIZE                     CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
#endif // INET_CONFIG_HEADER_RESERVE_SIZE

#endif // INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

/**
 *  @def INET_CONFIG_TEST
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable testing aids
 */
#ifndef INET_CONFIG_TEST
#define INET_CONFIG_TEST                                   0
#endif

/**
 *  @def INET_CONFIG_TUNNEL_DEVICE_NAME
 *
 *  @brief
 *    Defines tunnel device name
 */
#ifndef INET_CONFIG_TUNNEL_DEVICE_NAME
#define INET_CONFIG_TUNNEL_DEVICE_NAME                      "/dev/net/tun"
#endif //INET_CONFIG_TUNNEL_DEVICE_NAME

/**
 * @def INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
 *
 * @brief Enable asynchronous dns name resolution for Linux sockets.
 */
#ifndef INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#define INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS               1
#endif // INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

/**
 * @def INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT
 *
 * @brief The maximum number of POSIX threads that would be performing
 * asynchronous DNS resolution.
 */
#ifndef INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT
#define INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT             2
#endif // INET_CONFIG_DNS_ASYNC_MAX_THREAD_COUNT

/**
 *  @def INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
 *
 *  @brief
 *    When this flag is set, the InetLayer implements
 *    a mechanism that simulates the TCP_USER_TIMEOUT
 *    socket option for Linux sockets.
 *
 *  @details
 *    The TCP_USER_TIMEOUT option specifies the maximum
 *    amount of time in milliseconds that transmitted
 *    data may remain unacknowledged before TCP will
 *    forcibly close the corresponding connection.
 *    See RFC 5482, for further details.
 *
 *    This flag is set to override the usage of the system
 *    defined TCP_USER_TIMEOUT socket option for Linux.
 *    The reason for having this overriding feature
 *    was to overcome some limitations of the system
 *    socket option wherein the TCP_USER_TIMEOUT was
 *    not taking effect when
 *    1) The IP address of the interface of the TCP
 *       connection was removed.
 *    2) The interface of the TCP connection was
 *       brought down.
 *
 */
#ifndef INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT       1
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

/**
 *  @def INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
 *
 *  @brief
 *    When this flag is set, the InetLayer enables
 *    callbacks to the upper layer notifying it when
 *    the send channel of the TCP connection changes
 *    between being idle or not idle.
 *
 *  @note
 *    When enabled, the TCP send queue is actively
 *    polled to determine if sent data has been
 *    acknowledged.
 *
 */
#ifndef INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
#define INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS         0
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

/**
 *  @def INET_CONFIG_TCP_SEND_QUEUE_POLL_INTERVAL_MSEC
 *
 *  @brief
 *    The default polling interval to check the progress
 *    on the TCP SendQueue to determine if sent data is
 *    being acknowledged.
 *
 *  @details
 *    If progress is being made, then the TCP UserTimeout
 *    period would be shifted further ahead by resetting
 *    the max poll count. If, however, progress is not made,
 *    then the next timer would still fire at the next poll
 *    interval without resetting the poll count. The
 *    connection would be torn down when the poll count
 *    reaches zero.
 */
#ifndef INET_CONFIG_TCP_SEND_QUEUE_POLL_INTERVAL_MSEC
#define INET_CONFIG_TCP_SEND_QUEUE_POLL_INTERVAL_MSEC      500
#endif // INET_CONFIG_TCP_SEND_QUEUE_POLL_INTERVAL_MSEC

/**
 *  @def INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC
 *
 *  @brief
 *    The default value of the TCP_USER_TIMEOUT in
 *    milliseconds. Set to equivalent of 5 minutes.
 *
 *
 *  @details
 *    This value specifies the maximum amount of
 *    time in milliseconds that transmitted data may remain
 *    unacknowledged before TCP will forcibly close the
 *    corresponding connection.
 *    See RFC 5482, for further details.
 */
#ifndef INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC
#define INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC          (5 * 60 * 1000)
#endif // INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC

/**
 *  @def INET_CONFIG_IP_MULTICAST_HOP_LIMIT
 *
 *  @brief
 *    The default value of the Hop Limit field in
 *    the IP multicast packet.
 *
 *
 *  @details
 *    This value specifies the hop limit that could be set in the
 *    outgoing IP packet to override any default settings(made by
 *    the kernel) for the hop limit field in the IP header.
 *    On Linux platforms, the hoplimit field is set to 1 for
 *    multicast packets.
 */
#ifndef INET_CONFIG_IP_MULTICAST_HOP_LIMIT
#define INET_CONFIG_IP_MULTICAST_HOP_LIMIT                 (64)
#endif // INET_CONFIG_IP_MULTICAST_HOP_LIMIT
// clang-format on

#endif /* INETCONFIG_H */
