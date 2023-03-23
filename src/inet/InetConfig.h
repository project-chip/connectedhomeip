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
 * header was not included by <system/SystemConfig.h> and therefore it must be included here.
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
 *  @def INET_CONFIG_TEST
 *
 *  @brief
 *    Defines whether (1) or not (0) to enable testing aids
 */
#ifndef INET_CONFIG_TEST
#define INET_CONFIG_TEST                                   0
#endif

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

/**
 *  @def INET_CONFIG_UDP_SOCKET_PKTINFO
 *
 *  @brief
 *    Use IP_PKTINFO and IPV6_PKTINFO control messages to specify the network
 *    interface and the source address of a sent UDP packet.
 *
 *  @details
 *    When this flag is set, the socket-based implementation of UDP endpoints
 *    requires that IP_PKTINFO and IPV6_PKTINFO be supported. Otherwise, it is
 *    left to the operating system to select the network interface and the
 *    source address.
 */
#ifndef INET_CONFIG_UDP_SOCKET_PKTINFO
#ifndef __ZEPHYR__
#define INET_CONFIG_UDP_SOCKET_PKTINFO 1
#else
#define INET_CONFIG_UDP_SOCKET_PKTINFO 0
#endif
#endif // INET_CONFIG_UDP_SOCKET_PKTINFO

// clang-format on
