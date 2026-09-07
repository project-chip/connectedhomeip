#include <stdbool.h>
#include <string.h>

#include "bflb_route_table.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#define MAX_RIO_ROUTE 20
#define MAX_RIO_TIMEOUT UINT32_MAX / (1000 * 4) // lwIP defined reasonable timeout value

typedef struct route_node
{
    bflb_route_entry_t entry;
    struct route_node * next;
} route_node_t;

static route_node_t * s_route_list = NULL;
static size_t s_route_count        = 0;

static route_node_t * find_route_node(const bflb_route_entry_t * route_entry)
{
    for (route_node_t * node = s_route_list; node != NULL; node = node->next)
    {
        if (node->entry.netif == route_entry->netif && node->entry.prefix_length == route_entry->prefix_length &&
            memcmp(node->entry.gateway.addr, route_entry->gateway.addr, sizeof(route_entry->gateway.addr)) == 0 &&
            memcmp(node->entry.prefix.addr, route_entry->prefix.addr, route_entry->prefix_length / 8) == 0)
        {
            return node;
        }
    }
    return NULL;
}

static void route_timeout_handler(void * arg)
{
    bflb_route_entry_t * route = (bflb_route_entry_t *) arg;

    bflb_route_table_remove_route_entry(route);
}

bflb_route_entry_t * bflb_route_table_add_route_entry(const bflb_route_entry_t * route_entry)
{
    if (route_entry == NULL || (route_entry->lifetime_seconds > MAX_RIO_TIMEOUT && route_entry->lifetime_seconds != UINT32_MAX))
    {
        return NULL;
    }

    route_node_t * node = find_route_node(route_entry);

    if (node == NULL)
    {
        if (s_route_count >= MAX_RIO_ROUTE)
        {
            return NULL;
        }
        node = (route_node_t *) malloc(sizeof(route_node_t));
        if (node == NULL)
        {
            return NULL;
        }
        node->entry.netif   = route_entry->netif;
        node->entry.gateway = route_entry->gateway;
        ip6_addr_assign_zone(&node->entry.gateway, IP6_UNICAST, node->entry.netif);
        node->entry.prefix        = route_entry->prefix;
        node->entry.prefix_length = route_entry->prefix_length;
        node->next                = s_route_list;
        s_route_list              = node;
        s_route_count++;
    }
    else
    {
        sys_untimeout(route_timeout_handler, &node->entry);
    }
    node->entry.preference       = route_entry->preference;
    node->entry.lifetime_seconds = route_entry->lifetime_seconds;
    if (node->entry.lifetime_seconds != UINT32_MAX)
    {
        sys_timeout(node->entry.lifetime_seconds * 1000, route_timeout_handler, &node->entry);
    }
    return &node->entry;
}

int8_t bflb_route_table_remove_route_entry(bflb_route_entry_t * route_entry)
{
    route_node_t * prev = NULL;
    route_node_t * node = s_route_list;

    while (node != NULL)
    {
        if (&node->entry == route_entry)
        {
            if (prev == NULL)
            {
                s_route_list = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            free(node);
            s_route_count--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return -1;
}

static inline bool is_better_route(const bflb_route_entry_t * lhs, const bflb_route_entry_t * rhs)
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

static inline bool route_match(const bflb_route_entry_t * route, const ip6_addr_t * dest)
{
    return memcmp(dest, route->prefix.addr, route->prefix_length / 8) == 0;
}

struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest)
{
    bflb_route_entry_t * route = NULL;

    for (route_node_t * node = s_route_list; node != NULL; node = node->next)
    {
        if (route_match(&node->entry, dest) && is_better_route(&node->entry, route))
        {
            route = &node->entry;
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
    bflb_route_entry_t * route = NULL;

    for (route_node_t * node = s_route_list; node != NULL; node = node->next)
    {
        if (node->entry.netif == netif && route_match(&node->entry, dest) && is_better_route(&node->entry, route))
        {
            route = &node->entry;
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
