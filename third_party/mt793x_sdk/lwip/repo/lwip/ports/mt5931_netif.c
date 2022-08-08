/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/**
 * @file
 *
 * ps interface.
 */

#include <stdio.h>
#include <string.h>

#include "lwip/opt.h"
#include <stdint.h>
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>
#include "lwip/ethip6.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/netif.h"

#include "mt5931_netif.h"
#include "mtk_wifi_adapter.h"

#ifdef TCPIP_FOR_MT5931_ENABLE
#include "task.h"

#define IFNAME00 's'
#define IFNAME01 't'
#define IFNAME10 'a'
#define IFNAME11 'p'
#define IFNAME20 'l'
#define IFNAME21 'o'

struct netif sta_if, ap_if;
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

#if LWIP_IGMP
static err_t lwip_igmp_mac_filter( struct netif *netif, ip_addr_t *group, u8_t action );
#endif
static int32_t mt5931_low_level_output(struct netif *netif, struct pbuf *p);
static void mt5931_low_level_init1(struct netif *netif);
static void mt5931_low_level_init2(struct netif *netif);
static err_t mt5931_netif_init1(struct netif *netif);
static err_t mt5931_netif_init2(struct netif *netif);
static void
mt5931_ethernetif_init(ip4_addr_t *sta_ip_addr, ip4_addr_t *sta_net_mask, ip4_addr_t *sta_gw,
                             ip4_addr_t *ap_ip_addr, ip4_addr_t *ap_net_mask, ip4_addr_t *ap_gw,
                             uint8_t opmode);




#if LWIP_IGMP
/**
 * Interface between LwIP IGMP MAC filter and MAC filter
 */
static err_t lwip_igmp_mac_filter( struct netif *netif, ip_addr_t *group, u8_t action )
{
#if(WIFI_DRIVER_TYPE == WIFI_DRIVER_BCM)
    uint8 multicast_mac[6];

	//Multicast IP to MAC
	multicast_mac[0] = 0x01;
	multicast_mac[1] = 0x00;
	multicast_mac[2] = 0x5e;
	multicast_mac[3] = ((uint8*)group)[1] & 0x7F;
	multicast_mac[4] = ((uint8*)group)[2];
	multicast_mac[5] = ((uint8*)group)[3];

    switch ( action )
    {
        case IGMP_ADD_MAC_FILTER:
            if ( wlan_wifi_register_multicast_address( multicast_mac ) != 0 )
            {
                return ERR_VAL;
            }
            break;

        case IGMP_DEL_MAC_FILTER:
            if ( wlan_wifi_unregister_multicast_address( multicast_mac ) != 0 )
            {
                return ERR_VAL;
            }
            break;

        default:
            return ERR_VAL;
    }

    return ERR_OK;
#else
	return ERR_OK;
#endif
}
#endif


void
mt5931_low_level_input(struct netif *netif, struct pbuf *p)
{
    struct eth_hdr *ethhdr;
    //struct netif *tmpNetif;

    if (p == NULL) return;

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = (struct eth_hdr*)p->payload;

    if(netif == NULL)
    {
        // Received a packet for a network interface is not initialised
        // Cannot do anything with packet - just drop it.
        pbuf_free(p);
        p = NULL;
        LINK_STATS_INC(link.drop);
        return;
    }

    LINK_STATS_INC(link.recv);

    switch (htons(ethhdr->type)) {
    /* IP or ARP packet? */
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
        /* full packet send to tcpip_thread to process */
        if (tcpip_input(p, netif)!=ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));

            pbuf_free(p);
            p = NULL;
        }

        break;

    default:

        LINK_STATS_INC(link.proterr);
        pbuf_free(p);
        p = NULL;
        break;
    }
}


static int32_t
mt5931_low_level_output(struct netif *netif, struct pbuf *p)
{
    int cnt;
    (void) netif;   //unused parameter

    cnt = kalGetTxAvaliableFrameCount();

    if (cnt <= 8) {
        if (cnt > 4) {
            LINK_STATS_INC(link.err);
            vTaskDelay(1000);
            //MMPF_OS_Sleep(1);      // simple throttle
        }
        else {
            LINK_STATS_INC(link.opterr);
            while(1) {
                vTaskDelay(1000);
                //MMPF_OS_Sleep(1);  // complex throttle
                if ((kalGetTxAvaliableFrameCount() - cnt) >= 1) break;
            }
        }
    }

    wlan_tx_pkt(p);

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
mt5931_low_level_init1(struct netif *netif)
{
  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IGMP
  netif->flags |= NETIF_FLAG_IGMP;
  netif_set_igmp_mac_filter(netif, lwip_igmp_mac_filter);
#endif
  /* Do whatever else is needed to initialize interface. */
}

static void
mt5931_low_level_init2(struct netif *netif)
{

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
    netif_set_igmp_mac_filter(netif, lwip_igmp_mac_filter);
#endif
  /* Do whatever else is needed to initialize interface. */
}


/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t
mt5931_netif_init1(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  LOG_I(lwip, "=> %s\n", __FUNCTION__);

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

  LOG_I(lwip, "%s : malloc ok : %p\n", __FUNCTION__, ethernetif);
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME00;
  netif->name[1] = IFNAME01;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  #if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
  #endif /* LWIP_IPV6 */
  netif->linkoutput = (netif_linkoutput_fn)mt5931_low_level_output;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  mt5931_low_level_init1(netif);

#if 0
  /* Create a task that simulates an interrupt in a real system.  This will
  block waiting for packets, then send a message to the uIP task when data
  is available. */

  LOG_I(lwip, "Create RX task\n\r");
  xTaskCreate(InterruptSimulator, "RX", 400, (void *)netif, 3, NULL );
#endif
  return ERR_OK;
}

err_t
mt5931_netif_init2(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  LOG_I(lwip, "=> %s\n", __FUNCTION__);

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

  LOG_I(lwip, "%s : malloc ok : %p\n", __FUNCTION__, ethernetif);
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME10;
  netif->name[1] = IFNAME11;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  #if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
  #endif /* LWIP_IPV6 */
  netif->linkoutput = (netif_linkoutput_fn)mt5931_low_level_output;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  mt5931_low_level_init2(netif);

#if 0
  /* Create a task that simulates an interrupt in a real system.  This will
  block waiting for packets, then send a message to the uIP task when data
  is available. */

  LOG_I(lwip, "Create RX task\n\r");
  xTaskCreate(InterruptSimulator, "RX", 400, (void *)netif, 3, NULL );
#endif
  return ERR_OK;
}


static void
mt5931_ethernetif_init(ip4_addr_t *sta_ip_addr, ip4_addr_t *sta_net_mask, ip4_addr_t *sta_gw,
                             ip4_addr_t *ap_ip_addr, ip4_addr_t *ap_net_mask, ip4_addr_t *ap_gw,
                             uint8_t opmode)
{
    memset(&sta_if, 0, sizeof(sta_if));
    memset(&ap_if,  0, sizeof(ap_if));

    /* set MAC hardware address length */
    sta_if.hwaddr_len = ETHARP_HWADDR_LEN;
    //ap_if.hwaddr_len = ETHARP_HWADDR_LEN;
    /* set MAC hardware address */
    /* Setup the physical address of this IP instance.
    */
    wifi_config_get_mac_address(WIFI_PORT_STA, sta_if.hwaddr);

    //printf("[DBG]:["MACSTR"]",MAC2STR(sta_if.hwaddr));
    //wlan_get_mac_addr(sta_if.hwaddr);
    //WLAN_SYS_GetMacAddr(ap_if.hwaddr);

    netif_add(&sta_if, sta_ip_addr, sta_net_mask, sta_gw,
              NULL, mt5931_netif_init1, tcpip_input);
    netif_add(&ap_if, ap_ip_addr, ap_net_mask, ap_gw,
              NULL, mt5931_netif_init2, tcpip_input);

    netif_set_up(&sta_if);
    //netif_set_up(&ap_if);
    //if (opmode == WIFI_MODE_STA_ONLY) {
        netif_set_default(&sta_if);
    //}

    wlan_rx_register_callback(mt5931_low_level_input, &sta_if, &ap_if);
}


void lwip_tcpip_init(lwip_tcpip_config_t *tcpip_config, uint8_t opmode)
{
    mt5931_ethernetif_init(&tcpip_config->sta_addr,
                           &tcpip_config->sta_mask,
                           &tcpip_config->sta_gateway,
                           &tcpip_config->ap_addr,
                           &tcpip_config->ap_mask,
                           &tcpip_config->ap_gateway,
                           opmode);
}


int lwip_get_netif_name(netif_type_t netif_type, char *name)
{
    struct netif *netif = NULL;

    if (name == NULL) {
        return 0;
    }

    for (netif = netif_list; netif != NULL; netif = netif->next) {
        if (netif_type == NETIF_TYPE_AP &&
                IFNAME10 == netif->name[0] &&
                IFNAME11 == netif->name[1]) {
            name[0] = IFNAME10;
            name[1] = IFNAME11;
            name[2] = '0' + netif->num;
            return 1;
        } else if (netif_type == NETIF_TYPE_STA &&
                   IFNAME00 == netif->name[0] &&
                   IFNAME01 == netif->name[1]) {
            name[0] = IFNAME00;
            name[1] = IFNAME01;
            name[2] = '0' + netif->num;
            return 1;
        } else if (netif_type == NETIF_TYPE_LOOPBACK &&
                   IFNAME20 == netif->name[0] &&
                   IFNAME21 == netif->name[1]) {
            name[0] = IFNAME20;
            name[1] = IFNAME21;
            name[2] = '0' + netif->num;
            return 1;
        }
    }

    return 0;
}


struct netif *netif_find_by_type(netif_type_t netif_type)
{
    char name[4] = {0};

    if (lwip_get_netif_name(netif_type, (char *)name) == 1) {
        LOG_I(lwip, "name=%s\n\r", name);
        return netif_find(name);
    } else {
        return NULL;
    }
}

#endif
