/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright (C) 2021 MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef LWIP_HDR_IP4_NAPT_FORWARD_H
#define LWIP_HDR_IP4_NAPT_FORWARD_H

#include "lwip/opt.h"
#include "lwip/def.h"

#if LWIP_IPV4

#ifdef __cplusplus
extern "C" {
#endif

#define IP4_NAPT_PORT_START 49152

#define IP4_NAPT_MAX_ICMP_ENTRY 8

#define WIFI_PROFILE_BUFFER_LEN 128

extern uint32_t g_ip4_napt_clean_entry_timer;

typedef struct ip4_napt_bind
{
  struct netif *in_if;
  struct netif *out_if;
} ip4_napt_bind_t;

typedef struct ip4_napt_ip
{
  ip4_addr_t source;
  ip4_addr_t destination;
} ip4_napt_ip_t;

typedef struct ip4_napt_port
{
  u16_t sport;
  u16_t nport;
  u16_t dport;
} ip4_napt_port_t;

typedef struct ip4_napt_icmp_entry
{
  struct ip4_napt_icmp_entry *next;
  ip4_napt_ip_t ip;
  u16_t id;
  u16_t seqno;
} ip4_napt_icmp_entry_t;

typedef struct ip4_napt_tcpudp_entry
{
  struct ip4_napt_tcpudp_entry *next;
  ip4_napt_ip_t ip;
  ip4_napt_port_t port;
  u32_t timestamp;
} ip4_napt_tcpudp_entry_t;

typedef struct ip4_napt_icmp_list
{
  ip4_napt_icmp_entry_t *free_list;
  ip4_napt_icmp_entry_t *used_list;
} ip4_napt_icmp_list_t;

typedef struct ip4_napt_tcpudp_list
{
  ip4_napt_tcpudp_entry_t *free_list;
  ip4_napt_tcpudp_entry_t *used_list;
} ip4_napt_tcpudp_list_t;

typedef struct ip4_napt_talbe
{
  ip4_napt_icmp_list_t *icmp;
  ip4_napt_tcpudp_list_t *tcp;
  ip4_napt_tcpudp_list_t *udp;
} ip4_napt_table_t;
void ip4_napt_flush_timer_enable(void);
void ip4_napt_flush_timer_disable(void);
u8_t ip4_napt_iface_binding(struct netif *in_if, struct netif *out_if);
void  ip4_napt_init(void);
u8_t  ip4_napt_send(struct pbuf *p);
u8_t  ip4_napt_receive(struct pbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_IPV4 */

#endif /* LWIP_HDR_IP_ADDR_H */
