/* Copyright Statement:
 *
 * (C) 2022  MediaTek Inc. All rights reserved.
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

#include "lwip/apps/port_monitor.h"
#include "lwip/apps/port_monitor_priv.h"
#include "memory_attribute.h"

#if LWIP_TCP


#define PORT_MONITOR_DEBUG_TRACE        (PORT_MONITOR_DEBUG | LWIP_DBG_TRACE)
#define PORT_MONITOR_DEBUG_WARN         (PORT_MONITOR_DEBUG | LWIP_DBG_LEVEL_WARNING)

static const size_t port_monitor_packet_struct_size = sizeof( port_monitor_packet_t );

/* context of port monitor */
ATTR_ZIDATA_IN_SYSRAM port_monitor_context_t port_monitor_cntx;

/** Check if a port in blocked list
 *
 * @param block_port the port to be checked
 * @return 0 not in list, 1 in list
 */
int port_monitor_check_port_in_list(u16_t block_port)
{
    port_monitor_port_t *cur_port = port_monitor_cntx.port_list;

    while (cur_port) {
        if (cur_port->port == block_port) {
            LWIP_DEBUGF(PORT_MONITOR_DEBUG_TRACE, ("port %d found\r\n", block_port));
            return 1;
        }
        cur_port = cur_port->next;
    }
    
    LWIP_DEBUGF(PORT_MONITOR_DEBUG_TRACE, ("port %d not found\r\n", block_port));
    return 0;
}


/** Add a new port to be blocked
 *
 * @param block_port the port to be blocked
 * @return 0 succeeded, -1 failed
 */
int port_monitor_config_insert_port(u16_t block_port)
{
    port_monitor_port_t *cur_port = port_monitor_cntx.port_list;
    port_monitor_port_t *new_port = NULL;

    if (port_monitor_cntx.port_count >= PORT_MONITOR_BLOCKED_PORTS_NUM) {
        LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("port list full\r\n"));
        return -1;
    }
    
    while (cur_port) {
        if (cur_port->port == block_port) {
            LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("port %d exist\r\n", block_port));
            return 0;
        }
        if (cur_port->next) {
            cur_port = cur_port->next;
        }
        else {
            break;
        }
    }
        
    new_port = (port_monitor_port_t *)pvPortMalloc(sizeof(port_monitor_port_t));
    if (new_port) {
        new_port->next = NULL;
        new_port->port = block_port;
        if (cur_port) {
            cur_port->next = new_port;
        }
        else {
            port_monitor_cntx.port_list = new_port;
        }
        port_monitor_cntx.port_count++;
        LWIP_DEBUGF(PORT_MONITOR_DEBUG_TRACE, ("[%d]insert port %d\r\n", port_monitor_cntx.port_count, block_port));
        return 0;
    }
    else {
        LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("insert port %d fail\r\n", block_port));
        return -1;
    }
    
}

/** Remove a port from blocked port list
 *
 * @param block_port the port to be removed
 * @return 0 succeeded, -1 failed
 */
int port_monitor_config_remove_port(u16_t block_port)
{
    port_monitor_port_t *cur_port = port_monitor_cntx.port_list;
    port_monitor_port_t *pre_port = NULL;

    while (cur_port) {
        if (cur_port->port == block_port) {
            if (pre_port) {
                pre_port->next = cur_port->next;
            }
            else {
                port_monitor_cntx.port_list = cur_port->next;
            }
            vPortFree((void *)cur_port);
            port_monitor_cntx.port_count--;
            LWIP_DEBUGF(PORT_MONITOR_DEBUG_TRACE, ("[%d]port %d removed\r\n", port_monitor_cntx.port_count, block_port));
            return 0;
        }
        pre_port = cur_port;
        cur_port = cur_port->next;
    }

    LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("port %d not found\r\n", block_port));
    return -1;
    
}

/** Enable/disable logging packets
 *
 * @param logging_enable 1 enable, 0 disable
 */
void port_monitor_config_logging_enable(u8_t logging_enable)
{
    port_monitor_cntx.logging_enable = logging_enable;
}

/** Get port list
 * 
 * @return the pointer of port list
 */
port_monitor_port_t * port_monitor_get_port_list(void)
{
    return port_monitor_cntx.port_list;
}

/** Get saved packets
 * 
 * @return the header of packets list
 */
port_monitor_packet_t * port_monitor_get_logging_packets(void)
{
    return port_monitor_cntx.pkt_list;
}

/** clear saved packets
 * 
 */
void port_monitor_clear_logging_packets(void)
{
    port_monitor_packet_t *cur_pkt = port_monitor_cntx.pkt_list;
    port_monitor_packet_t *next_pkt;

    while (cur_pkt)
    {
        next_pkt = cur_pkt->next;
        vPortFree((void *)cur_pkt);
        cur_pkt = next_pkt;
    }
    port_monitor_cntx.pkt_list = NULL;
    port_monitor_cntx.pkt_count = 0;
}

/** Insert a new packet to list
 *
 * @param new_pkt the packet to be inserted
 * @return 0 succeeded, -1 failed
 */
static int port_monitor_insert_packet(port_monitor_packet_t *new_pkt)
{
    port_monitor_packet_t *cur_pkt = port_monitor_cntx.pkt_list;

    if (port_monitor_cntx.pkt_count >= PORT_MONITOR_LOGGING_PKTS_NUM) {
        port_monitor_cntx.pkt_list = cur_pkt->next;
        vPortFree((void *)cur_pkt);
        cur_pkt = port_monitor_cntx.pkt_list;
        port_monitor_cntx.pkt_count--;
    }
    
    while (cur_pkt) {
        if (cur_pkt->next) {
            cur_pkt = cur_pkt->next;
        }
        else {
            break;
        }
    }
        
    if (cur_pkt) {
        cur_pkt->next = new_pkt;
    }
    else {
        port_monitor_cntx.pkt_list = new_pkt;
    }
    
    port_monitor_cntx.pkt_count++;
    
    return 0;
    
}


/** The input processing of port monitor.
 * It verifies the dest port
 * @param port the dest port of the packet
 * @param is_syn the SYN flag of the packet
 * @param p received segment to process
 * @return 1 this packet is eaten by port monitor, 0 ignored by port monitor
 */
int port_monitor_packet_input(u16_t port, u8_t is_syn, struct pbuf *p)
{
    size_t offset_to = 0, len = 0;
    struct pbuf *p_from = p;
    port_monitor_packet_t *cur_pkt = NULL;
    
    if (port_monitor_check_port_in_list(port) == 0) {
        return 0;
    }

    if (port_monitor_cntx.logging_enable == 0) {
        return 1;
    }

    if ((p != NULL) && (p->tot_len != 0)) {
        len = (size_t)p->tot_len;
    }

    cur_pkt = (port_monitor_packet_t *)pvPortMalloc(port_monitor_packet_struct_size + len);
    if (cur_pkt) {
        cur_pkt->next = NULL;
        cur_pkt->dest_port = port;
        cur_pkt->syn_flag = is_syn;
        cur_pkt->len = len;
        cur_pkt->payload = (void *)cur_pkt + port_monitor_packet_struct_size;
        if (len) {
            do {
                memcpy((u8_t *)cur_pkt->payload + offset_to, (u8_t *)p_from->payload, p_from->len);
                offset_to += p_from->len;
                p_from = p_from->next;
            }while(p_from);
        }
        if (port_monitor_insert_packet(cur_pkt) != 0) {
            vPortFree((void *)cur_pkt);
            LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("insert pkt fail\n"));
        }
    }
    else {
        LWIP_DEBUGF(PORT_MONITOR_DEBUG_WARN, ("malloc pkt fail %d\n", len));
    }

    return 1;
}

#endif /* LWIP_TCP */
