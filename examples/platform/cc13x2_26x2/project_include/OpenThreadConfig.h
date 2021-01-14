/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *      Overrides to default OpenThread configuration.
 *
 */

#ifndef OPENTHREAD_PLATFORM_CONFIG_H
#define OPENTHREAD_PLATFORM_CONFIG_H

// use the logging in the CHIP platform layer
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_APP

// Turn on a moderate level of logging in OpenThread
// Enable use of external heap allocator (calloc/free) for OpenThread.
#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1

// Reduced for RAM consumption reasons
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 22

/* Define to 1 if you want to enable CoAP to an application. */
#define OPENTHREAD_CONFIG_COAP_API_ENABLE 1

/* Define to 1 to enable the border agent feature. */
#define OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE 0

/* Define to 1 if you want to enable Border Router */
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE 0

/* Define to 1 if you want to enable log for certification test */
#define OPENTHREAD_CONFIG_REFERENCE_DEVICE_ENABLE 0

/* Define to 1 if you want to enable channel manager feature */
#define OPENTHREAD_CONFIG_CHANNEL_MANAGER_ENABLE 0

/* Define to 1 if you want to use channel monitor feature */
#define OPENTHREAD_CONFIG_CHANNEL_MONITOR_ENABLE 0

/* Define to 1 if you want to use child supervision feature */
#define OPENTHREAD_CONFIG_CHILD_SUPERVISION_ENABLE 0

/* Define to 1 to enable the commissioner role. */
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 0

/* Define as 1 to enable support for adding of auto-configured SLAAC addresses by OpenThread */
#define OPENTHREAD_CONFIG_IP6_SLAAC_ENABLE 1

/* Define to 1 if you want to enable multicast DHCPv6 Solicit messages */
#define OPENTHREAD_ENABLE_DHCP6_MULTICAST_SOLICIT 1

/* Define to 1 if you want to enable DHCPv6 Client */
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE 1

/* Define to 1 if you want to enable DHCPv6 Server */
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 0

/* Define to 1 if you want to use diagnostics module */
#define OPENTHREAD_CONFIG_DIAG_ENABLE 1

/* Define to 1 if you want to enable DNS Client */
#define OPENTHREAD_CONFIG_DNS_CLIENT_ENABLE 0

/* Define to 1 to enable dtls support. */
#define OPENTHREAD_CONFIG_DTLS_ENABLE 1

/* Define to 1 if you want to use jam detection feature */
#define OPENTHREAD_CONFIG_JAM_DETECTION_ENABLE 0

/* Define to 1 to enable the joiner role. */
#define OPENTHREAD_CONFIG_JOINER_ENABLE 1

/* Define to 1 if you want to use legacy network support */
#define OPENTHREAD_CONFIG_LEGACY_ENABLE 0

/* Define to 1 if you want to use MAC filter feature */
#define OPENTHREAD_CONFIG_MAC_FILTER_ENABLE 0

/* Define to 1 to enable network diagnostic for MTD. */
#define OPENTHREAD_CONFIG_TMF_NETWORK_DIAG_MTD_ENABLE 0

/* Define to 1 if you want to enable support for multiple OpenThread
   instances. */
#define OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE 0

/* Define to 1 to enable the NCP SPI interface. */
#define OPENTHREAD_CONFIG_NCP_SPI_ENABLE 0

/* Define to 1 if using NCP Spinel Encrypter */
#define OPENTHREAD_ENABLE_NCP_SPINEL_ENCRYPTER 0

/* Define to 1 to enable the NCP UART interface. */
#define OPENTHREAD_CONFIG_NCP_UART_ENABLE 1

/* Define to 1 if using NCP vendor hook */
#define OPENTHREAD_ENABLE_NCP_VENDOR_HOOK 0

/* Define to 1 if you want to enable raw link-layer API */
#define OPENTHREAD_CONFIG_LINK_RAW_ENABLE 1

/* Define to 1 if you want to enable Service */
#define OPENTHREAD_CONFIG_TMF_NETDATA_SERVICE_ENABLE 0

/* Define to 1 to enable the UDP proxy feature. */
#define OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE 0

#define ENABLE_ALL_LOGS_TO_UART 0
#if ENABLE_ALL_LOGS_TO_UART
/* Enable volumous logging via the below */

/* Where should logs go?
 * For details & options see:
 * ${openthread}/src/core/openthread-core-default-config.h
 * Note: Default is: OPENTHREAD_CONFIG_LOG_OUTPUT_NONE
 */
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_DEBUG_UART

/* Details of the debug uart can be found in the file:
 * 'platform/dbg_uart.c' file within the examples.
 */

/*
 * This enables the Debug Uart, disabled by default
 */
#define OPENTHREAD_CONFIG_ENABLE_DEBUG_UART 1

/*
 * For details & selections see:
 * ${openthread}/include/openthread/platform/logging.h
 */
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_DEBG

/* Each significant area openthread can be configured to log, or not log
 * The areas are enabled via various #defines, see below for a partial list.
 * For more details, see the openthread internal ".hpp"
 *
 * See:  ${openthread}/openthread/src/core/common/logging.hpp
 * Also note above level is defined in the single ".h" log header
 * Whereas each area is controlled by the C++ header ".hpp" log header.
 */
#define OPENTHREAD_CONFIG_LOG_API 1
#define OPENTHREAD_CONFIG_LOG_ARP 1
#define OPENTHREAD_CONFIG_LOG_CLI 1
#define OPENTHREAD_CONFIG_LOG_COAP 1
#define OPENTHREAD_CONFIG_LOG_ICMP 1
#define OPENTHREAD_CONFIG_LOG_IP6 1
#define OPENTHREAD_CONFIG_LOG_MAC 1
#define OPENTHREAD_CONFIG_LOG_MEM 1
#define OPENTHREAD_CONFIG_LOG_MLE 1
#define OPENTHREAD_CONFIG_LOG_NETDATA 1
#define OPENTHREAD_CONFIG_LOG_NETDIAG 1
#define OPENTHREAD_CONFIG_LOG_PKT_DUMP 1
#define OPENTHREAD_CONFIG_LOG_PLATFORM 1
#define OPENTHREAD_CONFIG_LOG_PREPEND_LEVEL 1
#define OPENTHREAD_CONFIG_LOG_PREPEND_REGION 1

#endif

/* match the output buffer size to Diag process max buffer.
 * size = 256. Otherwise some output chars are chopped
 */
#define OPENTHREAD_CONFIG_CLI_MAX_LINE_LENGTH 256

#endif // OPENTHREAD_PLATFORM_CONFIG_H
