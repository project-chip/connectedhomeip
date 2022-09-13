/*
 * Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "FreeRTOSConfig.h"

#define LWIP_NETIF_TX_ZEROCP_ENABLE 1
#define LWIP_TCPIP_CORE_LOCKING 1
#define LWIP_NETIF_TX_SINGLE_PBUF 1
#define TCPIP_MBOX_SIZE 16
#define DEFAULT_RAW_RECVMBOX_SIZE 16
#define DEFAULT_UDP_RECVMBOX_SIZE 16
#define DEFAULT_TCP_RECVMBOX_SIZE 16
#define DEFAULT_ACCEPTMBOX_SIZE 16
#define LWIP_STATS_DISPLAY 0

// fix http IOT issue
#define LWIP_WND_SCALE 1
#define TCP_RCV_SCALE 1
#define MEMP_NUM_NETDB 4
// fix reuse address issue
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
// for ip display
#define LWIP_NETIF_STATUS_CALLBACK 1

#if defined(MTK_MDNS_ENABLE)
#define LWIP_IPV6 1
#define IP_REASS_MAXAGE 60 // 3
#define LWIP_IPV6_FRAG 1
#define LWIP_MULTICAST_PING 1
#define LWIP_HAVE_LOOPIF 1
#define LWIP_IGMP 1
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_DHCP_AUTOIP_COOP 1
#define LWIP_DHCP_AUTOIP_COOP_TRIES 3

#define LWIP_NETBUF_RECVINFO 1
#define LWIP_DNS 1
#define LWIP_SOCKET 1
#define LWIP_COMPAT_SOCKETS 1
#define LWIP_POSIX_SOCKETS_IO_NAMES 1
#define MEMP_NUM_NETBUF 16
#define MEMP_NUM_MLD6_GROUP 10

#define MEMP_NUM_ND6_QUEUE 30
#define LWIP_ND6_RETRANS_TIMER 200
#endif

#define ETH_PAD_SIZE 0

#define NO_SYS 0
#define LWIP_CALLBACK_API 1

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
#define CHECKSUM_GEN_UDP 0
#define CHECKSUM_GEN_TCP 0
#define CHECKSUM_CHECK_UDP 0
#define CHECKSUM_CHECK_TCP 0

/*
   ------------------------------------
   ---------- Memory options ----------
   ------------------------------------
*/
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT 8

/**
 * MEMP_MEM_MALLOC==1: Use mem_malloc/mem_free instead of the lwip pool allocator.
 * Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
 * speed and usage from interrupts!
 */
#define MEMP_MEM_MALLOC 1

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#if defined(MTK_WIFI_TGN_VERIFY_ENABLE) && !defined(MTK_HOMEKIT_ENABLE)
#define MEM_SIZE (100 * 1024)
#else
#if defined(MTK_WIFI_ROUTER_ENABLE)
#define MEM_SIZE (78 * 1024)
#else
#define MEM_SIZE (48 * 1024)
#endif
#endif

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */

#define SYS_LIGHTWEIGHT_PROT 1

/*
   ------------------------------------------------
   ---------- Internal Memory Pool Sizes ----------
   ------------------------------------------------
*/

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF 8
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB 8
/* MEMP_NUM_TCP_PCB: the number of simultaneously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB 32 // 8 original
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 32 // 16 original
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG 255

/**
 * MEMP_NUM_ARP_QUEUE: the number of simulateously queued outgoing
 * packets (pbufs) that are waiting for an ARP request (to resolve
 * their destination address) to finish.
 * (requires the ARP_QUEUEING option)
 */
#define MEMP_NUM_ARP_QUEUE 8

/**
 * MEMP_NUM_NETCONN: the number of struct netconns.
 * (only needed if you use the sequential API, like api_lib.c)
 */
#define MEMP_NUM_NETCONN 32

/*
   ----------------------------------
   ---------- Pbuf options ----------
   ----------------------------------
*/
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE 10

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
//#define PBUF_POOL_BUFSIZE       1536
/* packet of MT7687 IOT has extra TXD header and packet offset */
#define PBUF_POOL_BUFSIZE 1664

/**
 * PBUF_LINK_HLEN: the number of bytes that should be allocated for a
 * link level header. The default is 14, the standard value for
 * Ethernet.
 */

#define RESERVED_HLEN 0 // depend on CFG_CONNSYS_TXD_PAD_SIZE
#ifdef MTK_HIF_GDMA_ENABLE
#define PBUF_LINK_ENCAPSULATION_HLEN 0 // for WiFi headroom (TX zero copy)
#else
#define PBUF_LINK_ENCAPSULATION_HLEN (64 + 4)
#endif
#define PBUF_LINK_HLEN (RESERVED_HLEN + 14 + ETH_PAD_SIZE)
/*
   ---------------------------------
   ---------- TCP options ----------
   ---------------------------------
*/
#define LWIP_TCP 1
#define TCP_TTL 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ 1

/* TCP Maximum segment size. */
#define TCP_MSS 1476

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF (24 * 1024) //(12 * 1024)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

/* TCP receive window. */
#define TCP_WND (24 * 1024)

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX 12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX 4

#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS 1

/*
   ---------------------------------
   ---------- ARP options ----------
   ---------------------------------
*/
#define LWIP_ARP 1
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD 1

/* Define IP_NAPT to 1 If you wish to have the ability to forward
   IP packets across network interfaces using Network Address Port
   Translation (NAPT).*/
#if defined(MTK_WIFI_ROUTER_ENABLE)
#define IP_NAPT 1
#endif

/* If defined to 1, IP options are allowed (but not parsed). If
   defined to 0, all packets with IP options are dropped. */
#define IP_OPTIONS 1

#define IP_REASSEMBLY 1

/**
 * IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.
 * Since the received pbufs are enqueued, be sure to configure
 * PBUF_POOL_SIZE > IP_REASS_MAX_PBUFS so that the stack is still able to receive
 * packets even if the maximum amount of fragments is enqueued for reassembly!
 */
#if defined(MTK_WIFI_TGN_VERIFY_ENABLE)
#define IP_REASS_MAX_PBUFS 25
#else
#define IP_REASS_MAX_PBUFS 12
#endif
/* ---------- ICMP options ---------- */
#define ICMP_TTL 255

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP 1
#define LWIP_DHCP_CHECK_LINK_UP 1

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK 1

/* ---------- UDP options ---------- */
#define LWIP_UDP 1
#define UDP_TTL 255

/* ---------- Statistics options ---------- */
#if defined(MTK_LWIP_STATISTICS_ENABLE)
#define LWIP_STATS 1
#define LWIP_STATS_DISPLAY 1
#endif
/*
 * Should use the default value defined in tcp_impl.h
 */
/*
   ------------------------------------
   ---------- AUTOIP options ----------
   ------------------------------------
*/
#if defined(MTK_HOMEKIT_ENABLE) || defined(MTK_MDNS_ENABLE)
#define LWIP_AUTOIP 1
#else
#define LWIP_AUTOIP 0
#endif

/*
   ----------------------------------
   ---------- DNS options -----------
   ----------------------------------
*/
#define LWIP_DNS 1

/*
   ---------------------------------
   ---------- PPP options ----------
   ---------------------------------
*/
#define PPP_SUPPORT 0

/*
   ------------------------------------------------
   ---------- Network Interfaces options ----------
   ------------------------------------------------
*/

/**
 * LWIP_NETIF_LOOPBACK==1: Support sending packets with a destination IP
 * address equal to the netif IP address, looping them back up the stack.
 */
#define LWIP_NETIF_LOOPBACK 1

/**
 * LWIP_LOOPBACK_MAX_PBUFS: Maximum number of pbufs on queue for loopback
 * sending for each netif (0 = disabled)
 */
#define LWIP_LOOPBACK_MAX_PBUFS 12

/*
   ------------------------------------
   ---------- LOOPIF options ----------
   ------------------------------------
*/
/**
 * LWIP_HAVE_LOOPIF==1: Support loop interface (127.0.0.1) and loopif.c
 */
#define LWIP_HAVE_LOOPIF 1

/*
 * Should use the default value defined in opt.h
 */

/**
 * MTK_LWIP_DYNAMIC_DEBUG_ENABLE: Support LwIP dynamic debug
 * No dependency
 */
#if defined(MTK_LWIP_DYNAMIC_DEBUG_ENABLE)

struct lwip_debug_flags
{
    char * debug_flag_name;
    uint32_t debug_flag;
};

extern struct lwip_debug_flags lwip_debug_flags[];

#define LWIP_DEBUG LWIP_DBG_ON
#define LWIP_DEBUG_IDX(idx) ((idx) >> 8)

#undef ETHARP_DEBUG
#undef NETIF_DEBUG
#undef PBUF_DEBUG
#undef API_LIB_DEBUG
#undef API_MSG_DEBUG
#undef SOCKETS_DEBUG
#undef ICMP_DEBUG
#undef IGMP_DEBUG
#undef INET_DEBUG
#undef IP_DEBUG
#undef IP_REASS_DEBUG
#undef RAW_DEBUG
#undef MEM_DEBUG
#undef MEMP_DEBUG
#undef SYS_DEBUG
#undef TIMERS_DEBUG
#undef TCP_DEBUG
#undef TCP_INPUT_DEBUG
#undef TCP_FR_DEBUG
#undef TCP_RTO_DEBUG
#undef TCP_CWND_DEBUG
#undef TCP_WND_DEBUG
#undef TCP_OUTPUT_DEBUG
#undef TCP_RST_DEBUG
#undef TCP_QLEN_DEBUG
#undef UDP_DEBUG
#undef TCPIP_DEBUG
#undef PPP_DEBUG
#undef SLIP_DEBUG
#undef DHCP_DEBUG
#undef AUTOIP_DEBUG
#undef SNMP_MSG_DEBUG
#undef SNMP_MIB_DEBUG
#undef DNS_DEBUG

#define ETHARP_DEBUG 0x0000U
#define NETIF_DEBUG 0x0100U
#define PBUF_DEBUG 0x0200U
#define API_LIB_DEBUG 0x0300U
#define API_MSG_DEBUG 0x0400U
#define SOCKETS_DEBUG 0x0500U
#define ICMP_DEBUG 0x0600U
#define IGMP_DEBUG 0x0700U
#define INET_DEBUG 0x0800U
#define IP_DEBUG 0x0900U
#define IP_REASS_DEBUG 0x0a00U
#define RAW_DEBUG 0x0b00U
#define MEM_DEBUG 0x0c00U
#define MEMP_DEBUG 0x0d00U
#define SYS_DEBUG 0x0e00U
#define TIMERS_DEBUG 0x0f00U
#define TCP_DEBUG 0x1000U
#define TCP_INPUT_DEBUG 0x1100U
#define TCP_FR_DEBUG 0x1200U
#define TCP_RTO_DEBUG 0x1300U
#define TCP_CWND_DEBUG 0x1400U
#define TCP_WND_DEBUG 0x1500U
#define TCP_OUTPUT_DEBUG 0x1600U
#define TCP_RST_DEBUG 0x1700U
#define TCP_QLEN_DEBUG 0x1800U
#define UDP_DEBUG 0x1900U
#define TCPIP_DEBUG 0x1a00U
#define PPP_DEBUG 0x1b00U
#define SLIP_DEBUG 0x1c00U
#define DHCP_DEBUG 0x1d00U
#define AUTOIP_DEBUG 0x1e00U
#define SNMP_MSG_DEBUG 0x1f00U
#define SNMP_MIB_DEBUG 0x2000U
#define DNS_DEBUG 0x2100U
#endif

#endif /* __LWIPOPTS_H__ */
