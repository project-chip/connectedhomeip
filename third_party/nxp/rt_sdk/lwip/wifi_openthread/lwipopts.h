/*
 *
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *      Compile-time configuration for LwIP
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define LWIP_IPV4 1
#define MEMP_NUM_NETCONN (4)
#define NETIF_MAX_HWADDR_LEN 8U

// BR specific defines
#define LWIP_IPV6_SCOPES 1
#define MAX_SOCKETS_UDP 10
#define MEMP_NUM_UDP_PCB (MAX_SOCKETS_UDP + 2)
#define LWIP_IPV6_NUM_ADDRESSES 8
#define LWIP_IPV6_FORWARD 1
#define MEMP_NUM_RAW_PCB (2)
// Note: According to Thread Conformance v1.2.0, a Thread Border Router MUST be able to hold a Multicast Listeners Table
//  in memory with at least seventy five (75) entries.
#define MEMP_NUM_MLD6_GROUP 10 + 75

// DHCPv6 Prefix Delegation
#define LWIP_IPV6_DHCP6 1
#define LWIP_IPV6_DHCP6_STATEFUL 1
#define LWIP_IPV6_DHCP6_PD 1

// Header file with lwIP hooks
#define LWIP_HOOK_FILENAME "lwip_hooks.h"

// Hook for multicast forwarding and other filtering
#define LWIP_HOOK_IP6_CANFORWARD lwipCanForwardHook

// Hook for filtering of input traffic
#define LWIP_HOOK_IP6_INPUT lwipInputHook

#include "lwipopts_common.h"

#endif /* __LWIPOPTS_H__ */
