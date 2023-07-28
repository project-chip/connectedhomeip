#ifndef _LWIP_DEFAULT_HOOKS_H_
#define _LWIP_DEFAULT_HOOKS_H_
#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"

#ifdef CONFIG_LWIP_HOOK_IP6_ROUTE_DEFAULT
extern struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest);
#define LWIP_HOOK_IP6_ROUTE lwip_hook_ip6_route
#endif

#ifdef CONFIG_LWIP_HOOK_ND6_GET_GW_DEFAULT
extern const ip6_addr_t * lwip_hook_nd6_get_gw(struct netif * netif, const ip6_addr_t * dest);
#define LWIP_HOOK_ND6_GET_GW lwip_hook_nd6_get_gw
#endif

#endif /* _LWIP_DEFAULT_HOOKS_H_ */
