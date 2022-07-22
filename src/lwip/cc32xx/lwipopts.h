/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *      Compile-time configuration for LwIP on the TI CC32xx.
 *
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <stdlib.h>

#define NO_SYS 0
#define MEM_ALIGNMENT (4)
#define MEMP_NUM_TCP_SEG (TCP_SND_QUEUELEN + 1)
#define LWIP_TIMEVAL_PRIVATE (0)
#define MEM_LIBC_MALLOC (0)
#define LWIP_COMPAT_MUTEX (0)
#define SYS_LIGHTWEIGHT_PROT (1)
#define LWIP_AUTOIP (1)
#define LWIP_DHCP_AUTOIP_COOP (0)
#define LWIP_SOCKET_SET_ERRNO 0
#define IP_REASS_MAX_PBUFS 0
#define IP_REASSEMBLY 0
#define MEMP_NUM_REASSDATA 0
#define LWIP_SO_RCVTIMEO 0
#define SO_REUSE (1)
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS (1)
#define LWIP_STATS (0)
#define LWIP_TCPIP_CORE_LOCKING 1
#define TCP_QUEUE_OOSEQ 0
#define ARP_QUEUEING (0)
#define TCPIP_THREAD_NAME "LWIP"

#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 1

#define LWIP_SOCKET 0

// TODO: seems like this is unnecessary on Thread-only platforms
#define LWIP_RAW 1
#define MEMP_NUM_RAW_PCB (5)

#define MEMP_NUM_UDP_PCB (7)

#define LWIP_HAVE_LOOPIF (0)

// TODO: not sure why this is disabled
#define LWIP_NETIF_LOOPBACK (0)

#define MEMP_NUM_NETCONN (0)

// enable ethernet for ipv6 support
#define LWIP_ETHERNET 1

#define LWIP_IPV4 1
#define LWIP_IPV6 1
#define LWIP_ARP (1)
#define LWIP_DNS (0)
#define LWIP_ICMP (0)
#define LWIP_IGMP (1)
#define LWIP_DHCP (1)
#define LWIP_IPV6_REASS (0)
#define LWIP_IPV6_DHCP6 0
#define LWIP_IPV6_AUTOCONFIG (0)
#define LWIP_IPV6_ROUTER_SUPPORT 0
#define LWIP_ND6_LISTEN_RA 0

#define LWIP_ND6_NUM_NEIGHBORS (0)
#define LWIP_ND6_NUM_DESTINATIONS (0)
#define LWIP_ND6_NUM_PREFIXES (0)
#define LWIP_ND6_NUM_ROUTERS (0)
#define LWIP_ND6_MAX_MULTICAST_SOLICIT (0)
#define LWIP_ND6_MAX_UNICAST_SOLICIT (0)
#define LWIP_ND6_MAX_NEIGHBOR_ADVERTISEMENT (0)
#define LWIP_ND6_TCP_REACHABILITY_HINTS (0)

#define MEMP_SEPARATE_POOLS (1)
#define LWIP_PBUF_FROM_CUSTOM_POOLS (0)
#define MEMP_USE_CUSTOM_POOLS (0)
#define PBUF_POOL_SIZE (16)
#define PBUF_POOL_BUFSIZE (1500)
#define PBUF_POOL_SIZE_LARGE (3)
#define PBUF_POOL_SIZE_MEDIUM (4)
#define PBUF_POOL_SIZE_SMALL (5)
#define PBUF_POOL_BUFSIZE_LARGE (1280)
#define PBUF_POOL_BUFSIZE_MEDIUM (640)
#define PBUF_POOL_BUFSIZE_SMALL (256)
#define PBUF_CUSTOM_POOL_IDX_START (MEMP_PBUF_POOL_SMALL)
#define PBUF_CUSTOM_POOL_IDX_END (MEMP_PBUF_POOL_LARGE)

#define TCP_MSS (1152)
#define TCP_SND_BUF (2 * TCP_MSS)
#define TCP_LISTEN_BACKLOG (1)

#define ETH_PAD_SIZE (0)
#define SUB_ETHERNET_HEADER_SPACE (0)
#define PBUF_LINK_HLEN (14)

#define TCPIP_THREAD_STACKSIZE (4096)
#define TCPIP_THREAD_PRIO (2)

#define NETIF_MAX_HWADDR_LEN 8U

#define LWIP_IPV6_NUM_ADDRESSES 5

#define LWIP_IPV6_ND 1
#define LWIP_IPV6_MLD 1
#define LWIP_ND6_QUEUEING 0

#define LWIP_MULTICAST_PING 0

#define TCPIP_MBOX_SIZE 6
#define DEFAULT_RAW_RECVMBOX_SIZE 6
#define DEFAULT_UDP_RECVMBOX_SIZE 6
#define DEFAULT_TCP_RECVMBOX_SIZE 6

// TODO: make LWIP_DEBUG conditional on build type

#ifndef LWIP_DEBUG
#define LWIP_DEBUG 1
#endif

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

#define LWIP_DBG_TYPES_ON                                                                                                          \
    (LWIP_DBG_ON | LWIP_DBG_TRACE) /* (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT) */

#endif /* __LWIPOPTS_H__ */
