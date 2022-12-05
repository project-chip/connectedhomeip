/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WF200_WIFI
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#endif

#include "wfx_host_events.h"
#include "wifi_config.h"
#ifdef WF200_WIFI
#include "sl_wfx.h"
#endif
/* LwIP includes. */
#include "ethernetif.h"
#include "lwip/ethip6.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#ifndef SILABS_LOG
extern "C" {
void efr32Log(const char * aFormat, ...);
#define SILABS_LOG(...) efr32Log(__VA_ARGS__);
}
#endif

StaticSemaphore_t xEthernetIfSemaBuffer;

/*****************************************************************************
 * Defines
 ******************************************************************************/
#define STATION_NETIF0 's'
#define STATION_NETIF1 't'

#define LWIP_FRAME_ALIGNMENT 60

uint32_t gOverrunCount = 0;

/*****************************************************************************
 * Variables
 ******************************************************************************/

/*****************************************************************************
 * @fn static void low_level_init(struct netif *netif)
 * @brief
 *    Initializes the hardware parameters. Called from ethernetif_init().
 *
 * @param[in] netif: the already initialized lwip network interface structure
 *
 * @return
 *    None
 ******************************************************************************/
static void low_level_init(struct netif * netif)
{
    /* set netif MAC hardware address length */
    netif->hwaddr_len = ETH_HWADDR_LEN;

    /* Set netif MAC hardware address */
    sl_wfx_mac_address_t mac_addr;

    wfx_get_wifi_mac_addr(SL_WFX_STA_INTERFACE, &mac_addr);

    netif->hwaddr[0] = mac_addr.octet[0];
    netif->hwaddr[1] = mac_addr.octet[1];
    netif->hwaddr[2] = mac_addr.octet[2];
    netif->hwaddr[3] = mac_addr.octet[3];
    netif->hwaddr[4] = mac_addr.octet[4];
    netif->hwaddr[5] = mac_addr.octet[5];

    /* Set netif maximum transfer unit */
    netif->mtu = 1500;

    /* Accept broadcast address and ARP traffic */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

#if LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif /* LWIP_IPV6_MLD */
}

/********************************************************************************
 * @fn  static void low_level_input(struct netif *netif, uint8_t *b, uint16_t len)
 * @brief
 * Make PBUF out of a linear buffer - that can be fed into lwip
 * @param[in]  netif: the already initialized lwip network interface structure
 * @param[in]  len: length
 * @return
 *     None
 ************************************************************************************/
static void low_level_input(struct netif * netif, uint8_t * b, uint16_t len)
{
    struct pbuf *p, *q;
    uint32_t bufferoffset;

    if (len <= 0)
    {
        return;
    }
    if (len < LWIP_FRAME_ALIGNMENT)
    { /* 60 : LWIP frame alignment */
        len = LWIP_FRAME_ALIGNMENT;
    }

    /* Drop packets originated from the same interface and is not destined for the said interface */
    const uint8_t * src_mac = b + netif->hwaddr_len;
    const uint8_t * dst_mac = b;

    if (!(ip6_addr_ispreferred(netif_ip6_addr_state(netif, 0))) && (memcmp(netif->hwaddr, src_mac, netif->hwaddr_len) == 0) &&
        (memcmp(netif->hwaddr, dst_mac, netif->hwaddr_len) != 0))
    {
#ifdef WIFI_DEBUG_ENABLED
        SILABS_LOG("%s: DROP, [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x", __func__,

                   dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],

                   src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],

                   b[12], b[13]);
#endif
        return;
    }

    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool
     * and copy the data to the pbuf chain
     */
    if ((p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL)) != STRUCT_PBUF)
    {
        for (q = p, bufferoffset = 0; q != NULL; q = q->next)
        {
            memcpy((uint8_t *) q->payload, (uint8_t *) b + bufferoffset, q->len);
            bufferoffset += q->len;
        }
#ifdef WIFI_DEBUG_ENABLED
        SILABS_LOG("%s: ACCEPT %d, [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x", __func__,
                   bufferoffset,

                   dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],

                   src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],

                   b[12], b[13]);
#endif

        if (netif->input(p, netif) != ERR_OK)
        {
            gOverrunCount++;
            SILABS_LOG("overrun count entering when fail to alloc value %d", gOverrunCount);
            pbuf_free(p);
        }
    }
    else
    {
        gOverrunCount++;
        SILABS_LOG("overrun count entering when fail to alloc value %d", gOverrunCount);
    }
}

static SemaphoreHandle_t ethout_sem;
/*****************************************************************************
 *  @fn  static err_t low_level_output(struct netif *netif, struct pbuf *p)
 *  @brief
 *    This function is called from LWIP task when LWIP stack
 *    has some data to be forwarded over WiFi Network
 *
 * @param[in] netif: lwip network interface
 *
 * @param[in] p: the packet to send
 *
 * @return
 *    ERR_OK if successful
 ******************************************************************************/
static err_t low_level_output(struct netif * netif, struct pbuf * p)
{
    void * rsipkt;
    struct pbuf * q;
    uint16_t framelength;

    if (xSemaphoreTake(ethout_sem, portMAX_DELAY) != pdTRUE)
    {
        return ERR_IF;
    }
#ifdef WIFI_DEBUG_ENABLED
    SILABS_LOG("EN-RSI: Output");
#endif
    if ((netif->flags & (NETIF_FLAG_LINK_UP | NETIF_FLAG_UP)) != (NETIF_FLAG_LINK_UP | NETIF_FLAG_UP))
    {
        SILABS_LOG("EN-RSI:NOT UP");
        xSemaphoreGive(ethout_sem);
        return ERR_IF;
    }
    /* Confirm if packet is allocated */
    rsipkt = wfx_rsi_alloc_pkt();
    if (!rsipkt)
    {
        SILABS_LOG("EN-RSI:No buf");
        xSemaphoreGive(ethout_sem);
        return ERR_IF;
    }

#ifdef WIFI_DEBUG_ENABLED
    uint8_t * b = (uint8_t *) p->payload;
    SILABS_LOG("EN-RSI: Out [%02x:%02x:%02x:%02x:%02x:%02x][%02x:%02x:%02x:%02x:%02x:%02x]type=%02x%02x", b[0], b[1], b[2], b[3],
               b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13]);
#endif
    /* Generate the packet */
    for (q = p, framelength = 0; q != NULL; q = q->next)
    {
        wfx_rsi_pkt_add_data(rsipkt, (uint8_t *) (q->payload), (uint16_t) q->len, framelength);
        framelength += q->len;
    }
    if (framelength < LWIP_FRAME_ALIGNMENT)
    {
        /* Add junk data to the end for frame alignment if framelength is less than 60 */
        wfx_rsi_pkt_add_data(rsipkt, (uint8_t *) (p->payload), LWIP_FRAME_ALIGNMENT - framelength, framelength);
    }
#ifdef WIFI_DEBUG_ENABLED
    SILABS_LOG("EN-RSI: Sending %d", framelength);
#endif

    /* forward the generated packet to RSI to
     * send the data over wifi network
     */
    if (wfx_rsi_send_data(rsipkt, framelength))
    {
        SILABS_LOG("*ERR*EN-RSI:Send fail");
        xSemaphoreGive(ethout_sem);
        return ERR_IF;
    }

#ifdef WIFI_DEBUG_ENABLED
    SILABS_LOG("EN-RSI:Xmit %d", framelength);
#endif
    xSemaphoreGive(ethout_sem);

    return ERR_OK;
}

/*****************************************************************************
 *  @fn  void wfx_host_received_sta_frame_cb(uint8_t *buf, int len)
 *  @brief
 *    host received frame cb
 *
 * @param[in] buf: buffer
 *
 * @param[in] len: length
 *
 * @return
 *    None
 ******************************************************************************/
void wfx_host_received_sta_frame_cb(uint8_t * buf, int len)
{
    struct netif * ifp;

    /* get the network interface for STATION interface,
     * and forward the received frame buffer to LWIP
     */
    if ((ifp = wfx_get_netif(SL_WFX_STA_INTERFACE)) != (struct netif *) 0)
    {
        low_level_input(ifp, buf, len);
    }
}

/*****************************************************************************
 *  @fn  err_t sta_ethernetif_init(struct netif *netif)
 *  @brief
 *    sta ethernet if initialization
 *
 * @param[in] netif: the lwip network interface structure
 *
 * @return
 *    ERR_OK if successful
 ******************************************************************************/
err_t sta_ethernetif_init(struct netif * netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    /* Set the netif name to identify the interface */
    netif->name[0] = STATION_NETIF0;
    netif->name[1] = STATION_NETIF1;

#if LWIP_IPV4 && LWIP_ARP
    netif->output = etharp_output;
#endif /* #if LWIP_IPV4 && LWIP_ARP */
#if LWIP_IPV6 && LWIP_ETHERNET
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 && LWIP_ETHERNET */
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    /* Need single output only */
    ethout_sem = xSemaphoreCreateBinaryStatic(&xEthernetIfSemaBuffer);
    xSemaphoreGive(ethout_sem);

    return ERR_OK;
}
