/*
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023 NXP
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
 *      Compile-time configuration for LwIP on NXP platforms using the
 *      NXP SDK.
 *
 */

#ifndef __LWIPOPTS_COMMON_H__
#define __LWIPOPTS_COMMON_H__

#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h>
#endif

#include <stdlib.h>

#define LWIP_PROVIDE_ERRNO

#ifndef CONFIG_NETWORK_HIGH_PERF
#define CONFIG_NETWORK_HIGH_PERF 1
#endif

#if USE_RTOS

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT 1

/**
 * NO_SYS==0: Use RTOS
 */
#define NO_SYS 0
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN 1
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET 1

/**
 * LWIP_SO_RCVTIMEO==1: Enable receive timeout for sockets/netconns and
 * SO_RCVTIMEO processing.
 */
#define LWIP_SO_RCVTIMEO 1

#else
/**
 * NO_SYS==1: Bare metal lwIP
 */
#define NO_SYS 1
/**
 * LWIP_NETCONN==0: Disable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN 0
/**
 * LWIP_SOCKET==0: Disable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET 0

#endif

/* ---------- Multicast DNS options ---------- */
/* Multicast DNS module */
#ifndef LWIP_MDNS_RESPONDER
#define LWIP_MDNS_RESPONDER 1
#endif

/**
 * The maximum number of services per netif
 * recommended to be two times greater than number of matter fabrics supported
 */
#ifndef MDNS_MAX_SERVICES
#define MDNS_MAX_SERVICES 10
#endif

/* Search over multicast DNS module */
#ifndef LWIP_MDNS_SEARCH
#define LWIP_MDNS_SEARCH 0
#endif

/* Provide strncasecmp to lwIP so it does not provide it's own lwip_strnicmp implementation along the strncasecmp from toolchain */
#ifndef lwip_strnicmp
#define lwip_strnicmp strncasecmp
#endif

/* ---------- Core locking ---------- */

#ifdef __cplusplus
extern "C" {
#endif

#define LWIP_TCPIP_CORE_LOCKING 1

void sys_lock_tcpip_core(void);
#define LOCK_TCPIP_CORE() sys_lock_tcpip_core()

void sys_unlock_tcpip_core(void);
#define UNLOCK_TCPIP_CORE() sys_unlock_tcpip_core()

// void sys_check_core_locking(void);
// #define LWIP_ASSERT_CORE_LOCKED() sys_check_core_locking()

// void sys_mark_tcpip_thread(void);
// #define LWIP_MARK_TCPIP_THREAD() sys_mark_tcpip_thread()

#ifdef __cplusplus
}
#endif

#define LWIP_NETIF_HOSTNAME 1

/* ---------- Memory options ---------- */
/**
 * MEM_ALIGNMENT: should be set to the alignment of the CPU
 *    4 byte alignment -> #define MEM_ALIGNMENT 4
 *    2 byte alignment -> #define MEM_ALIGNMENT 2
 */
#ifndef MEM_ALIGNMENT
#define MEM_ALIGNMENT 4
#endif

/**
 * MEM_SIZE: the size of the heap memory. If the application will send
 * a lot of data that needs to be copied, this should be set high.
 */
#ifndef MEM_SIZE
#define MEM_SIZE (22 * 1024)
#endif

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#ifndef MEMP_NUM_PBUF
#if CONFIG_NETWORK_HIGH_PERF
#define MEMP_NUM_PBUF 30
#else
#define MEMP_NUM_PBUF 15
#endif
#endif

/**
 * MEMP_NUM_TCPIP_MSG_INPKT: the number of struct tcpip_msg, which are used
 * for incoming packets.
 * (only needed if you use tcpip.c)
 */
#ifndef MEMP_NUM_TCPIP_MSG_INPKT
#if CONFIG_NETWORK_HIGH_PERF
#define MEMP_NUM_TCPIP_MSG_INPKT 32
#else
#define MEMP_NUM_TCPIP_MSG_INPKT 16
#endif
#endif

/** MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#ifndef MEMP_NUM_TCPIP_MSG_API
#if CONFIG_NETWORK_HIGH_PERF
#define MEMP_NUM_TCPIP_MSG_API 64
#else
#define MEMP_NUM_TCPIP_MSG_API 16
#endif
#endif

/**
 * MEMP_NUM_NETBUF: the number of struct netbufs.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#ifndef MEMP_NUM_NETBUF
#if CONFIG_NETWORK_HIGH_PERF
#define MEMP_NUM_NETBUF 32
#else
#define MEMP_NUM_NETBUF 16
#endif
#endif

/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#ifndef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB (6 + (LWIP_MDNS_RESPONDER))
#endif
/* MEMP_NUM_TCP_PCB: the number of simultaneously active TCP
   connections. */
#ifndef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB 10
#endif
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#ifndef MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_PCB_LISTEN 6
#endif
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#ifndef MEMP_NUM_TCP_SEG
#if CONFIG_NETWORK_HIGH_PERF
#define MEMP_NUM_TCP_SEG 48
#else
#define MEMP_NUM_TCP_SEG 22
#endif
#endif
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#ifndef MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_SYS_TIMEOUT (10 + (7 * (LWIP_MDNS_RESPONDER)))
#endif

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE 40
#endif

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
/* Default value is defined in lwip\src\include\lwip\opt.h as
 * LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)*/

/* ---------- TCP options ---------- */
#ifndef LWIP_TCP
#define LWIP_TCP 1
#endif

#ifndef TCP_TTL
#define TCP_TTL 255
#endif

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#ifndef TCP_QUEUE_OOSEQ
#define TCP_QUEUE_OOSEQ 0
#endif

/* TCP Maximum segment size. */
#ifndef TCP_MSS
#define TCP_MSS (1500 - 40) /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
#endif

/* TCP sender buffer space (bytes). */
#ifndef TCP_SND_BUF
#if CONFIG_NETWORK_HIGH_PERF
#define TCP_SND_BUF (12 * TCP_MSS)
#else
#define TCP_SND_BUF (6 * TCP_MSS)
#endif
#endif

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#ifndef TCP_SND_QUEUELEN
#define TCP_SND_QUEUELEN (3 * TCP_SND_BUF) / TCP_MSS
#endif

/* TCP receive window. */
#ifndef TCP_WND
#if CONFIG_NETWORK_HIGH_PERF
#define TCP_WND (15 * TCP_MSS)
#else
#define TCP_WND (2 * TCP_MSS)
#endif
#endif

/* Enable backlog*/
#ifndef TCP_LISTEN_BACKLOG
#define TCP_LISTEN_BACKLOG 1
#endif

/* ---------- Network Interfaces options ---------- */
/* Support netif api (in netifapi.c). */
#ifndef LWIP_NETIF_API
#define LWIP_NETIF_API 1
#endif

/* Support an extended callback function
 * for several netif related event that supports multiple subscribers. */
#ifndef LWIP_NETIF_EXT_STATUS_CALLBACK
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#endif

/* Number of clients that may store data in client_data member array of struct netif (max. 256). */
#ifndef LWIP_NUM_NETIF_CLIENT_DATA
#define LWIP_NUM_NETIF_CLIENT_DATA (LWIP_MDNS_RESPONDER)
#endif

/* ---------- ICMP options ---------- */
#ifndef LWIP_ICMP
#define LWIP_ICMP 1
#endif

/* ---------- DHCP options ---------- */
/* Enable DHCP module. */
#ifndef LWIP_DHCP
#define LWIP_DHCP 1
#endif

/* ---------- IGMP options ---------- */
/**
 * LWIP_IGMP==1: Turn on IGMP module.
 */
#define LWIP_IGMP 1

/* ---------- UDP options ---------- */
#ifndef LWIP_UDP
#define LWIP_UDP 1
#endif
#ifndef UDP_TTL
#define UDP_TTL 255
#endif

/* ---------- Statistics options ---------- */
#ifndef LWIP_STATS
#define LWIP_STATS 0
#endif
#ifndef LWIP_PROVIDE_ERRNO
#define LWIP_PROVIDE_ERRNO 1
#endif

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/*
Some MCU allow computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
// #define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
/* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
#define CHECKSUM_GEN_IP 0
/* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP 0
/* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP 0
/* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
#define CHECKSUM_CHECK_IP 0
/* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP 0
/* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP 0
#else
/* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
#define CHECKSUM_GEN_IP 1
/* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP 1
/* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP 1
/* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
#define CHECKSUM_CHECK_IP 1
/* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP 1
/* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP 1
#endif

/**
 * DEFAULT_THREAD_STACKSIZE: The stack size used by any other lwIP thread.
 * The stack size value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef DEFAULT_THREAD_STACKSIZE
#define DEFAULT_THREAD_STACKSIZE 3000
#endif

/**
 * DEFAULT_THREAD_PRIO: The priority assigned to any other lwIP thread.
 * The priority value itself is platform-dependent, but is passed to
 * sys_thread_new() when the thread is created.
 */
#ifndef DEFAULT_THREAD_PRIO
#define DEFAULT_THREAD_PRIO 8
#endif

#define LWIP_TIMEVAL_PRIVATE (0)
#define LWIP_COMPAT_MUTEX (0)

#define LWIP_SOCKET_SET_ERRNO 0
#define IP_REASS_MAX_PBUFS 0
#define IP_REASSEMBLY 0
#define MEMP_NUM_REASSDATA 0

#define SO_REUSE (0)
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS (0)

#define TCP_QUEUE_OOSEQ 0
#define ARP_QUEUEING (0)

// For Thread-only platforms this is unnecessary but Border router config needs to redefine to larger value
#define LWIP_RAW 1
#ifndef MEMP_NUM_RAW_PCB
#define MEMP_NUM_RAW_PCB (1)
#endif

// TODO: verify count

#define LWIP_HAVE_LOOPIF (0)

// TODO: not sure why this is disabled
#define LWIP_NETIF_LOOPBACK (0)

#ifndef MEMP_NUM_NETCONN
#define MEMP_NUM_NETCONN (0)
#endif

#ifndef LWIP_IPV4
#define LWIP_IPV4 0
#endif
// LWIP IPV6 is mandatory to build matter application
#define LWIP_IPV6 1
#define LWIP_ARP (1)
#ifndef LWIP_DNS
#define LWIP_DNS (1)
#endif

#if LWIP_IPV6
// #define LWIP_IPV6_REASS (0)
// #define LWIP_IPV6_DHCP6 0
// #define LWIP_IPV6_AUTOCONFIG (1)
// #define LWIP_IPV6_ROUTER_SUPPORT 0
// #define LWIP_ND6_LISTEN_RA 0

// #define LWIP_ND6_NUM_NEIGHBORS (0)
// #define LWIP_ND6_NUM_DESTINATIONS (0)
// #define LWIP_ND6_NUM_PREFIXES (0)
// #define LWIP_ND6_NUM_ROUTERS (0)
// #define LWIP_ND6_MAX_MULTICAST_SOLICIT (0)
// #define LWIP_ND6_MAX_UNICAST_SOLICIT (0)
// #define LWIP_ND6_MAX_NEIGHBOR_ADVERTISEMENT (0)
// #define LWIP_ND6_TCP_REACHABILITY_HINTS (0)
#endif /* LWIP_IPV6 */

#define MEMP_SEPARATE_POOLS (0)
#define LWIP_PBUF_FROM_CUSTOM_POOLS (0)
#define MEMP_USE_CUSTOM_POOLS (0)

#define MEM_LIBC_MALLOC 1
#define mem_clib_malloc pvPortMalloc
#define mem_clib_free vPortFree

#define mem_clib_calloc pvPortCalloc

#define PBUF_CUSTOM_POOL_IDX_START (MEMP_NUM_PBUF)
#define PBUF_CUSTOM_POOL_IDX_END (MEMP_NUM_PBUF)

#define ETH_PAD_SIZE (0)
#define SUB_ETHERNET_HEADER_SPACE (0)
#define PBUF_LINK_HLEN (14)

#if LWIP_IPV6
#ifndef LWIP_IPV6_NUM_ADDRESSES
#define LWIP_IPV6_NUM_ADDRESSES 5
#endif

#define LWIP_IPV6_MLD 1
#define LWIP_ND6_QUEUEING 0

#ifndef LWIP_IPV6_SCOPES
#define LWIP_IPV6_SCOPES 0
#endif

#endif /* LWIP_IPV6 */

#define LWIP_MULTICAST_PING 0

#define LWIP_DNS_FOUND_CALLBACK_TYPE dns_found_callback

// TODO: make LWIP_DEBUG conditional on build type

#define MEMP_OVERFLOW_CHECK (0)
#define MEMP_SANITY_CHECK (0)
#define MDNS_DEBUG (LWIP_DBG_OFF)
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

/*
   ------------------------------------
   ---------- Debugging options ----------
   ------------------------------------
*/

#ifdef LWIP_DEBUG
#define U8_F "c"
#define S8_F "c"
#define X8_F "02x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
#define SZT_F "u"
#endif

#ifndef TCPIP_MBOX_SIZE
#if CONFIG_NETWORK_HIGH_PERF
#define TCPIP_MBOX_SIZE 64
#else
#define TCPIP_MBOX_SIZE 32
#endif
#endif

#define TCPIP_THREAD_STACKSIZE 1024
#define TCPIP_THREAD_PRIO 8

/**
 * DEFAULT_RAW_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_RAW. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_RAW_RECVMBOX_SIZE 12

/**
 * DEFAULT_UDP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_UDP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_UDP_RECVMBOX_SIZE 12

/**
 * DEFAULT_TCP_RECVMBOX_SIZE: The mailbox size for the incoming packets on a
 * NETCONN_TCP. The queue size value itself is platform-dependent, but is passed
 * to sys_mbox_new() when the recvmbox is created.
 */
#define DEFAULT_TCP_RECVMBOX_SIZE 32

/**
 * DEFAULT_ACCEPTMBOX_SIZE: The mailbox size for the incoming connections.
 * The queue size value itself is platform-dependent, but is passed to
 * sys_mbox_new() when the acceptmbox is created.
 */
#define DEFAULT_ACCEPTMBOX_SIZE 12

#if (LWIP_DNS || LWIP_IGMP || LWIP_IPV6) && !defined(LWIP_RAND)
/* When using IGMP or IPv6, LWIP_RAND() needs to be defined to a random-function returning an u32_t random value*/
#include "lwip/arch.h"
u32_t lwip_rand(void);
#define LWIP_RAND() lwip_rand()
#endif

/**
 * DAC macro (initially defined in MIMXRT1176_cm7_features.h) must be undefined
 * in the use case of RT1170 platform.
 * This is due to DAC name being used by Matter, which causes a conflict
 * resulting in a compilation failure.
 */
#ifdef MIMXRT1176_cm7_SERIES
#undef DAC
#endif

#endif /* __LWIPOPTS_COMMON_H__ */
