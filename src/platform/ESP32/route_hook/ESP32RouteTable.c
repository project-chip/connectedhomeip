/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ESP32/route_hook/ESP32RouteTable.h>

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#define LWIP_MAX_TIMEOUT_SECONDS LWIP_UINT32_MAX / (1000 * 4) // lwIP defined reasonable timeout value

static esp_route_entry_t * s_route_entries = NULL;

static esp_route_entry_t * find_route_entry(const esp_route_entry_t * route_entry)
{
    for (esp_route_entry_t * iter = s_route_entries; iter != NULL; iter = iter->next)
    {
        if (iter->netif == route_entry->netif && iter->prefix_length == route_entry->prefix_length &&
            memcmp(iter->gateway.addr, route_entry->gateway.addr, sizeof(route_entry->gateway.addr)) == 0 &&
            memcmp(iter->prefix.addr, route_entry->prefix.addr, route_entry->prefix_length / 8) == 0)
        {
            return iter;
        }
    }

    return NULL;
}

static esp_err_t remove_route_entry(esp_route_entry_t * route_entry)
{
    if (s_route_entries == route_entry)
    {
        s_route_entries = s_route_entries->next;
        free(route_entry);
        return ESP_OK;
    }

    for (esp_route_entry_t * iter = s_route_entries; iter != NULL; iter = iter->next)
    {
        if (iter->next == route_entry)
        {
            iter->next = route_entry->next;
            free(route_entry);
            return ESP_OK;
        }
    }

    ESP_LOGW(TAG, "The given route entry is not found");
    return ESP_ERR_NOT_FOUND;
}

static void route_timeout_handler(void * arg)
{
    esp_route_entry_t * route = (esp_route_entry_t *) arg;
    if (route->lifetime_seconds <= LWIP_MAX_TIMEOUT_SECONDS)
    {
        remove_route_entry(route);
    }
    else
    {
        route->lifetime_seconds -= LWIP_MAX_TIMEOUT_SECONDS;
        sys_timeout(route->lifetime_seconds <= LWIP_MAX_TIMEOUT_SECONDS ? route->lifetime_seconds * 1000
                                                                        : LWIP_MAX_TIMEOUT_SECONDS * 1000,
                    route_timeout_handler, route);
    }
}

esp_route_entry_t * esp_route_table_add_route_entry(const esp_route_entry_t * route_entry)
{
    if (route_entry == NULL)
    {
        return NULL;
    }

    esp_route_entry_t * entry = find_route_entry(route_entry);

    if (entry == NULL)
    {
        entry = (esp_route_entry_t *) malloc(sizeof(esp_route_entry_t));
        if (entry == NULL)
        {
            ESP_LOGW(TAG, "Cannot allocate route entry");
            return NULL;
        }

        entry->netif   = route_entry->netif;
        entry->gateway = route_entry->gateway;
        ip6_addr_assign_zone(&entry->gateway, IP6_UNICAST, entry->netif);
        entry->prefix        = route_entry->prefix;
        entry->prefix_length = route_entry->prefix_length;

        entry->next     = s_route_entries;
        s_route_entries = entry;
    }
    else
    {
        sys_untimeout(route_timeout_handler, entry);
    }
    entry->preference       = route_entry->preference;
    entry->lifetime_seconds = route_entry->lifetime_seconds;
    if (entry->lifetime_seconds != LWIP_UINT32_MAX)
    {
        sys_timeout(entry->lifetime_seconds <= LWIP_MAX_TIMEOUT_SECONDS ? entry->lifetime_seconds * 1000
                                                                        : LWIP_MAX_TIMEOUT_SECONDS * 1000,
                    route_timeout_handler, entry);
    }

    return entry;
}

static inline bool is_better_route(const esp_route_entry_t * lhs, const esp_route_entry_t * rhs)
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

static inline bool route_match(const esp_route_entry_t * route, const ip6_addr_t * dest)
{
    return memcmp(dest, route->prefix.addr, route->prefix_length / 8) == 0;
}

struct netif * lwip_hook_ip6_route(const ip6_addr_t * src, const ip6_addr_t * dest)
{
    esp_route_entry_t * route = NULL;

    for (esp_route_entry_t * iter = s_route_entries; iter != NULL; iter = iter->next)
    {
        if (route_match(iter, dest) && is_better_route(iter, route))
        {
            route = iter;
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
    esp_route_entry_t * route = NULL;

    for (esp_route_entry_t * iter = s_route_entries; iter != NULL; iter = iter->next)
    {
        if (iter->netif == netif && route_match(iter, dest) && is_better_route(iter, route))
        {
            route = iter;
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
