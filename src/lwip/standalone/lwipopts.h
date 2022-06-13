/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2014-2018 Nest Labs, Inc.
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
 *      This file describes compile-time constants for configuring LwIP
 *      for use in standalone (desktop) environments.
 *
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h>
#endif

#include <stdlib.h>

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS 0

/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#define MEM_ALIGNMENT (4)

/**
 * MEM_SIZE: specify bigger memory size to pass LwIP-internal unit tests
 * (only needed when building tests)
 */
#ifdef CHIP_WITH_TESTS
#define MEM_SIZE (16000)
#endif

/**
 * Use Malloc from LibC - saves code space
 */
#define MEM_LIBC_MALLOC (0)

/**
 * Do not use memory pools to create fixed, statically allocated pools of
 * memory in lieu of the Standard C Library heap and APIs.
 */
#define MEM_USE_POOLS (0)

/**
 * Do not use custom memory pools for specific, named LwIP objects, sourced
 * from lwippools.h.
 */
#define MEM_USE_CUSTOM_POOLS (MEM_USE_POOLS)

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETBUF (PBUF_POOL_SIZE)

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
#define MEMP_NUM_TCP_SEG (TCP_SND_QUEUELEN + 1)

/**
 * PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
 *
 * This is just a default designed to be overridden by the FreeRTOS.mk makefile
 * To perform this override, define the makefile variable LWIP_NUM_PACKET_BUFFERS_IN_POOL
 */
#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE (10)
#endif

/*
 * IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.
 * Since the received pbufs are enqueued, be sure to configure
 * PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS so that the stack is still able to receive
 * packets even if the maximum amount of fragments is enqueued for reassembly!
 *
 */
#if PBUF_POOL_SIZE > 2
#ifndef IP_REASS_MAX_PBUFS
#define IP_REASS_MAX_PBUFS (PBUF_POOL_SIZE - 2)
#endif
#else
#define IP_REASS_MAX_PBUFS 0
#define IP_REASSEMBLY 0
#endif

/**
 * MEMP_NUM_REASSDATA: the number of IP packets simultaneously queued for
 * reassembly (whole packets, not fragments!)
 */
#if IP_REASS_MAX_PBUFS > 1
#ifndef MEMP_NUM_REASSDATA
#define MEMP_NUM_REASSDATA (IP_REASS_MAX_PBUFS - 1)
#endif
#else
#define MEMP_NUM_REASSDATA 0
#endif

#define PAYLOAD_MTU (1500)

/**
 * TCP_MSS: TCP Maximum segment size. (default is 536, a conservative default,
 * you might want to increase this.)
 * For the receive side, this MSS is advertised to the remote side
 * when opening a connection. For the transmit size, this MSS sets
 * an upper limit on the MSS advertised by the remote host.
 */
#define TCP_MSS (1152)

/**
 * PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. The default is
 * designed to accommodate single full size link-layer frame in one pbuf, including
 * the link-layer header and any link-layer encapsulation header, and the pbuf
 * structure itself.
 */

#define PBUF_POOL_BUFSIZE                                                                                                          \
    LWIP_MEM_ALIGN_SIZE(PAYLOAD_MTU + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN) + LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf) + 1)

/**
 * TCP_SND_BUF: TCP sender buffer space (bytes).
 * must be at least as much as (2 * TCP_MSS) for things to work smoothly
 */
#define TCP_SND_BUF (6 * TCP_MSS)

/**
 * ETH_PAD_SIZE: the header space required preceeding the of each pbuf in the pbuf pool. The default is
 * designed to accommodate single full size TCP frame in one pbuf, including
 * TCP_MSS, IP header, and link header.
 *
 * This is zero since the role has been taken over by SUB_ETHERNET_HEADER_SPACE as ETH_PAD_SIZE was not always obeyed
 */
#define ETH_PAD_SIZE (0)

/**
 * LWIP_NETIF_TX_SINGLE_PBUF: if this is set to 1, lwIP tries to put all data
 * to be sent into one single pbuf. This is for compatibility with DMA-enabled
 * MACs that do not support scatter-gather.
 * Beware that this might involve CPU-memcpy before transmitting that would not
 * be needed without this flag! Use this only if you need to!
 *
 * @todo: TCP and IP-frag do not work with this, yet:
 */
#define LWIP_NETIF_TX_SINGLE_PBUF (0)

/** Define LWIP_COMPAT_MUTEX if the port has no mutexes and binary semaphores
 *  should be used instead
 */
#define LWIP_COMPAT_MUTEX (1)

/** Define LWIP_COMPAT_MUTEX_ALLOWED if the platform concurrency model has no
 * support for avoiding priority inversion deadlocks
 */
#define LWIP_COMPAT_MUTEX_ALLOWED (1)

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT (0)

/**
 * TCPIP_THREAD_STACKSIZE: The stack size used by the main tcpip thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define TCPIP_THREAD_STACKSIZE (1300)

/**
 * TCPIP_THREAD_PRIO: The priority assigned to the main tcpip thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#define TCPIP_THREAD_PRIO (7)

#define TCP_LISTEN_BACKLOG (1)

/**
 * LWIP_DHCP==1: Enable DHCP module.
 */
#define LWIP_DHCP (1)

/**
 * Enable automatic IPv4 link-local address assignment.
 */
#define LWIP_AUTOIP 1

/**
 * Allow DHCP and automatic IPv4 link-local address assignment to
 * work cooperatively.
 */
#define LWIP_DHCP_AUTOIP_COOP 1

/**
 * LWIP_PROVIDE_ERRNO: errno definitions from the Standard C Library.
 */
#undef LWIP_PROVIDE_ERRNO

/**
 * ERRNO: set errno on interface invocation failures
 */
#define ERRNO (1)

/**
 * MEMP_NUM_RAW_PCB: Number of raw connection PCBs
 * (requires the LWIP_RAW option)
 */
#ifndef MEMP_NUM_RAW_PCB
#define MEMP_NUM_RAW_PCB (5)
#endif

/**
 * MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
 * per active UDP "connection".
 * (requires the LWIP_UDP option)
 */
#ifndef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB (6)
#endif

/**
 * MEMP_NUM_SYS_TIMEOUT: the number of simulateously active timeouts.
 * (requires NO_SYS==0)
 * Must be larger than or equal to LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS +
 * PPP_SUPPORT Since each InetTimer requires one matching LwIP timeout (if built with LwIP option), the number should be expanded to
 * be (All LwIP needs) + (max number of InetTimers)
 */
#define MEMP_NUM_SYS_TIMEOUT (48)

/* ARP before DHCP causes multi-second delay  - turn it off */
#define DHCP_DOES_ARP_CHECK (0)

/**
 * LWIP_HAVE_LOOPIF==1: Support loop interface (127.0.0.1) and loopif.c
 */
#define LWIP_HAVE_LOOPIF (1)

/**
 * LWIP_NETIF_LOOPBACK==1: Support sending packets with a destination IP
 * address equal to the netif IP address, looping them back up the stack.
 */
#define LWIP_NETIF_LOOPBACK (0)

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN (8)

/**
 * LWIP_SO_RCVTIMEO==1: Enable SO_RCVTIMEO processing.
 */
#define LWIP_SO_RCVTIMEO (1)

/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#define LWIP_IGMP (1)

/**
 * SO_REUSE==1: Enable SO_REUSEADDR option.
 * Required by IGMP for reuse of multicast address and port by other sockets
 */
#define SO_REUSE (1)

/**
 * LWIP_DNS==1: Turn on DNS module. UDP must be available for DNS
 * transport.
 */
#define LWIP_DNS (1)

/**
 * LWIP_POSIX_SOCKETS_IO_NAMES==1: Enable POSIX-style sockets functions names.
 * Disable this option if you use a POSIX operating system that uses the same
 * names (read, write & close). (only used if you use sockets.c)
 *
 * We disable this because this otherwise collides with the Standard C
 * Library where both LWIP and its headers are included.
 */
#define LWIP_POSIX_SOCKETS_IO_NAMES (0)

#ifdef LWIP_SO_RCVBUF
#if (LWIP_SO_RCVBUF == 1)
#include <limits.h> /* Needed because RECV_BUFSIZE_DEFAULT is defined as INT_MAX */
#endif              /* if ( LWIP_SO_RCVBUF == 1 ) */
#endif              /* ifdef LWIP_SO_RCVBUF */

/**
 * LWIP_STATS : Turn on statistics gathering
 */
#define LWIP_STATS (1)

/**
 * LWIP_IPV6==1: Enable IPv6
 */
#ifndef LWIP_IPV6
#define LWIP_IPV6 1
#endif

/**
 * LWIP_IPV6_DHCP6==1: enable DHCPv6 stateful address autoconfiguration.
 */
#ifndef LWIP_IPV6_DHCP6
#define LWIP_IPV6_DHCP6 1
#endif

/**
 * LWIP_IPV6_MLD==1: Enable multicast listener discovery protocol.
 */
#ifndef LWIP_IPV6_MLD
#define LWIP_IPV6_MLD 1
#endif

/**
 * MEMP_NUM_MLD6_GROUP: Maximum number of IPv6 multicast groups that
 * can be joined. Allocate one (1) for the link local address
 * solicited node multicast group, one (1) for the any/unspecified
 * address solicited node multicast group (which seems to be used
 * for/by DAD in this epoch of LwIP), and another four (4) for
 * application groups.
 */
#define MEMP_NUM_MLD6_GROUP ((1 + 1) + 4)

/**
 * LWIP_IPV6_FORWARD==1: Enable IPv6 forwarding.
 */
#ifndef LWIP_IPV6_FORWARD
#define LWIP_IPV6_FORWARD 1
#endif

/**
 * LWIP_IPV6_ROUTE_TABLE_SUPPORT==1: Enable support for a routing table and referring these during forwarding.
 */
#ifndef LWIP_IPV6_ROUTE_TABLE_SUPPORT
#define LWIP_IPV6_ROUTE_TABLE_SUPPORT 1
#endif

/**
 * IPV6_FRAG_COPYHEADER==1: Enable copying of IPv6 fragment headers on 64-bit platforms.
 */
#ifndef IPV6_FRAG_COPYHEADER
#if defined(__x86_64__)
#define IPV6_FRAG_COPYHEADER 1
#else
#define IPV6_FRAG_COPYHEADER 0
#endif
#endif

/**
 * Debug printing
 * By default enable debug printing for debug build, but set level to off
 * This allows user to change any desired debug level to on.
 */
#ifdef LWIP_DEBUG

#define MEMP_OVERFLOW_CHECK (1)
#define MEMP_SANITY_CHECK (1)

#define MEM_DEBUG LWIP_DBG_OFF
#define MEMP_DEBUG LWIP_DBG_OFF
#define PBUF_DEBUG LWIP_DBG_ON
#define API_LIB_DEBUG LWIP_DBG_ON
#define API_MSG_DEBUG LWIP_DBG_ON
#define TCPIP_DEBUG LWIP_DBG_ON
#define NETIF_DEBUG LWIP_DBG_ON
#define SOCKETS_DEBUG LWIP_DBG_ON
#define DEMO_DEBUG LWIP_DBG_ON
#define IP_DEBUG LWIP_DBG_ON
#define IP6_DEBUG LWIP_DBG_ON
#define IP_REASS_DEBUG LWIP_DBG_ON
#define RAW_DEBUG LWIP_DBG_ON
#define ICMP_DEBUG LWIP_DBG_ON
#define UDP_DEBUG LWIP_DBG_ON
#define TCP_DEBUG LWIP_DBG_ON
#define TCP_INPUT_DEBUG LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG LWIP_DBG_ON
#define TCP_RTO_DEBUG LWIP_DBG_ON
#define TCP_CWND_DEBUG LWIP_DBG_ON
#define TCP_WND_DEBUG LWIP_DBG_ON
#define TCP_FR_DEBUG LWIP_DBG_ON
#define TCP_QLEN_DEBUG LWIP_DBG_ON
#define TCP_RST_DEBUG LWIP_DBG_ON
#define PPP_DEBUG LWIP_DBG_OFF

extern unsigned char gLwIP_DebugFlags;
#define LWIP_DBG_TYPES_ON gLwIP_DebugFlags

#endif

/**
 * The WICED definition of PBUF_POOL_BUFSIZE includes a number of
 * sizeof() instantiations which causes the C preprocessor to
 * fail. Disable TCP configuration constant sanity checks to work
 * around this.
 */
#define LWIP_DISABLE_TCP_SANITY_CHECKS (1)

/**
 * LwIP defaults the size of most mailboxes (i.e. message queues) to
 * zero (0). That generally makes RTOSes such as FreeRTOS very
 * unhappy. Specify reasonable defaults instead.
 */
#define TCPIP_MBOX_SIZE 6

#define DEFAULT_RAW_RECVMBOX_SIZE 6

#define DEFAULT_UDP_RECVMBOX_SIZE 6

#define DEFAULT_TCP_RECVMBOX_SIZE 6

/*
   ---------------------------------
   ---------- RAW options ----------
   ---------------------------------
*/

/**
 * LWIP_RAW==1: Enable application layer to hook into the IP layer itself.
 */
#define LWIP_RAW 1

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/

/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN 0

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/

/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET 0

/**
 * Enable locking in the lwip (tcpip) thread.
 */
#ifndef LWIP_TCPIP_CORE_LOCKING
#define LWIP_TCPIP_CORE_LOCKING 1
#endif

/**
 * Enable support for TCP keepalives.
 */
#ifndef LWIP_TCP_KEEPALIVE
#define LWIP_TCP_KEEPALIVE 1
#endif

/** LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS:
 * Ensure compatibilty with platforms where LwIP is configured not to define the host/network byte-order conversion
 * functions normally provided in <arpa/inet.h> on POSIX systems.
 */
#ifndef LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS 1
#endif

#endif /* __LWIPOPTS_H__ */
