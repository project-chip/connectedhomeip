/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Compile-time configuration for LwIP on CYW30739 platform.
 */
#pragma once

/* Make the lwip/errno.h to include the errno.h from the toolchain. */
#define LWIP_ERRNO_INCLUDE <../include/errno.h>

#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h>
#endif

/* NO SYS */
#define NO_SYS 1
#define LWIP_TIMERS 0

/* Core locking */
#define SYS_LIGHTWEIGHT_PROT 0
#define LOCK_TCPIP_CORE()
#define UNLOCK_TCPIP_CORE()

/* Memory options */
#define MEM_ALIGNMENT 4

/* Internal Memory Pool Sizes */
#define MEMP_SEPARATE_POOLS 1

/* ARP options */
#define LWIP_ARP 0

/* IP options */
#define LWIP_IPV4 0
#define IP_REASSEMBLY 0
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS 1

/* ICMP options */

/* RAW options */

/* DHCP options */

/* AUTOIP options */

/* SNMP MIB2 support */

/* Multicast options */

/* IGMP options */

/* DNS options */

/* UDP options */

/* TCP options */
#define LWIP_TCP 0

/* Pbuf options */
#define PBUF_POOL_BUFSIZE 1356

/* Network Interfaces options */
#define PBUF_POOL_SIZE 10

/* LOOPIF options */

/* Thread options */

/* Sequential layer options */
#define LWIP_NETCONN 0

/* Socket options */
#define LWIP_SOCKET 0
#define LWIP_SOCKET_SET_ERRNO 0

/* Statistics options */
#define LWIP_STATS 0

/* Checksum options */

/* IPv6 options */
#define LWIP_IPV6 1
#define LWIP_IPV6_REASS 0
#define LWIP_IPV6_ND 0
#define LWIP_ND6_TCP_REACHABILITY_HINTS 0
#define LWIP_ND6_LISTEN_RA 0
#define LWIP_IPV6_ROUTER_SUPPORT 0

/* Hook options */

/* Debugging options */
#ifndef LWIP_DEBUG
#define LWIP_DEBUG 0
#endif

#define INET_DEBUG LWIP_DBG_OFF
#define MEMP_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define IP6_DEBUG LWIP_DBG_OFF

/* Performance tracking options */
