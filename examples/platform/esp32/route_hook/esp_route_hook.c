/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_route_hook.h"

#include <stdint.h>
#include <string.h>

#include "esp_check.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_route_table.h"

#include "lwip/icmp6.h"
#include "lwip/mld6.h"
#include "lwip/netif.h"
#include "lwip/prot/icmp6.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/nd6.h"
#include "lwip/raw.h"

#define HOPLIM_MAX 255
#define PIO_FLAG_ON_LINK (1 << 7)
#define PIO_FLAG_AUTO_CONFIG (1 << 6)

#define TAG "ROUTE_HOOK"

typedef struct esp_route_hook_t
{
    struct netif * netif;
    struct raw_pcb * pcb;
    struct esp_route_hook_t * next;
} esp_route_hook_t;

PACK_STRUCT_BEGIN
struct rio_header_t
{
    PACK_STRUCT_FLD_8(u8_t type);
    PACK_STRUCT_FLD_8(u8_t length);
    PACK_STRUCT_FLD_8(u8_t prefix_length);
    PACK_STRUCT_FLD_8(u8_t preference);
    PACK_STRUCT_FIELD(u32_t route_lifetime);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

typedef struct rio_header_t rio_header_t;

static esp_route_hook_t * s_hooks;

static bool is_self_address(struct netif * netif, const ip6_addr_t * addr)
{
    for (size_t i = 0; i < LWIP_ARRAYSIZE(netif->ip6_addr); i++)
    {
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)) &&
            memcmp(addr->addr, netif_ip6_addr(netif, i)->addr, sizeof(addr->addr)) == 0)
        {
            return true;
        }
    }
    return false;
}

static void ra_recv_handler(struct netif * netif, const uint8_t * icmp_payload, uint16_t payload_len, const ip6_addr_t * src_addr)
{
    if (payload_len < sizeof(struct ra_header))
    {
        return;
    }
    icmp_payload += sizeof(struct ra_header);
    payload_len -= sizeof(struct ra_header);

    while (payload_len >= 2)
    {
        uint8_t opt_type = icmp_payload[0];
        uint8_t opt_len  = icmp_payload[1] << 3;

        if (opt_type == ND6_OPTION_TYPE_ROUTE_INFO && opt_len >= sizeof(rio_header_t) && !is_self_address(netif, src_addr) &&
            payload_len >= opt_len)
        {
            rio_header_t rio_header;
            memcpy(&rio_header, icmp_payload, sizeof(rio_header));

            // skip if prefix is longer than IPv6 address.
            if (rio_header.prefix_length > 128)
            {
                break;
            }
            uint8_t prefix_len_bytes = (rio_header.prefix_length + 7) / 8;
            int8_t preference        = -2 * ((rio_header.preference >> 4) & 1) + (((rio_header.preference) >> 3) & 1);
            const uint8_t * rio_data = &icmp_payload[sizeof(rio_header_t)];
            uint8_t rio_data_len     = opt_len - sizeof(rio_header_t);

            ESP_LOGI(TAG, "Received RIO");
            if (rio_data_len >= prefix_len_bytes)
            {
                ip6_addr_t prefix;
                esp_route_entry_t route;

                memset(&prefix, 0, sizeof(prefix));
                memcpy(&prefix.addr, rio_data, prefix_len_bytes);
                route.netif            = netif;
                route.gateway          = *src_addr;
                route.prefix_length    = rio_header.prefix_length;
                route.prefix           = prefix;
                route.preference       = preference;
                route.lifetime_seconds = lwip_ntohl(rio_header.route_lifetime);
                ESP_LOGI(TAG, "prefix %s lifetime %u\n", ip6addr_ntoa(&prefix), route.lifetime_seconds);
                if (esp_route_table_add_route_entry(&route) == NULL)
                {
                    ESP_LOGI(TAG, "Failed to add route table entry\n");
                }
            }
        }
        icmp_payload += opt_len;
        payload_len -= opt_len;
    }
}

static uint8_t icmp6_raw_recv_handler(void * arg, struct raw_pcb * pcb, struct pbuf * p, const ip_addr_t * addr)
{
    uint8_t * icmp_payload = NULL;
    uint16_t icmp_payload_len;
    struct ip6_hdr * ip6_header = (struct ip6_hdr *) p->payload;
    struct icmp6_hdr * icmp6_header;
    ip6_addr_t src;
    ip6_addr_t dest;
    esp_route_hook_t * hook = (esp_route_hook_t *) arg;

    memcpy(src.addr, ip6_header->src.addr, sizeof(src.addr));
    memcpy(dest.addr, ip6_header->dest.addr, sizeof(dest.addr));
#if LWIP_IPV6_SCOPES
    src.zone = 0;
#endif

    if (p->tot_len != p->len)
    {
        ESP_LOGW(TAG, "Ignore segmented ICMP packet");
        return 0;
    }
    if (p->tot_len <= sizeof(struct ip6_hdr) + sizeof(struct icmp6_hdr))
    {
        ESP_LOGW(TAG, "Ignore invalid ICMP packet");
        return 0;
    }
    if (!ip6_addr_islinklocal(&dest) && !ip6_addr_isallnodes_linklocal(&dest) && !ip6_addr_isallrouters_linklocal(&dest))
    {
        return 0;
    }

    icmp_payload_len = p->tot_len - sizeof(struct ip6_hdr);
    icmp_payload     = p->payload + sizeof(struct ip6_hdr);

    icmp6_header = (struct icmp6_hdr *) icmp_payload;
    if (icmp6_header->type == ICMP6_TYPE_RA)
    {
        ra_recv_handler(hook->netif, icmp_payload, icmp_payload_len, &src);
    }
    return 0;
}

esp_err_t esp_route_hook_init(esp_netif_t * netif)
{
    struct netif * lwip_netif;
    ip_addr_t router_group  = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0x02);
    esp_route_hook_t * hook = NULL;
    esp_err_t ret           = ESP_OK;

    ESP_RETURN_ON_FALSE(netif != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid network interface");
    lwip_netif = netif_get_by_index(esp_netif_get_netif_impl_index(netif));
    ESP_RETURN_ON_FALSE(lwip_netif != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid network interface");

    for (esp_route_hook_t * iter = s_hooks; iter != NULL; iter++)
    {
        if (iter->netif == lwip_netif)
        {
            ESP_LOGI(TAG, "Hook already installed on netif, skip...");
            return ESP_OK;
        }
    }

    hook = (esp_route_hook_t *) malloc(sizeof(esp_route_hook_t));
    ESP_RETURN_ON_FALSE(hook != NULL, ESP_ERR_NO_MEM, TAG, "Cannot allocate hook");

    ESP_GOTO_ON_FALSE(mld6_joingroup_netif(lwip_netif, ip_2_ip6(&router_group)) == ESP_OK, ESP_FAIL, exit, TAG,
                      "Failed to join multicast group");
    hook->netif = lwip_netif;
    hook->pcb   = raw_new_ip_type(IPADDR_TYPE_V6, IP6_NEXTH_ICMP6);
    hook->pcb->flags |= RAW_FLAGS_MULTICAST_LOOP;
    hook->pcb->chksum_reqd = 1;
    // The ICMPv6 header checksum offset
    hook->pcb->chksum_offset = 2;
    raw_bind_netif(hook->pcb, lwip_netif);
    raw_recv(hook->pcb, icmp6_raw_recv_handler, hook);
    hook->next = s_hooks;
    s_hooks    = hook;

exit:
    if (ret != ESP_OK && hook != NULL)
    {
        free(hook);
    }
    return ret;
}
