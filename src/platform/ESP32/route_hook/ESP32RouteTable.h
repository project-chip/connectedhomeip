/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_err.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "ROUTE_HOOK"

/**
 * @brief Route table entry
 *
 */
typedef struct esp_route_entry_t
{
    ip6_addr_t prefix;
    uint8_t prefix_length;
    ip6_addr_t gateway;
    int8_t preference;
    uint32_t lifetime_seconds;
    struct netif * netif;
    struct esp_route_entry_t * next;
} esp_route_entry_t;

/**
 * @brief Adds an entry to the route table
 *
 * @param[in] route_entry    The route entry to be added
 *
 * @return
 *   - The pointer to the added route entry on success
 *   - NULL on failure
 *
 */
esp_route_entry_t * esp_route_table_add_route_entry(const esp_route_entry_t * route_entry);

/**
 * @brief The lwIP ip6 route hook, called by the lwIP function ip6_route when sending packets.
 *
 * @param[in] src   The source address
 * @param[in] dest  The destination address
 *
 * @return
 *   - The target interface when route found
 *   - NULL when route not found
 *
 */
struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest);

/**
 * @brief The lwIP gateway hook, called by the lwIP when deciding next hop.
 *
 * @param[in] netif     The output network interface
 * @param[in] dest      The destination address
 *
 * @return
 *   - The gateway address when route found
 *   - NULL when route not found
 *
 */
const ip6_addr_t * lwip_hook_nd6_get_gw(struct netif * netif, const ip6_addr_t * dest);

#ifdef __cplusplus
}
#endif
