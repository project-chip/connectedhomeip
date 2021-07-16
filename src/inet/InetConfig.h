/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      for the CHIP InetLayer, an Internet Protocol communications
 *      abstraction layer.
 *
 *      Package integrators that wish to override these values should
 *      either use preprocessor definitions or create a project-
 *      specific InetProjectConfig.h header and then assert
 *      HAVE_INETPROJECTCONFIG_H via the package configuration tool
 *      via --with-chip-inet-project-includes=DIR where DIR is the
 *      directory that contains the header.
 *
 *  NOTE WELL: On some platforms, this header is included by C-language programs.
 *
 */

#pragma once

#if CHIP_HAVE_CONFIG_H
#include <inet/InetBuildConfig.h>
#endif

/*--- Include configuration headers ---*/
#include <system/SystemConfig.h>

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
