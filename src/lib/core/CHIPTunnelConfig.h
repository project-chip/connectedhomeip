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
 *      This file defines default compile-time configurations
 *      for the chip Tunneling Feature.
 *
 */

#ifndef CHIP_TUNNEL_CONFIG_H_
#define CHIP_TUNNEL_CONFIG_H_

// clang-format off

/**
 *  @def CHIP_CONFIG_ENABLE_TUNNELING
 *
 *  @brief
 *    (Default) Set equivalent to INET_CONFIG_ENABLE_TUN_ENDPOINT.
 *
 */
#ifndef CHIP_CONFIG_ENABLE_TUNNELING
#define CHIP_CONFIG_ENABLE_TUNNELING                              (INET_CONFIG_ENABLE_TUN_ENDPOINT)
#endif // CHIP_CONFIG_ENABLE_TUNNELING

/**
 *
 * @brief
 * Check to throw an error if CHIP_CONFIG_ENABLE_TUNNELING is explicitly set without
 * INET_CONFIG_ENABLE_TUN_ENDPOINT being set.
 *
 */
#if (CHIP_CONFIG_ENABLE_TUNNELING && !(INET_CONFIG_ENABLE_TUN_ENDPOINT))
#error chip Tunneling requires Tunnel EndPoint in InetLayer to be enabled
#endif // CHIP_CONFIG_ENABLE_TUNNELING

/**
 *  @def CHIP_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default time period(in seconds) at which
 *    the border gateway and the mobile device sends advertisements
 *    for any local tunneling peer to pick up and populate its
 *    nexthop cache in order to locally tunnel over UDP between them.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS
#define CHIP_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS   (5)
#endif // CHIP_CONFIG_TUNNELING_SHORTCUT_TUNNEL_ADV_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED
 *
 *  @brief
 *    This defines the default queue depth for queueing data packets
 *    destined for the Service when the connection to the Service
 *    is not yet established.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED
#define CHIP_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED              (8)
#endif // CHIP_CONFIG_TUNNELING_MAX_NUM_PACKETS_QUEUED

/**
 *  @def CHIP_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS
 *
 *  @brief
 *    This defines the default value for the maximum number of shortcut
 *    tunneling peers for which to keep an entry in the nexthop table.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS
#define CHIP_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS       (8)
#endif // CHIP_CONFIG_TUNNELING_MAX_NUM_SHORTCUT_TUNNEL_PEERS

/**
 *  @def CHIP_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS
 *
 *  @brief
 *    This defines whether (1) or not (0) the underlying platform-specific
 *    functions implemented within the chip Addressing and Routing Module,
 *    a.k.a. WARM, would override the corresponding supplied default
 *    implementation.
 *
 */
#ifndef CHIP_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS
#define CHIP_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS      (0)
#endif /* CHIP_TUNNEL_CONFIG_WILL_OVERRIDE_ADDR_ROUTING_FUNCS */

/**
 *  @def CHIP_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY
 *
 *  @brief
 *    This defines the default value for the maximum number of
 *    connection attempts before a notification is made about the
 *    connection failure.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY
#define CHIP_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY      (3)
#endif // CHIP_CONFIG_TUNNELING_MAX_NUM_CONNECT_BEFORE_NOTIFY

/**
 *  @def CHIP_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value of the response timeout
 *    (in seconds) for control messages sent over the
 *    tunnel.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS
#define CHIP_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS         (5)
#endif // CHIP_CONFIG_TUNNELING_CTRL_RESPONSE_TIMEOUT_SECS

/**
 *  @def CHIP_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval (in seconds) between
 *    keepalive probes for the primary tunnel's TCP connection.
 *    This value also controls the time between last data
 *    packet sent and the transmission of the first keepalive
 *    probe.
 *
 */
#ifndef CHIP_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS
#define CHIP_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS       (25)
#endif // CHIP_CONFIG_PRIMARY_TUNNEL_KEEPALIVE_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES
 *
 *  @brief
 *    This defines the default value for the maximum number of
 *    keepalive probes for both the primary and backup tunnel's
 *    TCP connection.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES
#define CHIP_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES                  (2)
#endif // CHIP_CONFIG_TUNNEL_MAX_KEEPALIVE_PROBES

/**
 *  @def CHIP_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value for the maximum timeout
 *    of unacknowledged data for the primary tunnel's TCP
 *    connection.
 *
 */
#ifndef CHIP_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS
#define CHIP_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS             (20)
#endif // CHIP_CONFIG_PRIMARY_TUNNEL_MAX_TIMEOUT_SECS

/**
 *  @def CHIP_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval (in seconds) between
 *    keepalive probes for the backup tunnel's TCP connection.
 *    This value also controls the time between last data
 *    packet sent and the transmission of the first keepalive
 *    probe.
 *
 */
#ifndef CHIP_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS
#define CHIP_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS       (50)
#endif // CHIP_CONFIG_BACKUP_TUNNEL_KEEPALIVE_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value for the maximum timeout
 *    of unacknowledged data for the backup tunnel's TCP
 *    connection.
 *
 */
#ifndef CHIP_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS
#define CHIP_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS              (10)
#endif // CHIP_CONFIG_BACKUP_TUNNEL_MAX_TIMEOUT_SECS

/**
 *  @def CHIP_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS
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
#ifndef CHIP_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS
#define CHIP_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS       (195)
#endif // CHIP_CONFIG_PRIMARY_TUNNEL_LIVENESS_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS
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
#ifndef CHIP_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS
#define CHIP_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS        (195)
#endif // CHIP_CONFIG_BACKUP_TUNNEL_LIVENESS_INTERVAL_SECS


/**
 *  @def CHIP_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default timeout for the tunnel connect
 *    attempt to either succeed or notify the caller of an
 *    error.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS
#define CHIP_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS                 (10)
#endif // CHIP_CONFIG_TUNNEL_CONNECT_TIMEOUT_SECS

/**
 *  @def CHIP_CONFIG_TUNNEL_FAILOVER_SUPPORTED
 *
 *  @brief
 *    This defines whether support for tunnel redundancy
 *    and failover is present.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_FAILOVER_SUPPORTED
#define CHIP_CONFIG_TUNNEL_FAILOVER_SUPPORTED                   (0)
#endif // CHIP_CONFIG_TUNNEL_FAILOVER_SUPPORTED

/**
 *  @def CHIP_CONFIG_TUNNEL_LIVENESS_SUPPORTED
 *
 *  @brief
 *    This defines whether support for the periodic tunnel
 *    liveness is present.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_LIVENESS_SUPPORTED
#define CHIP_CONFIG_TUNNEL_LIVENESS_SUPPORTED                   (1)
#endif // CHIP_CONFIG_TUNNEL_LIVENESS_SUPPORTED

/**
 *  @def CHIP_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    Keepalive socket option is present for the chip
 *    Tunnel.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED
#define CHIP_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED              (!CHIP_CONFIG_TUNNEL_LIVENESS_SUPPORTED)
#endif // CHIP_CONFIG_TUNNEL_TCP_KEEPALIVE_SUPPORTED

/**
 *  @def CHIP_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    UserTimeout socket option is present for the chip
 *    Tunnel.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED
#define CHIP_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED           (1)
#endif // CHIP_CONFIG_TUNNEL_TCP_USER_TIMEOUT_SUPPORTED

/**
 *  @def CHIP_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK
 *
 *  @brief
 *    This defines whether support for enabling the TCP
 *    send channel being idle is present for the chip
 *    Tunnel.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK
#define CHIP_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK             (INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS)
#endif // CHIP_CONFIG_TUNNEL_ENABLE_TCP_IDLE_CALLBACK

/**
 *  @def CHIP_CONFIG_TUNNEL_SHORTCUT_SUPPORTED
 *
 *  @brief
 *    This defines whether support for tunnel shortcut
 *    is present.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_SHORTCUT_SUPPORTED
#define CHIP_CONFIG_TUNNEL_SHORTCUT_SUPPORTED                   (0)
#endif // CHIP_CONFIG_TUNNEL_SHORTCUT_SUPPORTED

/**
 *  @def CHIP_CONFIG_TUNNEL_ENABLE_STATISTICS
 *
 *  @brief
 *    This defines whether support for tunnel statistics
 *    collection is present.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_ENABLE_STATISTICS
#define CHIP_CONFIG_TUNNEL_ENABLE_STATISTICS                    (1)
#endif // CHIP_CONFIG_TUNNEL_ENABLE_STATISTICS

/**
 *  @def CHIP_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK
 *
 *  @brief
 *    This defines whether the transiting packet would be
 *    passed to the application, for example, for logging purposes.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK
#define CHIP_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK              (1)
#endif // CHIP_CONFIG_TUNNEL_ENABLE_TRANSIT_CALLBACK

/**
 *  @def CHIP_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX
 *
 *  @brief
 *    This defines the maximum fibonacci index value for the
 *    back-off algortihm to generate the next reconnect time.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX
#define CHIP_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX     (14)
#endif // CHIP_CONFIG_TUNNELING_RECONNECT_MAX_FIBONACCI_INDEX

/**
 *  @def CHIP_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS
 *
 *  @brief
 *    This specifies the multiplying factor to the result of
 *    a fibonacci computation based on a specific index to provide
 *    a max wait time for the next connection attempt.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS
#define CHIP_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS (10)
#endif // CHIP_CONFIG_TUNNELING_CONNECT_WAIT_TIME_MULTIPLIER_SECS

/**
 *  @def CHIP_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT
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
#ifndef CHIP_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT
#define CHIP_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT    (30)
#endif // CHIP_CONFIG_TUNNELING_MIN_WAIT_TIME_INTERVAL_PERCENT

/**
 *  @def CHIP_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS
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
#ifndef CHIP_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS
#define CHIP_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS      (10)
#endif // CHIP_CONFIG_TUNNELING_RESET_RECONNECT_TIMEOUT_SECS

/**
 *  @def CHIP_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS
 *
 *  @brief
 *    The frequency of performing a network online check when
 *    the corresponding tunnel is down.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS
#define CHIP_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS       (30)
#endif // CHIP_CONFIG_TUNNELING_ONLINE_CHECK_FAST_FREQ_SECS

/**
 *  @def CHIP_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS
 *
 *  @brief
 *    The slower frequency of performing a network online check
 *    after detecting that the network has come online and before
 *    corresponding tunnel is established.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS
#define CHIP_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS  (60)
#endif // CHIP_CONFIG_TUNNELING_ONLINE_CHECK_PRIMARY_SLOW_FREQ_SECS

/**
 *  @def CHIP_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS
 *
 *  @brief
 *    The slower frequency of performing a network online check
 *    after detecting that the network has come online and before
 *    backup tunnel is established.
 *
 */
#ifndef CHIP_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS
#define CHIP_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS   (90)
#endif // CHIP_CONFIG_TUNNELING_ONLINE_CHECK_BACKUP_SLOW_FREQ_SECS

/**
 *  @def CHIP_CONFIG_TUNNEL_INTERFACE_MTU
 *
 *  @brief
 *    The maximum transmission unit for the Tunnel interface
 *    on a border gateway.
 *
 *  @note
 *    Setting a non-default value may also require updating
 *    CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX for socket
 *    platforms and PBUF size for LwIP platforms.
 *
 */
#ifndef CHIP_CONFIG_TUNNEL_INTERFACE_MTU
#define CHIP_CONFIG_TUNNEL_INTERFACE_MTU                           (1536)
#endif // CHIP_CONFIG_TUNNEL_INTERFACE_MTU

// clang-format on

#endif /* CHIP_TUNNEL_CONFIG_H_ */
