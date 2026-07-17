#include <stdbool.h>
#include <string.h>

#include "lwip/icmp6.h"
#include "lwip/mld6.h"
#include "lwip/netif.h"
#include "lwip/prot/icmp6.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/nd6.h"
#include "lwip/raw.h"

#include "mtk_route_hook.h"
#include "mtk_route_table.h"
#include <lwip/tcpip.h>

extern void mt793xLog(const char * aFormat, ...);

typedef struct mtk_route_hook_t
{
    struct netif * netif;
    struct raw_pcb * pcb;
    struct mtk_route_hook_t * next;
} mtk_route_hook_t;

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

static mtk_route_hook_t * s_hooks;

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

        if (opt_len == 0 || opt_len > payload_len)
        {
            break;
        }

        if (opt_type == ND6_OPTION_TYPE_ROUTE_INFO && opt_len >= sizeof(rio_header_t) && !is_self_address(netif, src_addr) &&
            payload_len >= opt_len)
        {
            rio_header_t rio_header;
            memcpy(&rio_header, icmp_payload, sizeof(rio_header));

            // skip if prefix is longer than IPv6 address.
            if (rio_header.prefix_length > 128)
            {
                icmp_payload += opt_len;
                payload_len -= opt_len;
                continue;
            }
            uint8_t prefix_len_bytes = (rio_header.prefix_length + 7) / 8;
            // RFC 4191 Section 2.3: Prf=01 High, Prf=11 Low, Prf=00 Medium,
            // Prf=10 is reserved and MUST be treated as Medium.
            uint8_t prf       = (rio_header.preference >> 3) & 3;
            int8_t preference = 0;
            if (prf == 1)
            {
                preference = 1;
            }
            else if (prf == 3)
            {
                preference = -1;
            }
            const uint8_t * rio_data = &icmp_payload[sizeof(rio_header_t)];
            uint8_t rio_data_len     = opt_len - sizeof(rio_header_t);

            mt793xLog("Received RIO");
            if (rio_data_len >= prefix_len_bytes)
            {
                ip6_addr_t prefix;
                mtk_route_entry_t route;

                memset(&prefix, 0, sizeof(prefix));
                memcpy(&prefix.addr, rio_data, prefix_len_bytes);
                route.netif            = netif;
                route.gateway          = *src_addr;
                route.prefix_length    = rio_header.prefix_length;
                route.prefix           = prefix;
                route.preference       = preference;
                route.lifetime_seconds = lwip_ntohl(rio_header.route_lifetime);
                mt793xLog("prefix %s lifetime %lu", ip6addr_ntoa(&prefix), route.lifetime_seconds);
                if (mtk_route_table_add_route_entry(&route) == NULL)
                {
                    mt793xLog("Failed to add route table entry");
                }
                else
                {
                    mt793xLog("Added entry to route table");
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
    mtk_route_hook_t * hook = (mtk_route_hook_t *) arg;

    memcpy(src.addr, ip6_header->src.addr, sizeof(src.addr));
    memcpy(dest.addr, ip6_header->dest.addr, sizeof(dest.addr));
#if LWIP_IPV6_SCOPES
    src.zone = 0;
#endif

    if (p->tot_len != p->len)
    {
        mt793xLog("Ignore segmented ICMP packet");
        return 0;
    }
    if (p->tot_len <= sizeof(struct ip6_hdr) + sizeof(struct icmp6_hdr))
    {
        mt793xLog("Ignore invalid ICMP packet");
        return 0;
    }
    if (!ip6_addr_islinklocal(&dest) && !ip6_addr_isallnodes_linklocal(&dest) && !ip6_addr_isallrouters_linklocal(&dest))
    {
        return 0;
    }

    icmp_payload_len = p->tot_len - sizeof(struct ip6_hdr);
    icmp_payload     = (uint8_t *) p->payload + sizeof(struct ip6_hdr);

    icmp6_header = (struct icmp6_hdr *) icmp_payload;
    if (icmp6_header->type == ICMP6_TYPE_RA)
    {
        ra_recv_handler(hook->netif, icmp_payload, icmp_payload_len, &src);
    }
    return 0;
}

int8_t mtk_route_hook_init()
{
    ip_addr_t router_group    = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0x02);
    mtk_route_hook_t * hook   = NULL;
    uint8_t ret               = 0;
    struct netif * lwip_netif = netif_default;

    LOCK_TCPIP_CORE();

    if (lwip_netif == NULL)
    {
        mt793xLog("Invalid network interface");
        ret = -1;
        goto exit;
    }

    for (mtk_route_hook_t * iter = s_hooks; iter != NULL; iter = iter->next)
    {
        if (iter->netif == lwip_netif)
        {
            mt793xLog("Hook already installed on netif, skip...");
            ret = 0;
            goto exit;
        }
    }

    hook = (mtk_route_hook_t *) malloc(sizeof(mtk_route_hook_t));
    if (hook == NULL)
    {
        mt793xLog("Cannot allocate hook");
        ret = -1;
        goto exit;
    }

    if (mld6_joingroup_netif(lwip_netif, ip_2_ip6(&router_group)) != ERR_OK)
    {
        mt793xLog("Failed to join multicast group");
        ret = -1;
        goto exit;
    }

    hook->netif = lwip_netif;
    hook->pcb   = raw_new_ip_type(IPADDR_TYPE_V6, IP6_NEXTH_ICMP6);
    if (hook->pcb == NULL)
    {
        mt793xLog("Failed to allocate route hook raw PCB");
        ret = -1;
        goto exit;
    }
    hook->pcb->flags |= RAW_FLAGS_MULTICAST_LOOP;
    hook->pcb->chksum_reqd = 1;
    // The ICMPv6 header checksum offset
    hook->pcb->chksum_offset = 2;
    raw_bind_netif(hook->pcb, lwip_netif);
    raw_recv(hook->pcb, icmp6_raw_recv_handler, hook);
    hook->next = s_hooks;
    s_hooks    = hook;

exit:
    if (ret != 0 && hook != NULL)
    {
        free(hook);
    }
    UNLOCK_TCPIP_CORE();

    return ret;
}
