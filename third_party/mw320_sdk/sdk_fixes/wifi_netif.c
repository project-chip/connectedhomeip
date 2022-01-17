
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

/** @file netif.c
 *
 *  @brief  This file provides network interface initialization code
 *
 *  Copyright 2008-2020 NXP
 *
 */

/*------------------------------------------------------*/
#include <netif_decl.h>
/*------------------------------------------------------*/
uint16_t g_data_nf_last;
uint16_t g_data_snr_last;
static struct netif * netif_arr[MAX_INTERFACES_SUPPORTED];
static t_u8 rfc1042_eth_hdr[MLAN_MAC_ADDR_LENGTH] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
/*------------------------------------------------------*/
static void register_interface(struct netif * iface, mlan_bss_type iface_type)
{
    netif_arr[iface_type] = iface;
}

void deliver_packet_above(struct pbuf * p, int recv_interface)
{
    err_t lwiperr = ERR_OK;
    /* points to packet payload, which starts with an Ethernet header */
    struct eth_hdr * ethhdr = p->payload;

    switch (htons(ethhdr->type))
    {
    case ETHTYPE_IP:
#ifdef CONFIG_IPV6
    case ETHTYPE_IPV6:
#endif
    case ETHTYPE_ARP:
        if (recv_interface >= MAX_INTERFACES_SUPPORTED)
        {
            while (true)
                ;
        }

        /* full packet send to tcpip_thread to process */
        lwiperr = netif_arr[recv_interface]->input(p, netif_arr[recv_interface]);
        if (lwiperr != ERR_OK)
        {
            LINK_STATS_INC(link.proterr);
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
        }
        break;
    case ETHTYPE_EAPOL:

        pbuf_free(p);
        p = NULL;
        break;
    default:
        /* drop the packet */
        LINK_STATS_INC(link.drop);
        pbuf_free(p);
        p = NULL;
        break;
    }
}

static struct pbuf * gen_pbuf_from_data(t_u8 * payload, t_u16 datalen)
{
    /* We allocate a pbuf chain of pbufs from the pool. */
    struct pbuf * p = pbuf_alloc(PBUF_RAW, datalen, PBUF_POOL);
    if (!p)
        return NULL;

    if (pbuf_take(p, payload, datalen) != 0)
    {
        pbuf_free(p);
        p = NULL;
    }

    return p;
}

static void process_data_packet(const t_u8 * rcvdata, const t_u16 datalen)
{
    RxPD * rxpd        = (RxPD *) ((t_u8 *) rcvdata + INTF_HEADER_LEN);
    int recv_interface = rxpd->bss_type;

    if (rxpd->rx_pkt_type == PKT_TYPE_AMSDU)
    {
        wrapper_wlan_handle_amsdu_rx_packet(rcvdata, datalen);
        return;
    }

    if (recv_interface == MLAN_BSS_TYPE_STA || recv_interface == MLAN_BSS_TYPE_UAP)
    {
        g_data_nf_last  = rxpd->nf;
        g_data_snr_last = rxpd->snr;
    }

    t_u8 * payload  = (t_u8 *) rxpd + rxpd->rx_pkt_offset;
    struct pbuf * p = gen_pbuf_from_data(payload, rxpd->rx_pkt_length);
    /* If there are no more buffers, we do nothing, so the data is
       lost. We have to go back and read the other ports */
    if (p == NULL)
    {
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        return;
    }

    /* points to packet payload, which starts with an Ethernet header */
    struct eth_hdr * ethhdr = p->payload;

#ifdef CONFIG_FILTER_LOCALLY_ADMINISTERED_AND_SELF_MAC_ADDR
    if ((ISLOCALLY_ADMINISTERED_ADDR(ethhdr->src.addr[0]) &&
         (!memcmp(&ethhdr->src.addr[3], &netif_arr[recv_interface]->hwaddr[3], 3))) ||
        (!memcmp(&ethhdr->src.addr, &netif_arr[recv_interface]->hwaddr, ETHARP_HWADDR_LEN)))
    {
        pbuf_free(p);
        p = NULL;
        return;
    }
#endif

    if (!memcmp((t_u8 *) p->payload + SIZEOF_ETH_HDR, rfc1042_eth_hdr, sizeof(rfc1042_eth_hdr)))
    {
        struct eth_llc_hdr * ethllchdr = (struct eth_llc_hdr *) ((t_u8 *) p->payload + SIZEOF_ETH_HDR);
        ethhdr->type                   = ethllchdr->type;
        p->len -= SIZEOF_ETH_LLC_HDR;
        (void) memcpy((t_u8 *) p->payload + SIZEOF_ETH_HDR, (t_u8 *) p->payload + SIZEOF_ETH_HDR + SIZEOF_ETH_LLC_HDR,
                      p->len - SIZEOF_ETH_LLC_HDR);
    }
    switch (htons(ethhdr->type))
    {
    case ETHTYPE_IP:
#ifdef CONFIG_IPV6
    case ETHTYPE_IPV6:
#endif
        /* To avoid processing of unwanted udp broadcast packets, adding
         * filter for dropping packets received on ports other than
         * pre-defined ports.
         */
        LINK_STATS_INC(link.recv);
        if (recv_interface == MLAN_BSS_TYPE_STA)
        {
            int rv = wrapper_wlan_handle_rx_packet(datalen, rxpd, p, payload);
            if (rv != WM_SUCCESS)
            {
                /* mlan was unsuccessful in delivering the
                   packet */
                LINK_STATS_INC(link.drop);
                pbuf_free(p);
            }
        }
        else
            deliver_packet_above(p, recv_interface);
        p = NULL;
        break;
    case ETHTYPE_ARP:
    case ETHTYPE_EAPOL:
        LINK_STATS_INC(link.recv);
        deliver_packet_above(p, recv_interface);
        break;
    default:
        /* fixme: avoid pbuf allocation in this case */
        LINK_STATS_INC(link.drop);
        pbuf_free(p);
        break;
    }
}

/* Callback function called from the wifi module */
void handle_data_packet(const t_u8 interface, const t_u8 * rcvdata, const t_u16 datalen)
{
    if (netif_arr[interface] != NULL)
        process_data_packet(rcvdata, datalen);
}

void handle_amsdu_data_packet(t_u8 interface, t_u8 * rcvdata, t_u16 datalen)
{
    struct pbuf * p = gen_pbuf_from_data(rcvdata, datalen);
    if (!p)
    {
        w_pkt_e("[amsdu] No pbuf available. Dropping packet");
        return;
    }

    deliver_packet_above(p, interface);
}

void handle_deliver_packet_above(t_u8 interface, t_void * lwip_pbuf)
{
    struct pbuf * p = (struct pbuf *) lwip_pbuf;

    deliver_packet_above(p, interface);
}

bool wrapper_net_is_ip_or_ipv6(const t_u8 * buffer)
{
    return net_is_ip_or_ipv6(buffer);
}

/**
 * Should be called at the beginning of the program to set up the
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
void low_level_init(struct netif * netif)
{
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    wlan_get_mac_address(netif->hwaddr);

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif * netif, struct pbuf * p)
{
    int ret;
    struct pbuf * q;
    struct ethernetif * ethernetif = netif->state;
    u32_t pkt_len, outbuf_len;
#ifdef CONFIG_WMM
    t_u8 tid;
    int pkt_prio = wifi_wmm_get_pkt_prio(p->payload, &tid);
    if (pkt_prio == -WM_FAIL)
    {
        return ERR_MEM;
    }
    ret = is_wifi_wmm_queue_full(pkt_prio);
    if (ret == true)
    {
        return ERR_MEM;
    }
    uint8_t * outbuf = wifi_wmm_get_outbuf(&outbuf_len, pkt_prio);
#else
    uint8_t * outbuf = wifi_get_outbuf(&outbuf_len);
#endif
    if (!outbuf)
        return ERR_MEM;

    pkt_len = sizeof(TxPD) + INTF_HEADER_LEN;

    (void) memset(outbuf, 0x00, pkt_len);

    for (q = p; q != NULL; q = q->next)
    {
        if (pkt_len > outbuf_len)
        {
            while (true)
            {
                LWIP_DEBUGF(NETIF_DEBUG,
                            ("PANIC: Xmit packet"
                             "is bigger than inbuf.\r\n"));
                vTaskDelay((3000) / portTICK_RATE_MS);
            }
        }
        (void) memcpy((u8_t *) outbuf + pkt_len, (u8_t *) q->payload, q->len);
        pkt_len += q->len;
    }

    ret = wifi_low_level_output(ethernetif->interface, outbuf + sizeof(TxPD) + INTF_HEADER_LEN,
                                pkt_len - sizeof(TxPD) - INTF_HEADER_LEN
#ifdef CONFIG_WMM
                                ,
                                pkt_prio, tid
#endif
    );

    if (ret == -WM_E_NOMEM)
    {
        LINK_STATS_INC(link.err);
        ret = ERR_MEM;
    }
    else if (ret == -WM_E_BUSY)
    {
        LINK_STATS_INC(link.err);
        ret = ERR_TIMEOUT;
    }
    else if (ret == WM_SUCCESS)
    {
        LINK_STATS_INC(link.xmit);
        ret = ERR_OK;
    }
    else
    { /* Do Nothing */
    }

    return ret;
}

/* Below struct is used for creating IGMP IPv4 multicast list */
typedef struct group_ip4_addr
{
    struct group_ip4_addr * next;
    uint32_t group_ip;
} group_ip4_addr_t;

/* Head of list that will contain IPv4 multicast IP's */
static group_ip4_addr_t * igmp_ip4_list;

/* Callback called by LwiP to add or delete an entry in the multicast filter table */
static err_t igmp_mac_filter(struct netif * netif, const ip4_addr_t * group, enum netif_mac_filter_action action)
{
    uint8_t mcast_mac[6];
    err_t result;
    int error;

    /* IPv4 to MAC conversion as per section 6.4 of rfc1112 */
    wifi_get_ipv4_multicast_mac(ntohl(group->addr), mcast_mac);
    group_ip4_addr_t *curr, *prev;

    switch (action)
    {
    case NETIF_ADD_MAC_FILTER:
        /* LwIP takes care of duplicate IP addresses and it always send
         * unique IP address. Simply add IP to top of list*/
        curr = (group_ip4_addr_t *) os_mem_alloc(sizeof(group_ip4_addr_t));
        if (curr == NULL)
        {
            result = ERR_IF;
            goto done;
        }
        curr->group_ip = group->addr;
        curr->next     = igmp_ip4_list;
        igmp_ip4_list  = curr;
        /* Add multicast MAC filter */
        error = wifi_add_mcast_filter(mcast_mac);
        if (error == 0)
        {
            result = ERR_OK;
        }
        else if (error == -WM_E_EXIST)
        {
            result = ERR_OK;
        }
        else
        {
            /* In case of failure remove IP from list */
            curr          = igmp_ip4_list;
            igmp_ip4_list = curr->next;
            os_mem_free(curr);
            curr   = NULL;
            result = ERR_IF;
        }
        break;
    case NETIF_DEL_MAC_FILTER:
        /* Remove multicast IP address from list */
        curr = igmp_ip4_list;
        prev = curr;
        while (curr != NULL)
        {
            if (curr->group_ip == group->addr)
            {
                if (prev == curr)
                {
                    igmp_ip4_list = curr->next;
                    os_mem_free(curr);
                }
                else
                {
                    prev->next = curr->next;
                    os_mem_free(curr);
                }
                curr = NULL;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        /* Check if other IP is mapped to same MAC */
        curr = igmp_ip4_list;
        while (curr != NULL)
        {
            /* If other IP is mapped to same MAC than skip Multicast MAC removal */
            if ((ntohl(curr->group_ip) & 0x7FFFFF) == (ntohl(group->addr) & 0x7FFFFF))
            {
                result = ERR_OK;
                goto done;
            }
            curr = curr->next;
        }
        /* Remove Multicast MAC filter */
        error = wifi_remove_mcast_filter(mcast_mac);
        if (error == 0)
        {
            result = ERR_OK;
        }
        else
        {
            result = ERR_IF;
        }
        break;
    default:
        result = ERR_IF;
        break;
    }
done:
    return result;
}

#ifdef CONFIG_IPV6
/* Below struct is used for creating IGMP IPv6 multicast list */
typedef struct group_ip6_addr
{
    struct group_ip6_addr * next;
    uint32_t group_ip;
} group_ip6_addr_t;

/* Head of list that will contain IPv6 multicast IP's */
static group_ip6_addr_t * mld_ip6_list;

/* Callback called by LwiP to add or delete an entry in the IPv6 multicast filter table */
static err_t mld_mac_filter(struct netif * netif, const ip6_addr_t * group, enum netif_mac_filter_action action)
{
    uint8_t mcast_mac[6];
    err_t result;
    int error;

    /* IPv6 to MAC conversion as per section 7 of rfc2464 */
    wifi_get_ipv6_multicast_mac(ntohl(group->addr[3]), mcast_mac);
    group_ip6_addr_t *curr, *prev;

    switch (action)
    {
    case NETIF_ADD_MAC_FILTER:
        /* LwIP takes care of duplicate IP addresses and it always send
         * unique IP address. Simply add IP to top of list*/
        curr = (group_ip6_addr_t *) os_mem_alloc(sizeof(group_ip6_addr_t));
        if (curr == NULL)
        {
            result = ERR_IF;
            goto done;
        }
        curr->group_ip = group->addr[3];
        curr->next     = mld_ip6_list;
        mld_ip6_list   = curr;
        /* Add multicast MAC filter */
        error = wifi_add_mcast_filter(mcast_mac);
        if (error == 0)
        {
            result = ERR_OK;
        }
        else if (error == -WM_E_EXIST)
        {
            result = ERR_OK;
        }
        else
        {
            /* In case of failure remove IP from list */
            curr         = mld_ip6_list;
            mld_ip6_list = mld_ip6_list->next;
            os_mem_free(curr);
            curr   = NULL;
            result = ERR_IF;
        }
        break;
    case NETIF_DEL_MAC_FILTER:
        /* Remove multicast IP address from list */
        curr = mld_ip6_list;
        prev = curr;
        while (curr != NULL)
        {
            if (curr->group_ip == group->addr[3])
            {
                if (prev == curr)
                {
                    mld_ip6_list = curr->next;
                    os_mem_free(curr);
                }
                else
                {
                    prev->next = curr->next;
                    os_mem_free(curr);
                }
                curr = NULL;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        /* Check if other IP is mapped to same MAC */
        curr = mld_ip6_list;
        while (curr != NULL)
        {
            /* If other IP is mapped to same MAC than skip Multicast MAC removal */
            if ((ntohl(curr->group_ip) & 0xFFFFFF) == (ntohl(group->addr[3]) & 0xFFFFFF))
            {
                result = ERR_OK;
                goto done;
            }
            curr = curr->next;
        }
        /* Remove Multicast MAC filter */
        error = wifi_remove_mcast_filter(mcast_mac);
        if (error == 0)
        {
            result = ERR_OK;
        }
        else
        {
            result = ERR_IF;
        }
        break;
    default:
        result = ERR_IF;
        break;
    }
done:
    return result;
}
#endif /* #ifdef CONFIG_IPV6 */

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netifapi_netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t lwip_netif_init(struct netif * netif)
{
    struct ethernetif * ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    ethernetif->interface = MLAN_BSS_TYPE_STA;
    netif->state          = ethernetif;
    netif->name[0]        = IFNAME0;
    netif->name[1]        = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;
#ifdef CONFIG_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    netif_set_igmp_mac_filter(netif, igmp_mac_filter);
    netif->flags |= NETIF_FLAG_IGMP;
#ifdef CONFIG_IPV6
    netif_set_mld_mac_filter(netif, mld_mac_filter);
    netif->flags |= NETIF_FLAG_MLD6;
#endif

    register_interface(netif, MLAN_BSS_TYPE_STA);
    return ERR_OK;
}

err_t lwip_netif_uap_init(struct netif * netif)
{
    struct ethernetif * ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

    ethernetif->interface = MLAN_BSS_TYPE_UAP;
    netif->state          = ethernetif;
    netif->name[0]        = 'u';
    netif->name[1]        = 'a';
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;

#ifdef CONFIG_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    netif_set_igmp_mac_filter(netif, igmp_mac_filter);
    netif->flags |= NETIF_FLAG_IGMP;
#ifdef CONFIG_IPV6
    netif_set_mld_mac_filter(netif, mld_mac_filter);
    netif->flags |= NETIF_FLAG_MLD6;
#endif

    register_interface(netif, MLAN_BSS_TYPE_UAP);

    return ERR_OK;
}
