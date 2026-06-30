#include <stdbool.h>
#include <string.h>

#include "mtk_route_table.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#define MAX_RIO_ROUTE 20
#define MAX_RIO_TIMEOUT UINT32_MAX / (1000 * 4) // lwIP defined reasonable timeout value

static mtk_route_entry_t s_route_entries[MAX_RIO_ROUTE];

static mtk_route_entry_t * find_route_entry(const mtk_route_entry_t * route_entry)
{
    for (size_t i = 0; i < LWIP_ARRAYSIZE(s_route_entries); i++)
    {
        if (s_route_entries[i].netif == NULL)
        {
            continue;
        }
        if (s_route_entries[i].netif == route_entry->netif && s_route_entries[i].prefix_length == route_entry->prefix_length &&
            memcmp(s_route_entries[i].gateway.addr, route_entry->gateway.addr, sizeof(route_entry->gateway.addr)) == 0 &&
            memcmp(s_route_entries[i].prefix.addr, route_entry->prefix.addr, route_entry->prefix_length / 8) == 0)
        {
            return &s_route_entries[i];
        }
    }
    return NULL;
}

static mtk_route_entry_t * find_empty_route_entry(void)
{
    for (size_t i = 0; i < LWIP_ARRAYSIZE(s_route_entries); i++)
    {
        if (s_route_entries[i].netif == NULL)
        {
            return &s_route_entries[i];
        }
    }
    return NULL;
}

static void route_timeout_handler(void * arg)
{
    mtk_route_entry_t * route = (mtk_route_entry_t *) arg;

    mtk_route_table_remove_route_entry(route);
}

mtk_route_entry_t * mtk_route_table_add_route_entry(const mtk_route_entry_t * route_entry)
{
    if (route_entry == NULL || (route_entry->lifetime_seconds > MAX_RIO_TIMEOUT && route_entry->lifetime_seconds != UINT32_MAX))
    {
        return NULL;
    }

    mtk_route_entry_t * entry = find_route_entry(route_entry);

    if (entry == NULL)
    {
        entry = find_empty_route_entry();
        if (entry == NULL)
        {
            return NULL;
        }
        entry->netif   = route_entry->netif;
        entry->gateway = route_entry->gateway;
        ip6_addr_assign_zone(&entry->gateway, IP6_UNICAST, entry->netif);
        entry->prefix        = route_entry->prefix;
        entry->prefix_length = route_entry->prefix_length;
    }
    else
    {
        sys_untimeout(route_timeout_handler, entry);
    }
    entry->preference       = route_entry->preference;
    entry->lifetime_seconds = route_entry->lifetime_seconds;
    if (entry->lifetime_seconds != UINT32_MAX)
    {
        sys_timeout(entry->lifetime_seconds * 1000, route_timeout_handler, entry);
    }
    return entry;
}

int8_t mtk_route_table_remove_route_entry(mtk_route_entry_t * route_entry)
{
    if (route_entry < &s_route_entries[0] || route_entry >= &s_route_entries[LWIP_ARRAYSIZE(s_route_entries)])
    {
        return -1;
    }
    sys_untimeout(route_timeout_handler, route_entry);
    route_entry->netif = NULL;
    return 0;
}

static inline bool is_better_route(const mtk_route_entry_t * lhs, const mtk_route_entry_t * rhs)
{
    if (rhs == NULL)
    {
        return true;
    }
    if (lhs == NULL)
    {
        return false;
    }
    return (lhs->prefix_length > rhs->prefix_length) ||
        (lhs->prefix_length == rhs->prefix_length && lhs->preference > rhs->preference);
}

static inline bool route_match(const mtk_route_entry_t * route, const ip6_addr_t * dest)
{
    uint8_t bytes = route->prefix_length / 8;
    uint8_t bits  = route->prefix_length % 8;
    if (memcmp(dest->addr, route->prefix.addr, bytes) != 0)
    {
        return false;
    }
    if (bits > 0)
    {
        uint8_t mask      = (uint8_t)(0xFF << (8 - bits));
        const uint8_t * d = (const uint8_t *) dest->addr;
        const uint8_t * p = (const uint8_t *) route->prefix.addr;
        if ((d[bytes] & mask) != (p[bytes] & mask))
        {
            return false;
        }
    }
    return true;
}

struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest)
{
    mtk_route_entry_t * route = NULL;

    for (size_t i = 0; i < LWIP_ARRAYSIZE(s_route_entries); i++)
    {
        if (s_route_entries[i].netif == NULL)
        {
            continue;
        }
        if (route_match(&s_route_entries[i], dest) && is_better_route(&s_route_entries[i], route))
        {
            route = &s_route_entries[i];
        }
    }

    if (route)
    {
        return route->netif;
    }
    else
    {
        return NULL;
    }
}

const ip6_addr_t * lwip_hook_nd6_get_gw(struct netif * netif, const ip6_addr_t * dest)
{
    mtk_route_entry_t * route = NULL;

    for (size_t i = 0; i < LWIP_ARRAYSIZE(s_route_entries); i++)
    {
        if (s_route_entries[i].netif == NULL)
        {
            continue;
        }
        if (s_route_entries[i].netif == netif && route_match(&s_route_entries[i], dest) &&
            is_better_route(&s_route_entries[i], route))
        {
            route = &s_route_entries[i];
        }
    }

    if (route)
    {
        return &route->gateway;
    }
    else
    {
        return NULL;
    }
}
