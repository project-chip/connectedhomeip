/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#ifndef _LWIP_DEFAULT_HOOKS_H_
#define _LWIP_DEFAULT_HOOKS_H_
#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"

#include "lwiphooks.h"

extern struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest);
#define LWIP_HOOK_IP6_ROUTE lwip_hook_ip6_route

extern const ip6_addr_t * lwip_hook_nd6_get_gw(struct netif * netif, const ip6_addr_t * dest);
#define LWIP_HOOK_ND6_GET_GW lwip_hook_nd6_get_gw

#endif /* _LWIP_DEFAULT_HOOKS_H_ */
