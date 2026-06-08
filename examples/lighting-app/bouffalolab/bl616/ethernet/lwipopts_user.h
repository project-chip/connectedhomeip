/**
 * Copyright (c) 2022 Bouffalolab team
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 ******************************************************************************
 */

#ifndef LWIP_HDR_LWIPOPTS_H__
#define LWIP_HDR_LWIPOPTS_H__

#define LWIP_NETIF_API 1

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT 4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE (32 * 1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF 20
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB 16
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB 6
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 5
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8 + 3)

/* ---------- IPv4 options ---------- */
#define LWIP_IPV4 1

/* ---------- TCP options ---------- */
#define LWIP_TCP 1
#define TCP_TTL 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ 0

/* TCP Maximum segment size. */
#define TCP_MSS (1500 - 40) /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF (16 * TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#define TCP_SND_QUEUELEN (8 * TCP_SND_BUF / TCP_MSS)

/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG TCP_SND_QUEUELEN

/* TCP receive window. */
#define TCP_WND (4 * TCP_MSS)

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
   @ note: used to allocate Tx pbufs only
   mix pbuf size is (TCP_WND / TCP_MSS)  */
#define PBUF_POOL_SIZE (2 * TCP_WND / TCP_MSS)

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE 1524

/* LWIP_SUPPORT_CUSTOM_PBUF == 1: to pass directly MAC Rx buffers to the stack
   no copy is needed */
#define LWIP_SUPPORT_CUSTOM_PBUF 1

/* ---------- ICMP options ---------- */
#define LWIP_ICMP 1

/* ---------- DHCP options ---------- */
#define LWIP_DHCP 1

/* ---------- UDP options ---------- */
#define LWIP_UDP 1
#define UDP_TTL 255

#define LWIP_NETIF_LOOPBACK 1
#define LWIP_HAVE_LOOPIF 1
#define LWIP_LOOPBACK_MAX_PBUFS 0

#define LWIP_RAW 1
#define LWIP_MULTICAST_TX_OPTIONS 1
#define SO_REUSE 1

#define LWIP_IGMP 1
#define LWIP_IPV6 1
#define LWIP_IPV6_DHCP6 1
#define LWIP_IPV6_SCOPES 0
#define LWIP_IPV6_MLD 1
#define MEMP_NUM_MLD6_GROUP 10

/* ---------- Statistics options ---------- */
#define LWIP_STATS 0

#define LWIP_TIMEVAL_PRIVATE 0 // use sys/time.h for struct timeval

#define LWIP_ERRNO_STDINCLUDE 1
#define LWIP_SOCKET_SET_ERRNO 1

#ifdef __cplusplus
extern "C" int * __errno(void);
#else
extern int * __errno(void);
#endif
#define errno (*__errno())
#define LWIP_RAND() ((u32_t) random())

/* ---------- link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Support a callback function from an interface
 * whenever the link changes (i.e., link down)
 */
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_API 1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/
/* LWIP_CHECKSUM_ON_COPY==1: Calculate checksum when copying data from application buffers to pbufs. */
#define LWIP_CHECKSUM_ON_COPY 0

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
/* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_GEN_ICMP 0
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
/* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_GEN_ICMP 1
#define CHECKSUM_CHECK_ICMP 1
#endif

/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN 1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET 1

/* ---------------- httpd options --------------- */
#define HTTPD_USE_CUSTOM_FSDATA 0

/*
   ----------------------------------------
   ---------- Lwip Debug options ----------
   ----------------------------------------
*/
#define LWIP_DEBUG LWIP_DBG_OFF
#define ETHARP_DEBUG LWIP_DBG_OFF
#define ICMP_DEBUG LWIP_DBG_OFF
#define TCPIP_DEBUG LWIP_DBG_OFF
#define TCP_DEBUG LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define SOCKET_DEBUG LWIP_DBG_OFF
#define API_LIB_DEBUG LWIP_DBG_OFF
#define HTTPD_DEBUG LWIP_DBG_OFF
#define PBUF_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF

/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/

#define osPriorityIdle 2         //< priority: idle (lowest)
#define osPriorityLow 5          //< priority: low
#define osPriorityBelowNormal 10 //< priority: below normal
#define osPriorityNormal 15      //< priority: normal (default)
#define osPriorityAboveNormal 20 //< priority: above normal
#define osPriorityHigh 25        //< priority: high
#define osPriorityRealtime 29    //< priority: realtime (highest)

#define TCPIP_THREAD_NAME "TCP/IP"
#define TCPIP_THREAD_STACKSIZE 1024
#define TCPIP_MBOX_SIZE 100
#define DEFAULT_UDP_RECVMBOX_SIZE 100
#define DEFAULT_TCP_RECVMBOX_SIZE 100
#define DEFAULT_ACCEPTMBOX_SIZE 100
#define DEFAULT_THREAD_STACKSIZE 512
#define TCPIP_THREAD_PRIO osPriorityHigh

/* no wifi ram */
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) u8_t variable_name[LWIP_MEM_ALIGN_BUFFER(size)]

#define LWIP_DNS_SERVER 0

#endif /* LWIP_HDR_LWIPOPTS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
