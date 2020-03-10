/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file defines default compile-time configurations
 *      for the Nest Weave Tunneling Feature.
 *
 */

#ifndef WEAVE_TUNNEL_CONFIG_H_
#define WEAVE_TUNNEL_CONFIG_H_

// clang-format off

/**
 *  @def WEAVE_CONFIG_ENABLE_TUNNELING
 *
 *  @brief
 *    (Default) Set equivalent to INET_CONFIG_ENABLE_TUN_ENDPOINT.
 *
 */
#ifndef WEAVE_CONFIG_ENABLE_TUNNELING
#define WEAVE_CONFIG_ENABLE_TUNNELING                              (INET_CONFIG_ENABLE_TUN_ENDPOINT)
#endif // WEAVE_CONFIG_ENABLE_TUNNELING

/**
 *
 * @brief
 * Check to throw an error if WEAVE_CONFIG_ENABLE_TUNNELING is explicitly set without
 * INET_CONFIG_ENABLE_TUN_ENDPOINT being set.
 *
 */
#if (WEAVE_CONFIG_ENABLE_TUNNELING && !(INET_CONFIG_ENABLE_TUN_ENDPOINT))
#error Weave Tunneling requires Tunnel EndPoint in InetLayer to be enabled
#endif // WEAVE_CONFIG_ENABLE_TUNNELING

/**
 *  @def WEAVE_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default time period(in seconds) at which
 *    the border gateway and the mobile device sends advertisements
 *    for any local tunneling peer to pick up and populate its
 *    nexthop cache in order to locally tunnel over UDP between them.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS
#define WEAVE_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS   (5)
#endif // WEAVE_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS

/**
 *  @def WEAVE_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED
 *
 *  @brief
 *    This defines the default queue depth for queueing data packets
 *    destined for the Service when the connection to the Service
 *    is not yet established.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED
#define WEAVE_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED              (8)
#endif // WEAVE_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED

/**
 *  @def WEAVE_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS
 *
 *  @brief
 *    This defines the default value for the maximum number of shortcut
 *    tunneling peers for which to keep an entry in the nexthop table.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS
#define WEAVE_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS       (8)
#endif // WEAVE_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS

/**
 *  @def WEAVE_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) the underlying platform-specific
 *    functions implemented within the Weave Addressing and Routing Module,
 *    a.k.a. WARM, would override the corresponding supplied default
 *    implementation.
 *
 */
#ifndef WEAVE_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS
#define WEAVE_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS      (0)
#endif /* WEAVE_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS */

/**
 *  @def WEAVE_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY
 *
 *  @brief
 *    This defines the default value for the maximum number of
 *    connection attempts before a notification is made about the
 *    connection failure.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY
#define WEAVE_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY      (3)
#endif // WEAVE_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY

/**
 *  @def WEAVE_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value of the response timeout
 *    (in seconds) for control messages sent over the
 *    tunnel.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS
#define WEAVE_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS         (5)
#endif // WEAVE_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS

/**
 *  @def WEAVE_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval (in seconds) between
 *    keepalive probes for the primary tunnel's TCP connection.
 *    This value also controls the time between last data
 *    packet sent and the transmission of the first keepalive
 *    probe.
 *
 */
#ifndef WEAVE_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS
#define WEAVE_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS       (25)
#endif // WEAVE_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS

/**
 *  @def WEAVE_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES
 *
 *  @brief
 *    This defines the default value for the maximum number of
 *    keepalive probes for both the primary and backup tunnel's
 *    TCP connection.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES
#define WEAVE_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES                  (2)
#endif // WEAVE_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES

/**
 *  @def WEAVE_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value for the maximum timeout
 *    of unacknowledged data for the primary tunnel's TCP
 *    connection.
 *
 */
#ifndef WEAVE_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS
#define WEAVE_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS             (20)
#endif // WEAVE_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS

/**
 *  @def WEAVE_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval (in seconds) between
 *    keepalive probes for the backup tunnel's TCP connection.
 *    This value also controls the time between last data
 *    packet sent and the transmission of the first keepalive
 *    probe.
 *
 */
#ifndef WEAVE_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS
#define WEAVE_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS       (50)
#endif // WEAVE_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS

/**
 *  @def WEAVE_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value for the maximum timeout
 *    of unacknowledged data for the backup tunnel's TCP
 *    connection.
 *
 */
#ifndef WEAVE_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS
#define WEAVE_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS              (10)
#endif // WEAVE_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS

/**
 *  @def WEAVE_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval that the primary
 *    tunnel would wait since the last activity over the tunnel
 *    to send a Tunnel Liveness probe to the other end.
 *
 *  @note
 *    A smaller value for this interval would mean a faster
 *    detection of a broken connection while, simultaneously,
 *    increasing the amount of traffic over the connection.
 *    This trade-off needs to be carefully considered when
 *    setting this value especially for devices that are
 *    operating in a power constrained mode and are sensitive
 *    to traffic induced wake-ups.
 *
 */
#ifndef WEAVE_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS
#define WEAVE_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS       (195)
#endif // WEAVE_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS

/**
 *  @def WEAVE_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval that the backup
 *    tunnel would wait since the last activity over the tunnel
 *    to send a Tunnel Liveness probe to the other end.
 *
 *  @note
 *    A smaller value for this interval would mean a faster
 *    detection of a broken connection while, simultaneously,
 *    increasing the amount of traffic over the connection.
 *    This trade-off needs to be carefully considered when
 *    setting this value especially for devices that are
 *    operating in a power constrained mode and are sensitive
 *    to traffic induced wake-ups.
 *
 */
#ifndef WEAVE_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS
#define WEAVE_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS        (195)
#endif // WEAVE_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS


/**
 *  @def WEAVE_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default timeout for the tunnel connect
 *    attempt to either succeed or notify the caller of an
 *    error.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS
#define WEAVE_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS                 (10)
#endif // WEAVE_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS

/**
 *  @def WEAVE_CONFIG_TUNNEL_FAILOVER_SUPPORTED
 *
 *  @brief
 *    This defines whether support for tunnel redundancy
 *    and failover is present.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_FAILOVER_SUPPORTED
#define WEAVE_CONFIG_TUNNEL_FAILOVER_SUPPORTED                   (0)
#endif // WEAVE_CONFIG_TUNNEL_FAILOVER_SUPPORTED

/**
 *  @def WEAVE_CONFIG_TUNNEL_LIVENESS_SUPPORTED
 *
 *  @brief
 *    This defines whether support for the periodic tunnel
 *    liveness is present.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_LIVENESS_SUPPORTED
#define WEAVE_CONFIG_TUNNEL_LIVENESS_SUPPORTED                   (1)
#endif // WEAVE_CONFIG_TUNNEL_LIVENESS_SUPPORTED

/**
 *  @def WEAVE_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    Keepalive socket option is present for the Weave
 *    Tunnel.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED
#define WEAVE_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED              (!WEAVE_CONFIG_TUNNEL_LIVENESS_SUPPORTED)
#endif // WEAVE_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED

/**
 *  @def WEAVE_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    UserTimeout socket option is present for the Weave
 *    Tunnel.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED
#define WEAVE_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED           (1)
#endif // WEAVE_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED

/**
 *  @def WEAVE_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    send channel being idle is present for the Weave
 *    Tunnel.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK
#define WEAVE_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK             (INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS)
#endif // WEAVE_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK

/**
 *  @def WEAVE_CONFIG_TUNNEL_SHORTCUT_SUPPORTED
 *
 *  @brief
 *    This defines whether support for tunnel shortcut
 *    is present.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_SHORTCUT_SUPPORTED
#define WEAVE_CONFIG_TUNNEL_SHORTCUT_SUPPORTED                   (0)
#endif // WEAVE_CONFIG_TUNNEL_SHORTCUT_SUPPORTED

/**
 *  @def WEAVE_CONFIG_TUNNEL_ENABLE_STATISTICS
 *
 *  @brief
 *    This defines whether support for tunnel statistics
 *    collection is present.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_ENABLE_STATISTICS
#define WEAVE_CONFIG_TUNNEL_ENABLE_STATISTICS                    (1)
#endif // WEAVE_CONFIG_TUNNEL_ENABLE_STATISTICS

/**
 *  @def WEAVE_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK
 *
 *  @brief
 *    This defines whether the transiting packet would be
 *    passed to the application, for example, for logging purposes.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK
#define WEAVE_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK              (1)
#endif // WEAVE_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK

/**
 *  @def WEAVE_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX
 *
 *  @brief
 *    This defines the maximum fibonacci index value for the
 *    back-off algortihm to generate the next reconnect time.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX
#define WEAVE_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX     (14)
#endif // WEAVE_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX

/**
 *  @def WEAVE_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS
 *
 *  @brief
 *    This specifies the multiplying factor to the result of
 *    a fibonacci computation based on a specific index to provide
 *    a max wait time for the next connection attempt.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS
#define WEAVE_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS (10)
#endif // WEAVE_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS

/**
 *  @def WEAVE_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT
 *
 *  @brief
 *    The minimum wait time as a percentage of the max wait interval
 *    for that step.
 *
 *  @note
 *    The minimum delay specified in the reconnect param configuration
 *    will get precedence if it is greater than the delay calculated
 *    using this percentage calculation.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT
#define WEAVE_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT    (30)
#endif // WEAVE_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT

/**
 *  @def WEAVE_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS
 *
 *  @brief
 *    The maximum time to wait when the current reconnect timeout
 *    is reset.
 *
 *  @note
 *    The actual wait period is chosen as a random duration
 *    less than this value. The user can also request to not
 *    wait and reconnect immediately.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS
#define WEAVE_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS      (10)
#endif // WEAVE_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS

/**
 *  @def WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS
 *
 *  @brief
 *    The frequency of performing a network online check when
 *    the corresponding tunnel is down.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS
#define WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS       (30)
#endif // WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS

/**
 *  @def WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS
 *
 *  @brief
 *    The slower frequency of performing a network online check
 *    after detecting that the network has come online and before
 *    corresponding tunnel is established.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS
#define WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS  (60)
#endif // WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS

/**
 *  @def WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS
 *
 *  @brief
 *    The slower frequency of performing a network online check
 *    after detecting that the network has come online and before
 *    backup tunnel is established.
 *
 */
#ifndef WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS
#define WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS   (90)
#endif // WEAVE_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS

/**
 *  @def WEAVE_CONFIG_TUNNEL_INTERFACE_MTU
 *
 *  @brief
 *    The maximum transmission unit for the Tunnel interface
 *    on a border gateway.
 *
 *  @note
 *    Setting a non-default value may also require updating
 *    WEAVE_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX for socket
 *    platforms and PBUF size for LwIP platforms.
 *
 */
#ifndef WEAVE_CONFIG_TUNNEL_INTERFACE_MTU
#define WEAVE_CONFIG_TUNNEL_INTERFACE_MTU                           (1536)
#endif // WEAVE_CONFIG_TUNNEL_INTERFACE_MTU

// clang-format on

#endif /* WEAVE_TUNNEL_CONFIG_H_ */
