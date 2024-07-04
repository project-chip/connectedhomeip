/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#pragma once

#include <lib/core/CHIPConfig.h>

namespace chip {

/**
 * @def CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS
 *
 * @brief Maximum Number of TCP connections a device can simultaneously have
 */
#ifndef CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS
#define CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS 4
#endif

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS < 1
#error "If TCP is enabled, the device needs to support at least 1 TCP connection"
#endif

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS > INET_CONFIG_NUM_TCP_ENDPOINTS
#error "If TCP is enabled, the maximum number of connections cannot exceed the number of tcp endpoints"
#endif

/**
 * @def CHIP_CONFIG_MAX_TCP_PENDING_PACKETS
 *
 * @brief Maximum Number of outstanding pending packets in the queue before a TCP connection
 *        needs to be established
 */
#ifndef CHIP_CONFIG_MAX_TCP_PENDING_PACKETS
#define CHIP_CONFIG_MAX_TCP_PENDING_PACKETS 4
#endif

/**
 *  @def CHIP_CONFIG_TCP_CONNECT_TIMEOUT_MSECS
 *
 *  @brief
 *    This defines the default timeout for the TCP connect
 *    attempt to either succeed or notify the caller of an
 *    error.
 *
 */
#ifndef CHIP_CONFIG_TCP_CONNECT_TIMEOUT_MSECS
#define CHIP_CONFIG_TCP_CONNECT_TIMEOUT_MSECS (10000)
#endif // CHIP_CONFIG_TCP_CONNECT_TIMEOUT_MSECS

/**
 *  @def CHIP_CONFIG_KEEPALIVE_INTERVAL_SECS
 *
 *  @brief
 *    This defines the default interval (in seconds) between
 *    keepalive probes for a TCP connection.
 *    This value also controls the time between last data
 *    packet sent and the transmission of the first keepalive
 *    probe.
 *
 */
#ifndef CHIP_CONFIG_TCP_KEEPALIVE_INTERVAL_SECS
#define CHIP_CONFIG_TCP_KEEPALIVE_INTERVAL_SECS (25)
#endif // CHIP_CONFIG_TCP_KEEPALIVE_INTERVAL_SECS

/**
 *  @def CHIP_CONFIG_MAX_TCP_KEEPALIVE_PROBES
 *
 *  @brief
 *    This defines the default value for the maximum number of
 *    keepalive probes for a TCP connection.
 *
 */
#ifndef CHIP_CONFIG_MAX_TCP_KEEPALIVE_PROBES
#define CHIP_CONFIG_MAX_TCP_KEEPALIVE_PROBES (5)
#endif // CHIP_CONFIG_MAX_TCP_KEEPALIVE_PROBES

/**
 *  @def CHIP_CONFIG_MAX_UNACKED_DATA_TIMEOUT_SECS
 *
 *  @brief
 *    This defines the default value for the maximum timeout
 *    of unacknowledged data for a TCP connection.
 *
 */
#ifndef CHIP_CONFIG_MAX_UNACKED_DATA_TIMEOUT_SECS
#define CHIP_CONFIG_MAX_UNACKED_DATA_TIMEOUT_SECS (30)
#endif // CHIP_CONFIG_MAX_UNACKED_DATA_TIMEOUT_SECS

} // namespace chip
