/****************************************************************************
 * Copyright 2018, Silicon Laboratories Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"


#ifndef WF200_WIFI
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#endif

#include "wifi_config.h"
#include "wfx_host_events.h"
#ifdef WF200_WIFI
#include "sl_wfx.h"
#endif
/* LwIP includes. */
#include "ethernetif.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "lwip/ethip6.h"

#ifndef EFR32_LOG
extern "C" {
void efr32Log(const char * aFormat, ...);
#define EFR32_LOG(...) efr32Log(__VA_ARGS__);
}
#endif


/*****************************************************************************
 * Defines
 ******************************************************************************/
#define STATION_NETIF0 's'
#define STATION_NETIF1 't'

/*****************************************************************************
 * Variables
 ******************************************************************************/

/*****************************************************************************
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
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP| NETIF_FLAG_IGMP;

    /* Set netif link flag */
//    netif->flags |= NETIF_FLAG_LINK_UP|NETIF_FLAG_UP;
}

/*
 * Make PBUF out of a linear buffer - that can be fed into lwip
 */
static void
low_level_input (struct netif * netif, uint8_t *b, uint16_t len)
{
        struct pbuf *p, *q;
        uint32_t bufferoffset;

        if (len <= 0)
                return;
        if (len < 60) len = 60;
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool
         * and copy the data to the pbuf chain
         */
        if ((p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL)) != (struct pbuf *) 0) {
                for (q = p, bufferoffset = 0; q != NULL; q = q->next) {
                        memcpy((uint8_t *) q->payload, (uint8_t *) b + bufferoffset, q->len);
                        bufferoffset += q->len;
                        // ASSERT(bufferoffset <= len);
                }
#if 0
                EFR32_LOG ("EN:IN %d,[%02x:%02x:%02x:%02x:%02x%02x][%02x:%02x:%02x:%02x:%02x:%02x]type=%02x%02x", bufferoffset,
                           b [0], b [1], b [2], b [3], b [4], b [5],
                           b [6], b [7], b [8], b [9], b [10], b [11],
                           b[12], b [13]);
#endif
                if (netif->input (p, netif) != ERR_OK) {
                        pbuf_free (p);
                }
        }
}

/*****************************************************************************
 * @brief
 *    This function should does the actual transmission of the packet(s).
 *    The packet is contained in the pbuf that is passed to the function.
 *    This pbuf might be chained.
 *
 * @param[in] netif: the lwip network interface structure
 *
 * @param[in] p: the packet to send
 *
 * @return
 *    ERR_OK if successful
 ******************************************************************************/
#ifdef WF200_WIFI
static err_t
low_level_output(struct netif * netif, struct pbuf * p)
{
    struct pbuf * q;
    sl_wfx_send_frame_req_t *tx_buffer;
    uint8_t * buffer;
    uint32_t framelength,asize;
    uint32_t bufferoffset;
    uint32_t padding;
    sl_status_t result;

    for (q = p, framelength = 0; q != NULL; q = q->next)
    {
        framelength += q->len;
    }
    if (framelength < 60)
    {
        padding = 60 - framelength;
    }
    else
    {
        padding = 0;
    }

    asize = SL_WFX_ROUND_UP(framelength + padding, 64) + sizeof(sl_wfx_send_frame_req_t);
    // 12 is size of other data in buffer struct, user shouldn't have to care about this?
    if (sl_wfx_host_allocate_buffer((void **)&tx_buffer, SL_WFX_TX_FRAME_BUFFER, asize)
        != SL_STATUS_OK)
    {
        EFR32_LOG ("*ERR*EN-Out: No mem frame len=%d", framelength);
        return ERR_MEM;
    }
    buffer = tx_buffer->body.packet_data;
    /* copy frame from pbufs to driver buffers */
    for (q = p, bufferoffset = 0; q != NULL; q = q->next)
    {
        /* Get bytes in current lwIP buffer */
        memcpy((uint8_t *) ((uint8_t *) buffer + bufferoffset), (uint8_t *) ((uint8_t *) q->payload), q->len);
        bufferoffset += q->len;
    }
    /* No requirement to do this - but we should for security */
    if (padding) {
        memset (buffer + bufferoffset, 0, padding);
        framelength += padding;
    }
    /* transmit */
    int i  = 0;
    result = SL_STATUS_FAIL;

    //EFR32_LOG ("WF200: Out %d", (int)framelength);
    while ((result != SL_STATUS_OK) && (i++ < 10))
    {
        result = sl_wfx_send_ethernet_frame(tx_buffer, framelength, SL_WFX_STA_INTERFACE, 0);
    }
    sl_wfx_host_free_buffer(tx_buffer, SL_WFX_TX_FRAME_BUFFER);

    if (result != SL_STATUS_OK)
    {
        EFR32_LOG ("*ERR*Send enet %d",(int)framelength);
        return ERR_IF;
    }
    return ERR_OK;
}


/*****************************************************************************
 * @brief
 *    This function implements the wf200 received frame callback.
 *    Called from the context of the bus_task (not ISR)
 *
 * @param[in] rx_buffer: the ethernet frame received by the wf200
 *
 * @return
 *    None
 ******************************************************************************/
void
sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t * rx_buffer)
{
    struct netif * netif;

    /* Check packet interface to send to AP or STA interface */
    if ((rx_buffer->header.info & SL_WFX_MSG_INFO_INTERFACE_MASK) ==
        (SL_WFX_STA_INTERFACE << SL_WFX_MSG_INFO_INTERFACE_OFFSET))
    {
        /* Send to station interface */
        if ((netif = wfx_get_netif (SL_WFX_STA_INTERFACE)) != NULL) {
            uint8_t *buffer;
            uint16_t len;

            len = rx_buffer->body.frame_length;
            buffer = (uint8_t *) &(rx_buffer->body.frame [rx_buffer->body.frame_padding]);
            //EFR32_LOG ("WF200: In %d", (int)len);
            low_level_input (netif, buffer, len);
        } else {
            //EFR32_LOG ("WF200: NO-INTF");
        }
    } else {
        //EFR32_LOG ("WF200: Invalid frame IN");
    }
}
#else /* For RS911x - using LWIP */
static SemaphoreHandle_t ethout_sem;
static err_t
low_level_output(struct netif * netif, struct pbuf * p)
{
        void *rsipkt;
        struct pbuf * q;
        uint16_t framelength;

        if (xSemaphoreTake(ethout_sem, portMAX_DELAY) != pdTRUE) {
                return ERR_IF;
        }
        //EFR32_LOG ("EN-RSI: Output");
        if ((netif->flags & (NETIF_FLAG_LINK_UP|NETIF_FLAG_UP)) != (NETIF_FLAG_LINK_UP|NETIF_FLAG_UP)) {
                EFR32_LOG ("EN-RSI:NOT UP");
                xSemaphoreGive(ethout_sem);
                return ERR_IF;
        }
//    netif->flags |= NETIF_FLAG_LINK_UP|NETIF_FLAG_UP;
        rsipkt = wfx_rsi_alloc_pkt ();
        if (!rsipkt) {
                EFR32_LOG ("EN-RSI:No buf");
                xSemaphoreGive(ethout_sem);
                return ERR_IF;
        }

#if 0
        uint8_t *b = (uint8_t *)p->payload;
        EFR32_LOG ("EN-RSI: Out [%02x:%02x:%02x:%02x:%02x:%02x][%02x:%02x:%02x:%02x:%02x:%02x]type=%02x%02x",
                           b [0], b [1], b [2], b [3], b [4], b [5],
                           b [6], b [7], b [8], b [9], b [10], b [11],
                           b[12], b [13]);
#endif
        for (q = p, framelength = 0; q != NULL; q = q->next) {
                wfx_rsi_pkt_add_data (rsipkt, (uint8_t *) (q->payload), (uint16_t)q->len, framelength);
                framelength += q->len;
        }
        if (framelength < 60) {
                /* Add junk data to the end */
                wfx_rsi_pkt_add_data (rsipkt, (uint8_t *)(p->payload),60-framelength, framelength);
        }
        //EFR32_LOG ("EN-RSI: Sending %d", framelength);
        if (wfx_rsi_send_data (rsipkt, framelength)) {
                EFR32_LOG ("*ERR*EN-RSI:Send fail");
                xSemaphoreGive(ethout_sem);
                return ERR_IF;
        }

        //EFR32_LOG ("EN-RSI:Xmit %d", framelength);
        xSemaphoreGive (ethout_sem);

        return ERR_OK;
}
void
wfx_host_received_sta_frame_cb (uint8_t *buf, int len)
{
        struct netif *ifp;

        if ((ifp = wfx_get_netif (SL_WFX_STA_INTERFACE)) != (struct netif *)0) {
                low_level_input (ifp, buf, len);
        }
}

#endif /* RS911x - with LWIP */

err_t
sta_ethernetif_init(struct netif * netif)
{
        LWIP_ASSERT("netif != NULL", (netif != NULL));

        /* Set the netif name to identify the interface */
        netif->name[0] = STATION_NETIF0;
        netif->name[1] = STATION_NETIF1;

        netif->output     = etharp_output;
        netif->output_ip6 = ethip6_output;
        netif->linkoutput = low_level_output;

        /* initialize the hardware */
        low_level_init (netif);
        //wfx_SetStationNetif (netif); /* I think netif already points to station */
#ifndef WF200_WIFI
        /* Need single output only */
        ethout_sem = xSemaphoreCreateBinary();
        xSemaphoreGive(ethout_sem);
#endif
        return ERR_OK;
}
