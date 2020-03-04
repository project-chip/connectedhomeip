/**
 * @file
 * Common IPv4 and IPv6 code
 *
 * @defgroup ip IP
 * @ingroup callbackstyle_api
 * 
 * @defgroup ip4 IPv4
 * @ingroup ip
 *
 * @defgroup ip6 IPv6
 * @ingroup ip
 * 
 * @defgroup ipaddr IP address handling
 * @ingroup infrastructure
 * 
 * @defgroup ip4addr IPv4 only
 * @ingroup ipaddr
 * 
 * @defgroup ip6addr IPv6 only
 * @ingroup ipaddr
 */

/*
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
 *
 */

#include "lwip/opt.h"

#if LWIP_IPV4 || LWIP_IPV6

#include "lwip/ip_addr.h"
#include "lwip/ip.h"

/** Global data for both IPv4 and IPv6 */
struct ip_globals ip_data;

#if LWIP_IP_DEBUG_TARGET

#define DEBUG_TARGET_MODE_ALLOW_ALL 0
#define DEBUG_TARGET_MODE_ALLOW_NONE 1
#define DEBUG_TARGET_MODE_FILTER 2

int debug_target_mode = DEBUG_TARGET_MODE_ALLOW_ALL;
int debug_target_is_ipv6;
ipX_addr_t debug_target_ip;

int lwip_set_debug_target(const char *addr)
{
  if (addr == 0) {
    debug_target_mode = DEBUG_TARGET_MODE_ALLOW_ALL;
  }
  else if (*addr == 0) {
    debug_target_mode = DEBUG_TARGET_MODE_ALLOW_NONE;
  }
  else {
    if (ipaddr_aton(addr, ipX_2_ip(&debug_target_ip))) {
      debug_target_is_ipv6 = 0;
      debug_target_mode = DEBUG_TARGET_MODE_FILTER;
    }
#if LWIP_IPV6
    else if (ip6addr_aton(addr, ipX_2_ip6(&debug_target_ip))) {
      debug_target_is_ipv6 = 1;
      debug_target_mode = DEBUG_TARGET_MODE_FILTER;
    }
#endif
    else {
      return 0;
    }
  }
  return 1;
}

int debug_target_match(int is_ipv6, ipX_addr_t *src, ipX_addr_t *dest)
{
  if (debug_target_mode == DEBUG_TARGET_MODE_ALLOW_ALL) {
    return 1;
  }
  if (debug_target_mode == DEBUG_TARGET_MODE_ALLOW_NONE) {
    return 0;
  }
  if (is_ipv6 != debug_target_is_ipv6) {
    return 0;
  }
  if (ipX_addr_isany(is_ipv6, &debug_target_ip)) {
    return 1;
  }
  if (ipX_addr_cmp(is_ipv6, &debug_target_ip, src)) {
    return 1;
  }
  if (ipX_addr_cmp(is_ipv6, &debug_target_ip, dest)) {
    return 1;
  }
  return 0;
}

#endif

#if LWIP_IPV4 && LWIP_IPV6

const ip_addr_t ip_addr_any_type = IPADDR_ANY_TYPE_INIT;

/**
 * @ingroup ipaddr
 * Convert IP address string (both versions) to numeric.
 * The version is auto-detected from the string.
 *
 * @param cp IP address string to convert
 * @param addr conversion result is stored here
 * @return 1 on success, 0 on error
 */
int
ipaddr_aton(const char *cp, ip_addr_t *addr)
{
  if (cp != NULL) {
    const char* c;
    for (c = cp; *c != 0; c++) {
      if (*c == ':') {
        /* contains a colon: IPv6 address */
        if (addr) {
          IP_SET_TYPE_VAL(*addr, IPADDR_TYPE_V6);
        }
        return ip6addr_aton(cp, ip_2_ip6(addr));
      } else if (*c == '.') {
        /* contains a dot: IPv4 address */
        break;
      }
    }
    /* call ip4addr_aton as fallback or if IPv4 was found */
    if (addr) {
      IP_SET_TYPE_VAL(*addr, IPADDR_TYPE_V4);
    }
    return ip4addr_aton(cp, ip_2_ip4(addr));
  }
  return 0;
}

/**
 * @ingroup lwip_nosys
 * If both IP versions are enabled, this function can dispatch packets to the correct one.
 * Don't call directly, pass to netif_add() and call netif->input().
 */
err_t
ip_input(struct pbuf *p, struct netif *inp)
{
  if (p != NULL) {
    if (IP_HDR_GET_VERSION(p->payload) == 6) {
      return ip6_input(p, inp);
    }
    return ip4_input(p, inp);
  }
  return ERR_VAL;
}

#endif /* LWIP_IPV4 && LWIP_IPV6 */

#endif /* LWIP_IPV4 || LWIP_IPV6 */
