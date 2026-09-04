/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* C runtime includes */
#include "ti/drivers/net/wifi/wifi_host_driver/inc_adapt/osi_kernel.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/api.h"
#include "lwip/debug.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"

#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "lwip/ethip6.h"
#include "netif/ethernet.h"

/* applications includes */
#include "lwip/apps/httpd.h"
#include "lwip/apps/netbiosns.h"

// #include "default_netif.h"
#include "network_lwip.h"
#include "ti/drivers/net/wifi/wifi_host_driver/inc_adapt/wlan_if.h"
#include "ti_wifi_structs.h"

#if NO_SYS
/* ... then we need information about the timer intervals: */
#include "lwip/igmp.h"
#include "lwip/ip4_frag.h"
#endif /* NO_SYS */

#include "netif/ppp/ppp_opts.h"
#if PPP_SUPPORT
/* PPP includes */
#include "lwip/sio.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/pppoe.h"
#include "netif/ppp/pppos.h"
#if !NO_SYS && !LWIP_PPP_API
#error With NO_SYS==0, LWIP_PPP_API==1 is required.
#endif
#endif /* PPP_SUPPORT */

#ifndef LWIP_EXAMPLE_APP_ABORT
#define LWIP_EXAMPLE_APP_ABORT() 0
#endif

/** Define this to 1 to enable a port-specific ethernet interface as default interface. */
#ifndef USE_DEFAULT_ETH_NETIF
#define USE_DEFAULT_ETH_NETIF 1
#endif

/** Define this to 1 to enable a PPP interface. */
#ifndef USE_PPP
#define USE_PPP 0
#endif

/** Define this to 1 or 2 to support 1 or 2 SLIP interfaces. */
#ifndef USE_SLIPIF
#define USE_SLIPIF 0
#endif

/** Use an ethernet adapter? Default to enabled if port-specific ethernet netif or PPPoE are used. */
#ifndef USE_ETHERNET
#define USE_ETHERNET (USE_DEFAULT_ETH_NETIF || PPPOE_SUPPORT)
#endif

/** Use an ethernet adapter for TCP/IP? By default only if port-specific ethernet netif is used. */
#ifndef USE_ETHERNET_TCPIP
#define USE_ETHERNET_TCPIP (USE_DEFAULT_ETH_NETIF)
#endif

#if USE_SLIPIF
#include <netif/slipif.h>
#endif /* USE_SLIPIF */

#ifndef USE_DHCP
#define USE_DHCP LWIP_DHCP
#endif
#ifndef USE_AUTOIP
#define USE_AUTOIP LWIP_AUTOIP
#endif

#define ETH_MAX_PAYLOAD 1514
#define VLAN_TAG_SIZE (4U)
#define ETHHDR_SIZE 14
#define ETH_FRAME_SIZE (ETH_MAX_PAYLOAD + VLAN_TAG_SIZE)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* globales variables for netifs */
#if USE_ETHERNET
#if LWIP_DHCP
/* dhcp struct for the ethernet netif */
struct dhcp netif_dhcp;
#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
/* autoip struct for the ethernet netif */
struct autoip netif_autoip;
#endif /* LWIP_AUTOIP */
#endif /* USE_ETHERNET */
#if USE_PPP
/* THE PPP PCB */
ppp_pcb * ppp;
/* THE PPP interface */
struct netif ppp_netif;
/* THE PPP descriptor */
u8_t sio_idx = 0;
sio_fd_t ppp_sio;
#endif /* USE_PPP */
#if USE_SLIPIF
struct netif slipif1;
#if USE_SLIPIF > 1
struct netif slipif2;
#endif /* USE_SLIPIF > 1 */
#endif /* USE_SLIPIF */

// #define STATIC_IP
#ifdef STATIC_IP
#ifdef CC35XX
const char static_ip[4] = { 10, 0, 0, 103 };
const char static_gw[4] = { 10, 0, 0, 1 };
#endif
#ifdef CC33XX
const char static_ip[4] = { 10, 123, 45, 12 };
const char static_gw[4] = { 10, 123, 45, 1 };
#endif
const char static_mask[4] = { 255, 255, 255, 0 };

#endif

struct netif staif = { 0 };
struct netif apif  = { 0 };
// extern
uint32_t isIp = 0;
appControlBlock app_CB;
uint32_t ActiveNetIfBitMap = 0x00;

int update_arp(void * ip_addr)
{
    err_t result = ERR_CONN;
    ssize_t arp_find;
    struct netif * pNetIf     = &staif;
    struct eth_addr * eth_ret = NULL;
    ip4_addr_t * ip4_ret      = NULL;
    ip4_addr_t * ip4_addr     = (ip4_addr_t *) ip_addr;

    if (netif_is_up(pNetIf))
    {
        LOCK_TCPIP_CORE();
        result = etharp_query(pNetIf, (const ip4_addr_t *) ip4_addr, NULL);
        UNLOCK_TCPIP_CORE();
        if (result == ERR_OK)
        {
            result                   = ERR_CONN;
            const int kMaxArpRetries = 50;
            for (int retry = 0; retry < kMaxArpRetries; retry++)
            {
                osi_uSleep(100);
                eth_ret  = NULL;
                ip4_ret  = NULL;
                arp_find = etharp_find_addr(pNetIf, (ip4_addr_t *) ip4_addr, &eth_ret, (const ip4_addr_t **) &ip4_ret);
                if (arp_find >= 0 && ip4_ret && eth_ret && ip4_addr_cmp(ip4_ret, ip4_addr))
                {
                    result = ERR_OK;
                    break;
                }
            }
        }
    }
    return result;
}

void status_callback(struct netif * state_netif)
{
    WlanRole_e roleid;
    if (network_get_sta_if() == state_netif)
    {
        roleid = WLAN_ROLE_STA;
    }
    else if (network_get_ap_if() == state_netif)
    {
        roleid = WLAN_ROLE_AP;
    }
    if (netif_is_up(state_netif))
    {
        const ip4_addr_t * temp;

        WlanMacAddress_t macAddressParams;
        memset(&macAddressParams, 0, sizeof(WlanMacAddress_t));

        macAddressParams.roleType = roleid;

        Wlan_Get(WLAN_GET_MACADDRESS, (void *) &macAddressParams);

        memcpy(state_netif->hwaddr, macAddressParams.pMacAddress, sizeof(macAddressParams.pMacAddress));

        state_netif->hwaddr_len = 6;
        Report("status_callback==UP, local interface IP is %s\r\n", ip4addr_ntoa(netif_ip4_addr(state_netif)));
        temp = netif_ip4_addr(state_netif);
        if (temp->addr)
        {
            isIp = 1;
        }
        else
        {
            isIp = 0;
        }
    }
    else
    {
        Report("status_callback==DOWN\r\n");
    }
}

void printBuffer(uint8_t * in, uint32_t len)
{
    int j, total;
    total = 0;

    Report("\n\r");

    while (total < 8)
    {
        for (j = 0; j < 8; j++)
        {
            Report("%x ", in[total]);
            total++;
        }
        Report("\n\r");
    }
}

void network_recv(WlanRole_e roleId, uint8_t * inBuf, uint32_t inLen)
{
    struct netif * pIf = NULL;
    struct pbuf * packet;

    if (roleId == WLAN_ROLE_STA)
    {
        pIf = (struct netif *) network_get_sta_if();
    }
    else if (roleId == WLAN_ROLE_AP)
    {
        pIf = (struct netif *) network_get_ap_if();
    }
    else
    {
        assert(0);
    }

    packet = pbuf_alloc(PBUF_RAW, inLen, PBUF_POOL);
    if (!packet)
    {
        //        Report("\nno storage for allocating packets, DROP");
        return;
    }
    memcpy(packet->payload, inBuf, inLen);
    packet->len     = inLen;
    packet->tot_len = inLen;

    tcpip_input(packet, pIf);
}

void link_callback(struct netif * state_netif)
{
    WlanRole_e roleid;
    err_t err;
    if (network_get_sta_if() == state_netif)
    {
        roleid = WLAN_ROLE_STA;
    }
    else if (network_get_ap_if() == state_netif)
    {
        roleid = WLAN_ROLE_AP;
    }
    if (netif_is_link_up(state_netif))
    {
        Wlan_EtherPacketRecvRegisterCallback(roleid, network_recv);
        if (roleid == WLAN_ROLE_STA)
        {
            WlanMacAddress_t macAddressParams;
            memset(&macAddressParams, 0, sizeof(WlanMacAddress_t));
            macAddressParams.roleType = roleid;
            Wlan_Get(WLAN_GET_MACADDRESS, (void *) &macAddressParams);
            memcpy(state_netif->hwaddr, macAddressParams.pMacAddress, sizeof(macAddressParams.pMacAddress));
            state_netif->hwaddr_len = 6;
            netif_create_ip6_linklocal_address(state_netif, 1);

            if (!isIp)
            {
                Report("\n\r link_callback==UP starting DHCP");
#ifdef STATIC_IP
                err = dhcp_start(state_netif);
                dhcp_inform(state_netif);
                etharp_gratuitous(state_netif);
#else
                err = dhcp_start(state_netif);
#endif
                Report("\n\r DHCP is %d\n\n\r", err);
            }
            else
            {
                dhcp_inform(state_netif);
                etharp_gratuitous(state_netif);
            }
        }
    }
    else
    {
        Wlan_EtherPacketRecvRegisterCallback(roleid, NULL);
        if (roleid == WLAN_ROLE_STA)
        {
            dhcp_stop(state_netif);
            Report("DHCP stopped\r\n");
        }

        Report("link_callback==DOWN\r\n");
    }
}

err_t network_send(struct netif * netif, struct pbuf * p)
{
    WlanRole_e role;
    uint16_t total_len;
    struct pbuf * currentPacket = p;
    uint8_t * buff;
    uint16_t offset = 0;

    total_len = currentPacket->tot_len;

    if (total_len != currentPacket->len)
    {
        buff = os_malloc(total_len);
        // aggregate all packets
        do
        {
            memcpy(buff + offset, currentPacket->payload, currentPacket->len);
            offset += currentPacket->len;
            currentPacket = currentPacket->next;
        } while (currentPacket);
    }
    else
    {
        buff = currentPacket->payload;
    }

    if (netif_is_up(netif))
    {
        if (netif == &staif)
        {
            role = WLAN_ROLE_STA;
        }
        else
        {
            role = WLAN_ROLE_AP;
        }

        Wlan_EtherPacketSend(role, buff, total_len, 0);
#ifdef CC33XX
        osi_uSleep(10);
#endif // CC33XX
    }
    if (buff != currentPacket->payload)
    {
        os_free(buff);
    }
    // done review
    return ERR_OK;
}

void tcpinternal_network_set_up(void * newif)
{
    struct netif * nif = newif;
#if 0
#ifdef STATIC_IP
    WlanRole_e role;
    if(nif == &staif)
    {
        role = WLAN_ROLE_STA;
    }
    else
    {
        role = WLAN_ROLE_AP;
    }

    if(WLAN_ROLE_AP != role)
    {
        struct ip4_addr ipaddr;
        struct ip4_addr netmask;
        struct ip4_addr gw;

        IP4_ADDR(&ipaddr, static_ip[0], static_ip[1], static_ip[2], static_ip[3]);
        IP4_ADDR(&netmask, static_mask[0], static_mask[1], static_mask[2], static_mask[3]);
        IP4_ADDR(&gw, static_gw[0], static_gw[1], static_gw[2], static_gw[3]);

        dhcp_release(nif);

        dhcp_stop(nif);

        netif_set_addr(nif, &ipaddr, &netmask, &gw);
    }
#endif
#endif
    netif_set_up(nif);
    nif->mtu = ETH_FRAME_SIZE - ETHHDR_SIZE - VLAN_TAG_SIZE;
    netif_set_link_up(nif);
}

void tcpinternal_network_set_down(void * newif)
{
    struct netif * nif = newif;
    netif_set_down(nif);
    nif->mtu = ETH_FRAME_SIZE - ETHHDR_SIZE - VLAN_TAG_SIZE;
    netif_set_link_down(nif);
}

void _network_stop(struct netif * state_netif)
{
    if (network_get_sta_if() == state_netif)
    {
        Wlan_RoleDown(WLAN_ROLE_STA, WLAN_WAIT_FOREVER);
    }
    else if (network_get_ap_if() == state_netif)
    {
        Wlan_RoleDown(WLAN_ROLE_AP, WLAN_WAIT_FOREVER);
    }
    else
    {
        Report("\n_network_stop");
        assert(0);
    }
}

signed char _role_sta_up(struct netif * newif)
{
    netif_set_status_callback(newif, status_callback);
    netif_set_link_callback(newif, link_callback);
    autoip_set_struct(newif, &netif_autoip);
    dhcp_set_struct(newif, &netif_dhcp);

    newif->name[0] = 's';
    newif->name[1] = 't';

    newif->mtu = ETH_FRAME_SIZE - ETHHDR_SIZE - VLAN_TAG_SIZE;

    /* Populate the Driver Interface Functions. */
    // newif->remove_callback      = _network_stop;
    newif->output     = etharp_output;
    newif->output_ip6 = ethip6_output;
    newif->linkoutput = network_send;
    newif->flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST;

    // Signal to application that network interface is ready for Wlan_RoleUp
    if (app_CB.CON_CB.staRoleupSyncObj != NULL)
    {
        osi_SyncObjSignal(&app_CB.CON_CB.staRoleupSyncObj);
    }

    return 0;
}

signed char _role_ap_up(struct netif * newif)
{
    netif_set_status_callback(newif, status_callback);
    netif_set_link_callback(newif, link_callback);
    // autoip_set_struct(newif, &netif_autoip);
    // dhcp_set_struct(newif, &netif_dhcp);

    newif->name[0] = 'a';
    newif->name[1] = 'p';

    newif->mtu = ETH_FRAME_SIZE - ETHHDR_SIZE - VLAN_TAG_SIZE;

    /* Populate the Driver Interface Functions. */
    // newif->remove_callback      = _network_stop;
    newif->output     = etharp_output;
    newif->linkoutput = network_send;
    newif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

    return 0;
}

void tcpip_network_stack_add_if_sta(void * ctx)
{
    ip4_addr_t ipaddr, netmask, gw;

    struct netif * pNetIf = &staif;
    if (!netif_is_up(pNetIf))
    {
        memset(pNetIf, 0, sizeof(struct netif));

        ip4_addr_set_zero(&gw);
        ip4_addr_set_zero(&ipaddr);
        ip4_addr_set_zero(&netmask);

        netif_add(pNetIf, &ipaddr, &netmask, &gw, NULL, _role_sta_up, tcpip_input);
    }
}

void tcpip_network_stack_remove_if_sta(void * ctx)
{
    struct netif * pNetIf = &staif;
    tcpinternal_network_set_down(pNetIf);
    etharp_cleanup_netif(pNetIf);
    netif_remove(pNetIf);

    // Signal to application that network interface has been removed
    if (app_CB.CON_CB.staRoledownSyncObj != NULL)
    {
        osi_SyncObjSignal(&app_CB.CON_CB.staRoledownSyncObj);
    }
}

void tcpip_network_stack_add_if_ap(void * ctx)
{
    ip4_addr_t ipaddr, netmask, gw;

    struct netif * pNetIf = &apif;
    if (!netif_is_up(pNetIf))
    {
        memset(pNetIf, 0, sizeof(struct netif));

        IP4_ADDR(&ipaddr, 10, 0, 0, 3);
        IP4_ADDR(&netmask, 255, 255, 255, 0);
        IP4_ADDR(&gw, 10, 0, 0, 1);

        netif_add(pNetIf, &ipaddr, &netmask, &gw, NULL, _role_ap_up, tcpip_input);
        netif_set_addr(pNetIf, &ipaddr, &netmask, &gw);
        netif_set_ipaddr(pNetIf, &ipaddr);
    }
}

void tcpip_network_stack_remove_if_ap(void * ctx)
{
    struct netif * pNetIf = &apif;
    tcpinternal_network_set_down(pNetIf);
    netif_remove(pNetIf);
}

/* This function initializes this lwIP test. When NO_SYS=1, this is done in
 * the main_loop context (there is no other one), when NO_SYS=0, this is done
 * in the tcpip_thread context */
void tcpip_network_internal_init(void * arg)
{ /* remove compiler warning */
    sys_sem_t * init_sem;
    LWIP_ASSERT("arg != NULL", arg != NULL);
    init_sem = (sys_sem_t *) arg;
    sys_sem_signal(init_sem);
}

/*************************** network API *****************************/

void * network_get_sta_if()
{
    return (void *) &staif;
}

void * network_get_ap_if()
{
    return (void *) &apif;
}

void network_set_up(void * newif)
{
    tcpip_callback(tcpinternal_network_set_up, newif);
}

void network_set_down(void * newif)
{ /* suspect need to move to when there is wlan connect event */
    if (netif_is_up((struct netif *) newif))
    {
        tcpip_callback(tcpinternal_network_set_down, newif);
    }
}

void network_stack_add_if_sta()
{
    tcpip_callback(tcpip_network_stack_add_if_sta, NULL);
}

void network_stack_remove_if_sta()
{
    tcpip_callback(tcpip_network_stack_remove_if_sta, NULL);
}

void network_stack_add_if_ap()
{
    tcpip_callback(tcpip_network_stack_add_if_ap, NULL);
}

void network_stack_remove_if_ap()
{
    tcpip_callback(tcpip_network_stack_remove_if_ap, NULL);
}

void network_stack_init()
{
    sys_sem_t init_sem;
    sys_sem_new(&init_sem, 0);
    tcpip_init(tcpip_network_internal_init, &init_sem);
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
}
