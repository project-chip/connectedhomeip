/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *      Compile-time configuration for LwIP on PSOC6 platforms using the
 *      PSOC6 SDK.
 *
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h>
#endif

#include <stdlib.h>

#include <whd_types.h>

#define MEM_ALIGNMENT (4)

#define LWIP_RAW (1)

//
// Enable IPV4 networking
//
#define LWIP_IPV4 (1)

//
// Enable IPV6 networking
//
#define LWIP_IPV6 (1)

#define ETHARP_SUPPORT_STATIC_ENTRIES (1)

//
// Enable IPV4 networking
//
#define LWIP_ICMP (1)
#define LWIP_TCP (1)
#define LWIP_UDP (1)
#define LWIP_IGMP (1)

//
// Use malloc to allocate any memory blocks instead of the
// malloc that is part of LWIP
//
#define MEM_LIBC_MALLOC (1)

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
//
// Use the timeval from the GCC library, not the one
// from LWIP
//
#define LWIP_TIMEVAL_PRIVATE (0)
#endif

//
// Make sure DHCP is part of the stack
//
#define LWIP_DHCP (1)

//
// Enable LwIP send timeout
//
#define LWIP_SO_SNDTIMEO (1)

//
// Enable LwIP receive timeout
//
#define LWIP_SO_RCVTIMEO (1)

//
// Enable SO_REUSEADDR option
//
#define SO_REUSE (1)

//
// Enable TCP Keep-alive
//
#define LWIP_TCP_KEEPALIVE (0)

//
// The amount of space to leave before the packet when allocating a pbuf. Needs to
// be enough for the link layer data and the WHD header
//
#define PBUF_LINK_HLEN (WHD_PHYSICAL_HEADER)

//
// TCP Maximum segment size
//
#define TCP_MSS (WHD_PAYLOAD_MTU)

#define LWIP_CHECKSUM_CTRL_PER_NETIF 1
#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_GEN_TCP 1
#define CHECKSUM_GEN_ICMP 1
#define CHECKSUM_GEN_ICMP6 1
#define CHECKSUM_CHECK_IP 1
#define CHECKSUM_CHECK_UDP 1
#define CHECKSUM_CHECK_TCP 1
#define CHECKSUM_CHECK_ICMP 1
#define CHECKSUM_CHECK_ICMP6 1
#define LWIP_CHECKSUM_ON_COPY 1

//
// Enable the thread safe NETCONN interface layer
//
#define LWIP_NETCONN (1)

/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * To achieve good performance, this should be at least 2 * TCP_MSS.
 */
#define TCP_SND_BUF (4 * TCP_MSS)

/**
 * TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
 * as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work.
 */
#define TCP_SND_QUEUELEN ((6 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

//
// Taken from WICED to speed things up
//
#define DHCP_DOES_ARP_CHECK (0)

//
// Light weight protection for things that may be clobbered by interrupts
//
#define SYS_LIGHTWEIGHT_PROT (1)
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT (1)

#define LWIP_SO_RCVBUF (128)

#define LWIP_SOCKET (1)
#define LWIP_NETCONN (1)
#define DEFAULT_TCP_RECVMBOX_SIZE (6)
#define TCPIP_MBOX_SIZE (6)
#define TCPIP_THREAD_STACKSIZE (2 * 1024)
#define TCPIP_THREAD_PRIO (2)
#define DEFAULT_RAW_RECVMBOX_SIZE (6)
#define DEFAULT_UDP_RECVMBOX_SIZE (6)
#define DEFAULT_ACCEPTMBOX_SIZE (6)
#define TCPIP_THREAD_NAME "LWIP"

/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#define MEMP_NUM_UDP_PCB 8

/**
 * MEMP_NUM_TCP_PCB: the number of simultaneously active TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB 8

/**
 * MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP connections.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_PCB_LISTEN 1

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG 27

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simultaneously active timeouts.
 */
#define MEMP_NUM_SYS_TIMEOUT 12

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 */
#define PBUF_POOL_SIZE 48

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF 8

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN 16

/**
 * LWIP_TCPIP_CORE_LOCKING
 * Creates a global mutex that is held during TCPIP thread operations.
 * Can be locked by client code to perform lwIP operations without changing
 * into TCPIP thread using callbacks. See LOCK_TCPIP_CORE() and
 * UNLOCK_TCPIP_CORE().
 * Your system should provide mutexes supporting priority inversion to use this.
 */
#define LWIP_TCPIP_CORE_LOCKING 1

/**
 * LWIP_TCPIP_CORE_LOCKING_INPUT: when LWIP_TCPIP_CORE_LOCKING is enabled,
 * this lets tcpip_input() grab the mutex for input packets as well,
 * instead of allocating a message and passing it to tcpip_thread.
 *
 * ATTENTION: this does not work when tcpip_input() is called from
 * interrupt context!
 */
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1

/**
 * LWIP_NETIF_API==1: Support netif api (in netifapi.c)
 */
#define LWIP_NETIF_API 1

#define LWIP_DNS (1)

#define LWIP_NETIF_TX_SINGLE_PBUF (1)

#define LWIP_RAND rand

#define LWIP_FREERTOS_CHECK_CORE_LOCKING (1)

#define LWIP_NETIF_STATUS_CALLBACK (1)
#define LWIP_NETIF_LINK_CALLBACK (1)
#define LWIP_NETIF_REMOVE_CALLBACK (1)

#define LWIP_CHKSUM_ALGORITHM (3)

#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#define netifapi_dhcp_release_and_stop(n) netifapi_dhcp_release(n)

#ifndef LWIP_DEBUG
#define LWIP_DEBUG 1
#endif
#define LWIP_STATS (0)

#define MEMP_OVERFLOW_CHECK (0)
#define MEMP_SANITY_CHECK (0)
#define MEM_DEBUG (LWIP_DBG_OFF)
#define MEMP_DEBUG (LWIP_DBG_OFF)
#define PBUF_DEBUG (LWIP_DBG_OFF)
#define API_LIB_DEBUG (LWIP_DBG_OFF)
#define API_MSG_DEBUG (LWIP_DBG_OFF)
#define TCPIP_DEBUG (LWIP_DBG_OFF)
#define NETIF_DEBUG (LWIP_DBG_OFF)
#define SOCKETS_DEBUG (LWIP_DBG_OFF)
#define DEMO_DEBUG (LWIP_DBG_OFF)
#define DHCP_DEBUG (LWIP_DBG_OFF)
#define AUTOIP_DEBUG (LWIP_DBG_OFF)
#define ETHARP_DEBUG (LWIP_DBG_OFF)
#define IP_DEBUG (LWIP_DBG_OFF)
#define IP_REASS_DEBUG (LWIP_DBG_OFF)
#define IP6_DEBUG (LWIP_DBG_OFF)
#define RAW_DEBUG (LWIP_DBG_OFF)
#define ICMP_DEBUG (LWIP_DBG_OFF)
#define UDP_DEBUG (LWIP_DBG_OFF)
#define TCP_DEBUG (LWIP_DBG_OFF)
#define TCP_INPUT_DEBUG (LWIP_DBG_OFF)
#define TCP_OUTPUT_DEBUG (LWIP_DBG_OFF)
#define TCP_RTO_DEBUG (LWIP_DBG_OFF)
#define TCP_CWND_DEBUG (LWIP_DBG_OFF)
#define TCP_WND_DEBUG (LWIP_DBG_OFF)
#define TCP_FR_DEBUG (LWIP_DBG_OFF)
#define TCP_QLEN_DEBUG (LWIP_DBG_OFF)
#define TCP_RST_DEBUG (LWIP_DBG_OFF)
#define PPP_DEBUG (LWIP_DBG_OFF)

#define LWIP_DBG_TYPES_ON (LWIP_DBG_ON | LWIP_DBG_TRACE)

#endif /* __LWIPOPTS_H__ */
