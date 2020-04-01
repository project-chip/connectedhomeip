/**
 * @file
 *
 * IPv6 static route table.
 */

/*
 * Copyright (c) 2020 Project CHIP Authors
 * Copyright (c) 2015 Nest Labs, Inc.
 */

#ifndef __LWIP_IP6_ROUTE_TABLE_H__
#define __LWIP_IP6_ROUTE_TABLE_H__

#include "lwip/opt.h"

#if LWIP_IPV6  /* don't build if not configured for use in lwipopts.h */

#include "lwip/ip.h"
#include "lwip/ip6_addr.h"
#include "lwip/def.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#if LWIP_IPV6_ROUTE_TABLE_SUPPORT

#define IP6_MAX_PREFIX_LEN                  (128)
#define IP6_PREFIX_ALLOWED_GRANULARITY      (8)
/* Prefix length cannot be greater than 128 bits and needs to be at a byte boundary */
#define ip6_prefix_valid(prefix_len)        (((prefix_len) <= IP6_MAX_PREFIX_LEN) &&                 \
                                             (((prefix_len) % IP6_PREFIX_ALLOWED_GRANULARITY) == 0))

struct ip6_prefix {
  ip6_addr_t addr;
  u8_t prefix_len; /* prefix length in bits at byte boundaries */
};

struct ip6_route_entry {
  struct ip6_prefix prefix;
  struct netif *netif;
  ip6_addr_t *gateway;
};

err_t ip6_add_route_entry(struct ip6_prefix *ip6_prefix, struct netif *netif, 
                         ip6_addr_t *gateway, s8_t *index);
void ip6_remove_route_entry(struct ip6_prefix *ip6_prefix);
s8_t ip6_find_route_entry(const ip6_addr_t *ip6_dest_addr);
struct netif *ip6_static_route(const ip6_addr_t *src, const ip6_addr_t *dest);
ip6_addr_t *ip6_get_gateway(struct netif *netif, const ip6_addr_t *dest);
struct ip6_route_entry *ip6_get_route_table(void);
#endif /* LWIP_IPV6_ROUTE_TABLE_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_IPV6 */

#endif /* __LWIP_IP6_ROUTE_TABLE_H__ */
