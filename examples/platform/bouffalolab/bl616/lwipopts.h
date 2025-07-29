/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * Author: Simon Goldschmidt
 *
 */
#ifndef LWIP_HDR_LWIPOPTS_H__
#define LWIP_HDR_LWIPOPTS_H__

#include "arch/sys_arch.h"
#include "rtos_def.h"
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" long random(void);
extern "C" int * __errno(void);
extern "C" uint16_t fhost_ip_chksum(const void * dataptr, int len);
extern "C" sys_mutex_t lock_tcpip_core;
extern "C" int sys_is_inside_interrupt(void);
extern "C" int sys_mutex_is_locked(sys_mutex_t * mutex);
extern "C" int sys_current_is_tcpip(void);
#else
extern long random(void);
extern int * __errno(void);
extern uint16_t fhost_ip_chksum(const void * dataptr, int len);
extern sys_mutex_t lock_tcpip_core;
extern int sys_is_inside_interrupt(void);
extern int sys_mutex_is_locked(sys_mutex_t * mutex);
extern int sys_current_is_tcpip(void);
extern const int fhost_tcpip_priority;
#endif

#define LWIP_TCPIP_CORE_LOCKING 1
#define IP_DEFAULT_TTL 64

#if defined(CONFIG_HIGH_PERFORMANCE) && (CONFIG_HIGH_PERFORMANCE == 1)
#define LWIP_ASSERT_CORE_LOCKED()
#define LWIP_NOASSERT
#else
#if LWIP_TCPIP_CORE_LOCKING
#define LWIP_ASSERT_CORE_LOCKED()                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        if (lock_tcpip_core)                                                                                                       \
        {                                                                                                                          \
            LWIP_ASSERT("api must call with lwip core lock", !sys_is_inside_interrupt() && sys_mutex_is_locked(&lock_tcpip_core)); \
        }                                                                                                                          \
    } while (0)
#else
#define LWIP_ASSERT_CORE_LOCKED()                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        LWIP_ASSERT("api must call inside lwip task", !sys_is_inside_interrupt() && sys_current_is_tcpip());                       \
    } while (0)
#endif
#endif

#define LWIP_NETIF_API 1
#define LWIP_DEBUG 1
#define LWIP_STATS_DISPLAY 1
#define SOCKETS_DEBUG LWIP_DBG_OFF
#ifdef BL616_DHCP_DEBUG
#define DHCP_DEBUG LWIP_DBG_ON
#else
#define DHCP_DEBUG LWIP_DBG_OFF
#endif
#define ICMP_DEBUG LWIP_DBG_OFF
#define ETHARP_DEBUG LWIP_DBG_OFF
#define LWIP_MULTICAST_PING 1
#define LWIP_BROADCAST_PING 1

#define TCPIP_MBOX_SIZE 64
#define TCPIP_THREAD_STACKSIZE FHOST_TCPIP_STACK_SIZE
#define TCPIP_THREAD_PRIO fhost_tcpip_priority

#define DEFAULT_THREAD_STACKSIZE 1024
#define DEFAULT_THREAD_PRIO 1
#define DEFAULT_RAW_RECVMBOX_SIZE 32
#define DEFAULT_UDP_RECVMBOX_SIZE 64
#define DEFAULT_TCP_RECVMBOX_SIZE 64
#define DEFAULT_ACCEPTMBOX_SIZE 32

#define LWIP_NETIF_LOOPBACK 1
#define LWIP_HAVE_LOOPIF 1
#define LWIP_LOOPBACK_MAX_PBUFS 0

#define LWIP_CHKSUM_ALGORITHM 3
#define LWIP_CHKSUM fhost_ip_chksum
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1

#define PBUF_LINK_ENCAPSULATION_HLEN 388

#define MEMP_NUM_NETBUF 32
#define MEMP_NUM_NETCONN 16
#define MEMP_NUM_UDP_PCB 16
#define MEMP_NUM_REASSDATA LWIP_MIN((IP_REASS_MAX_PBUFS), 5)

#define MEM_ALIGNMENT 4
#define MEM_SIZE 30720
#define PBUF_POOL_BUFSIZE (1280 + 462 + 26)
#define MEMP_MEM_MALLOC 1

// #define LWIP_HOOK_FILENAME        "lwiphooks.h"
#define LWIP_HOOK_FILENAME "bl616_lwip_hooks.h"

#define LWIP_RAW 1
#define LWIP_MULTICAST_TX_OPTIONS 1

#define LWIP_TIMEVAL_PRIVATE                                                                                                       \
    0 // use sys/time.h for struct timeval
      //
// #define LWIP_PROVIDE_ERRNO            1
#ifndef PLATFORM_PRIVDE_ERRNO
#define LWIP_PROVIDE_ERRNO 1
#else
#define LWIP_ERRNO_STDINCLUDE 1
#define LWIP_SOCKET_SET_ERRNO 1
#endif

#define LWIP_DHCP 1
#define LWIP_DNS 1
#define LWIP_IGMP 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define SO_REUSE 1
#define LWIP_TCP_KEEPALIVE 1
#define LWIP_SO_RCVBUF 1

#define LWIP_IPV6 1
#define LWIP_IPV6_DHCP6 1
#define LWIP_IPV6_SCOPES 0
#define LWIP_IPV6_MLD 1
#define MEMP_NUM_MLD6_GROUP 10

#define MEMP_NUM_SYS_TIMEOUT (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8 + 3)

#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_API 1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1

#define ETHARP_SUPPORT_STATIC_ENTRIES 1

#define LWIP_SUPPORT_CUSTOM_PBUF 1
#ifdef HIGH_PERFORMANCE_PBUF
#define LWIP_NETIF_TX_SINGLE_PBUF 0
#else
#define LWIP_NETIF_TX_SINGLE_PBUF 1
#endif
#define LWIP_RAND() ((u32_t) rand())
#undef LWIP_DECLARE_MEMORY_ALIGNED
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size)                                                                           \
    u8_t variable_name[size] __attribute__((aligned(4))) __attribute__((section("SHAREDRAM")))

#if defined(CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_RAM) && CHIP_SYSTEM_CONFIG_PACKETBUFFER_LWIP_PBUF_RAM
#define PBUF_POOL_SIZE 0

#include <lwip/arch.h>
#include <lwip/mem.h>
#define LWIP_PBUF_CUSTOM_DATA mem_size_t pool;

#if defined(__cplusplus)
extern "C" const mem_size_t * memp_sizes;
extern "C" struct pbuf * pbuf_rightsize(struct pbuf * p, s16_t offset);
#else
extern const mem_size_t * memp_sizes;
extern struct pbuf * pbuf_rightsize(struct pbuf * p, s16_t offset);
#endif
#else
#define PBUF_POOL_SIZE 20
#endif

#endif /* LWIP_HDR_LWIPOPTS_H__ */
