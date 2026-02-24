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

// LwIP includes - Includes must be before the rsi headers due to redefination errors
#include "lwip/ethip6.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "sl_wfx.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/lwip-support/ethernetif.h>
#include <platform/silabs/wifi/lwip-support/lwip_netif.h>

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
    chip::MutableByteSpan byteSpan(netif->hwaddr, ETH_HWADDR_LEN);
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::Silabs::WifiInterface::GetInstance().GetMacAddress(SL_WFX_STA_INTERFACE, byteSpan);

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
#if WIFI_DEBUG_ENABLED
        ChipLogProgress(DeviceLayer,
                        "lwip_input: DROP, [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x",

                        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],

                        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],

                        b[12], b[13]);
#endif
        return;
    }

    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool
     * and copy the data to the pbuf chain
     */
    if ((p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL)) != NULL)
    {
        for (q = p, bufferoffset = 0; q != NULL; q = q->next)
        {
            memcpy((uint8_t *) q->payload, (uint8_t *) b + bufferoffset, q->len);
            bufferoffset += q->len;
        }
#if WIFI_DEBUG_ENABLED
        ChipLogProgress(DeviceLayer,
                        "lwip_input: ACCEPT %ld, [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x",
                        bufferoffset,

                        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],

                        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],

                        b[12], b[13]);
#endif

        if (netif->input(p, netif) != ERR_OK)
        {
            gOverrunCount++;
            ChipLogProgress(DeviceLayer, "overrun count entering when fail to alloc value %ld", gOverrunCount);
            pbuf_free(p);
        }
    }
    else
    {
        gOverrunCount++;
        ChipLogProgress(DeviceLayer, "overrun count entering when fail to alloc value %ld", gOverrunCount);
    }
}

/*****************************************************************************
 *  @fn  static err_t low_level_output(struct netif *netif, struct pbuf *p)
 *  @brief
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
static err_t low_level_output(struct netif * netif, struct pbuf * p)
{
    struct pbuf * q;
    sl_wfx_send_frame_req_t * tx_buffer;
    uint8_t * buffer;
    uint32_t framelength, asize;
    uint32_t bufferoffset;
    uint32_t padding;
    sl_status_t result;

    for (q = p, framelength = 0; q != NULL; q = q->next)
    {
        framelength += q->len;
    }
    if (framelength < LWIP_FRAME_ALIGNMENT)
    { /* 60 : Frame alignment for LWIP */
        padding = LWIP_FRAME_ALIGNMENT - framelength;
    }
    else
    {
        padding = 0;
    }

    /* choose padding of 64 */
    asize = SL_WFX_ROUND_UP(framelength + padding, 64) + sizeof(sl_wfx_send_frame_req_t);
    // 12 is size of other data in buffer struct, user shouldn't have to care about this?
    if (sl_wfx_host_allocate_buffer((void **) &tx_buffer, SL_WFX_TX_FRAME_BUFFER, asize) != SL_STATUS_OK)
    {
        ChipLogProgress(DeviceLayer, "*ERR*EN-Out: No mem frame len=%ld", framelength);
        gOverrunCount++;
        ChipLogProgress(DeviceLayer, "overrun count exiting when faied to alloc value %ld", gOverrunCount);
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
    if (padding)
    {
        memset(buffer + bufferoffset, 0, padding);
        framelength += padding;
    }
    /* transmit */
    int i  = 0;
    result = SL_STATUS_FAIL;

#if WIFI_DEBUG_ENABLED
    ChipLogProgress(DeviceLayer, "WF200: Out %d", (int) framelength);
#endif

    /* send the generated frame over Wifi network */
    while ((result != SL_STATUS_OK) && (i++ < 10))
    {
        result = sl_wfx_send_ethernet_frame(tx_buffer, framelength, SL_WFX_STA_INTERFACE, 0 /* priority */);
    }
    sl_wfx_host_free_buffer(tx_buffer, SL_WFX_TX_FRAME_BUFFER);

    if (result != SL_STATUS_OK)
    {
        ChipLogProgress(DeviceLayer, "*ERR*Send enet %d", (int) framelength);
        return ERR_IF;
    }
    return ERR_OK;
}

/*****************************************************************************
 * @fn   void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t *rx_buffer)
 * @brief
 *    This function implements the wf200 received frame callback.
 *    Called from the context of the bus_task (not ISR)
 *
 * @param[in] rx_buffer: the ethernet frame received by the wf200
 *
 * @return
 *    None
 ******************************************************************************/
void sl_wfx_host_received_frame_callback(sl_wfx_received_ind_t * rx_buffer)
{
    struct netif * netif;

    /* Check packet interface to send to AP or STA interface */
    if ((rx_buffer->header.info & SL_WFX_MSG_INFO_INTERFACE_MASK) == (SL_WFX_STA_INTERFACE << SL_WFX_MSG_INFO_INTERFACE_OFFSET))
    {

        /* Send received frame to station interface */
        if ((netif = chip::DeviceLayer::Silabs::Lwip::GetNetworkInterface(SL_WFX_STA_INTERFACE)) != NULL)
        {
            uint8_t * buffer;
            uint16_t len;

            len    = rx_buffer->body.frame_length;
            buffer = (uint8_t *) &(rx_buffer->body.frame[rx_buffer->body.frame_padding]);

#if WIFI_DEBUG_ENABLED
            ChipLogProgress(DeviceLayer, "WF200: In %d", (int) len);
#endif

            low_level_input(netif, buffer, len);
        }
        else
        {
#if WIFI_DEBUG_ENABLED
            ChipLogProgress(DeviceLayer, "WF200: NO-INTF");
#endif
        }
    }
    else
    {
#if WIFI_DEBUG_ENABLED
        ChipLogProgress(DeviceLayer, "WF200: Invalid frame IN");
#endif
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

    return ERR_OK;
}
