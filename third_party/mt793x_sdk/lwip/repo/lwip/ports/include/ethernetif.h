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

#ifndef __ETHERNETIF_H
#define __ETHERNETIF_H


#include <stdint.h>

#include <lwip/pbuf.h>
#include "ethernet_filter.h"
#include "connsys_adapter.h"

typedef enum {
    STA_IP_MODE_STATIC   = 0,
    STA_IP_MODE_DHCP     = 1
} sta_ip_mode_t;

typedef enum {
    NETIF_TYPE_LOOPBACK = 0,
    NETIF_TYPE_AP       = 1,
    NETIF_TYPE_STA      = 2
} netif_type_t;

typedef struct {
    sta_ip_mode_t sta_ip_mode;
    ip4_addr_t sta_addr;
    ip4_addr_t sta_mask;
    ip4_addr_t sta_gateway;
    uint8_t sta_mac_addr[6];

    ip4_addr_t ap_addr;
    ip4_addr_t ap_mask;
    ip4_addr_t ap_gateway;
    uint8_t ap_mac_addr[6];
} tcpip_config_t;

typedef struct {
    int32_t ip_mode;

    ip4_addr_t sta_addr;
    ip4_addr_t sta_mask;
    ip4_addr_t sta_gateway;

    ip4_addr_t ap_addr;
    ip4_addr_t ap_mask;
    ip4_addr_t ap_gateway;
} lwip_tcpip_config_t;

/**
 * The task function called by net task. Used to simulate incoming packet
 * from SDIO interface port 0 or 1.
 */
void ethernetif_input_sim(uint32_t arg1, uint32_t arg2, uint32_t arg3);
PKT_HANDLE_RESULT_T lwip_deliver_tcpip(void* pkt, uint8_t *payload, uint32_t len, int inf);
void ethernetif_init_callback(void);
void register_eapol_rx_socket(int eapol_rx_socket);
void unregister_eapol_rx_socket(void);
void register_eapol_rx_socket_dual_intf(int eapol_rx_socket, int eapol_rx_socket_second);
void unregister_eapol_rx_socket_dual_intf(int eapol_rx_socket, int eapol_rx_socket_second);
int ethernet_raw_pkt_sender(unsigned char *buf, unsigned int len, struct netif *netif);

void low_level_set_mac_addr(struct netif *netif, uint8_t *mac_addr);
#if defined(MTK_BSP_LOOPBACK_ENABLE)
unsigned int loopback_start();
void loopback_start_set(unsigned int start);
#endif
void ethernetif_init(sta_ip_mode_t sta_ip_mode,
                     uint8_t *sta_mac_addr,
                     uint8_t *ap_mac_addr,
                     ip4_addr_t *sta_ip_addr, ip4_addr_t *sta_net_mask, ip4_addr_t *sta_gw,
                     ip4_addr_t *ap_ip_addr, ip4_addr_t *ap_net_mask, ip4_addr_t *ap_gw,
                     uint8_t opmode);

void tcpip_stack_init(tcpip_config_t *tcpip_config, uint8_t opmode);

void lwip_tcpip_init(lwip_tcpip_config_t *tcpip_config, uint8_t opmode);

int lwip_get_netif_name(netif_type_t netif_type, char *name);

struct netif *netif_find_by_type(netif_type_t netif_type);

#endif /* __ETHERNETIF_H */

