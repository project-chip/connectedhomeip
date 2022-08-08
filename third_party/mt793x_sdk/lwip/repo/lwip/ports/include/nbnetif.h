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

#ifndef __NBNETIF_H__
#define __NBNETIF_H__

#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
#include "lwip/def.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "lwip/ip4_addr.h"
#include "queue.h"
#if LWIP_IPV6
#include "lwip/ip6_addr.h"
#include "lwip/nd6.h"
#endif

#ifndef PS_DATA_DEBUG
//#define PS_DATA_DEBUG
#endif

//#define PS_DATA_UT

typedef enum {
    MSG_ID_BEARER_INFO_ADD,
    MSG_ID_BEARER_INFO_DEL,
    MSG_ID_RESUME_CHID_ADD,
    MSG_ID_RESUME_CHID_ID,
} nb_netif_msg_id_t;


typedef struct {
    nb_netif_msg_id_t msg_id;
    void              *msg_data;
} nb_netif_msg_t;

typedef struct _nbiot_bearer_info_struct
{
    u8_t is_activated; //activated (1) and deactivated(0)
    u8_t cid;
    u8_t type; //ipv4(0) or ipv6(1)
    u32_t mtu;
    u32_t channel_id;
    unsigned char ip_addr[64];
    unsigned char pri_dns[64];
    unsigned char snd_dns[64];
    //unsigned char ipv6_addr[16];
    //unsigned char pri_dnsv6[16];
    //unsigned char snd_dnsv6[16];
} nbiot_bearer_info_struct;

typedef struct nb_netif_cid_list{
    u8_t cid;
    u32_t channel_id;
    struct nb_netif_cid_list *next;
} nb_netif_cid_list_t;

typedef struct nb_resume_chid_list{
    u32_t channel_id;
    struct nb_resume_chid_list *next;
} nb_resume_chid_list_t;

typedef struct {
    u8_t cid;
    u32_t channel_id;
} nb_net_info_del_t;

typedef struct {
    u8_t cid;
    u8_t type; //ipv4(0) or ipv6(1)
#if LWIP_IPV6
    u8_t ip6addr[16];
#endif
    u32_t channel_id;
    u32_t mtu;
    ip4_addr_t ipaddr;
} nb_net_info_add_t;

typedef struct {
    QueueHandle_t qid;
    u8_t need_delay;
    nb_netif_cid_list_t *cid_list;
    nb_resume_chid_list_t *chid_list;
} nb_netif_context_struct;

int nb_netif_bearer_info_ind(nbiot_bearer_info_struct *netinfo);

int nb_resume_channel_id(void);

void nb_netif_init(void);

#endif

#endif /* __NBNETIF_H__ */

