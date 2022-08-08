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
/*
***************************************************************************
 Module Name:
 LWIP

 Abstract:
 Realize NAPT Routing, support ICMP/TCP/UDP

 Revision History:
 Who             When              What
 Evan Li         2021/11/17        Initial
 --------        ----------        --------------------------------------
***************************************************************************
*/


#include "lwip/opt.h"


#if LWIP_IPV4
#if IP_NAPT

#include "lwip/ip4_frag.h"
#include "lwip/ip4_napt_forward.h"
#include "lwip/def.h"
#include "lwip/inet_chksum.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/icmp.h"
#include "lwip/prot/tcp.h"
#include "lwip/udp.h"
#include "lwip/timeouts.h"
#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#endif
#include <string.h>

/*unit in millisecond*/
#define GET_SYS_TIME sys_now() * portTICK_RATE_MS
#define GET_SYS_TIME_DIFF(x, y) (((x) >= (y)) ? ((x) - (y)) : ((((0xFFFFFFFF) + 1) - y) + x))

ip4_napt_bind_t ip4_napt_bind_iface;
ip4_napt_table_t ip4_napt_table;
uint32_t g_ip4_napt_clean_entry_timer;

u8_t
ip4_napt_iface_binding(struct netif *in_if, struct netif *out_if)
{
  if (in_if == NULL || out_if == NULL) {
  	LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: an interface is not ready.\n"));
	return -1;
  }

  ip4_napt_bind_iface.in_if = in_if;
  ip4_napt_bind_iface.out_if = out_if;
  return 0;
}

void
ip4_napt_icmp_entry_init(ip4_napt_icmp_entry_t **head, u8_t num)
{
  u8_t i;

  for (i = 0; i < num; i++) {
    ip4_napt_icmp_entry_t *new_node = (ip4_napt_icmp_entry_t *)mem_malloc(sizeof(ip4_napt_icmp_entry_t));

    new_node->next = *head;
    *head = new_node;
  }
}

ip4_napt_icmp_list_t *
ip4_napt_icmp_list_init(u8_t num)
{
  ip4_napt_icmp_list_t *icmp_list;

  icmp_list = (ip4_napt_icmp_list_t *)mem_malloc(sizeof(ip4_napt_icmp_list_t));
  ip4_napt_icmp_entry_init(&icmp_list->free_list, num);

  return icmp_list;
}

void
ip4_napt_tcpudp_entry_init(ip4_napt_tcpudp_entry_t **head, u8_t num)
{
  u8_t i;

  for (i = 0; i < num; i++) {
    ip4_napt_tcpudp_entry_t *new_node = (ip4_napt_tcpudp_entry_t *)mem_malloc(sizeof(ip4_napt_tcpudp_entry_t));
    new_node->port.nport = htons((u16_t) (IP4_NAPT_PORT_START + num - i -1));
    new_node->next = *head;
    *head = new_node;
  }
}

ip4_napt_tcpudp_list_t *
ip4_napt_tcpudp_list_init(u8_t num)
{
  ip4_napt_tcpudp_list_t *tcpudp_list;

  tcpudp_list = (ip4_napt_tcpudp_list_t *)mem_malloc(sizeof(ip4_napt_tcpudp_list_t));
  ip4_napt_tcpudp_entry_init(&tcpudp_list->free_list, num);

  return tcpudp_list;
}

static void
ip4_napt_expired_entry_remove(ip4_napt_tcpudp_entry_t **used_list, ip4_napt_tcpudp_entry_t **free_list)
{
  u32_t now = GET_SYS_TIME;
  ip4_napt_tcpudp_entry_t *temp, *prev;

  temp = *used_list;

  /*check the head of list*/
  while (temp != NULL) {
    if (GET_SYS_TIME_DIFF(now, temp->timestamp) > g_ip4_napt_clean_entry_timer) {
      *used_list = temp->next;
      temp->next = *free_list;
      *free_list = temp;
      temp = *used_list;
    } else {
      break;
    }
  }

  if (temp == NULL)
    return;

  /*check others entry of list*/
  prev = temp;
  temp = temp->next;
  while (temp != NULL) {
    if (GET_SYS_TIME_DIFF(now, temp->timestamp) > g_ip4_napt_clean_entry_timer) {
      prev->next = temp->next;
      temp->next = *free_list;
      *free_list = temp;
      temp = prev->next;
    } else {
      prev = temp;
      temp = temp->next;
    }
  }
}

static void
ip4_napt_expired_entry_check(void)
{
  LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: napt timer timeout, check napt table.\n"));

  ip4_napt_expired_entry_remove(&ip4_napt_table.tcp->used_list, &ip4_napt_table.tcp->free_list);
  ip4_napt_expired_entry_remove(&ip4_napt_table.udp->used_list, &ip4_napt_table.udp->free_list);

  sys_timeout(g_ip4_napt_clean_entry_timer, ip4_napt_expired_entry_check, NULL);
}

void
ip4_napt_flush_timer_enable(void)
{
  /*init a timer to make the napt table fresh*/
  sys_timeout(g_ip4_napt_clean_entry_timer, ip4_napt_expired_entry_check, NULL);
}

void
ip4_napt_flush_timer_disable(void)
{
  /*remove a timer to make the napt table fresh*/
  sys_untimeout(ip4_napt_expired_entry_check, NULL);
}

void
ip4_napt_init(void)
{
  ip4_napt_icmp_list_t *icmp_list;
  ip4_napt_tcpudp_list_t *tcp_list;
  ip4_napt_tcpudp_list_t *udp_list;
  u8_t IP4_NAPT_MAX_TCP_ENTRY = 32;
  u8_t IP4_NAPT_MAX_UDP_ENTRY = 32;

#ifdef MTK_NVDM_ENABLE
  char buf[WIFI_PROFILE_BUFFER_LEN] = {0};
  uint32_t len = sizeof(buf);

  nvdm_read_data_item("AP", "NatpTcpEntry", (uint8_t *)buf, &len);
  IP4_NAPT_MAX_TCP_ENTRY = (uint8_t)atoi((char *)buf);
  len = sizeof(buf);
  nvdm_read_data_item("AP", "NatpUdpEntry", (uint8_t *)buf, &len);
  IP4_NAPT_MAX_UDP_ENTRY = (uint8_t)atoi((char *)buf);
#endif

  /* init free entry into free list*/
  icmp_list = ip4_napt_icmp_list_init(IP4_NAPT_MAX_ICMP_ENTRY);
  tcp_list = ip4_napt_tcpudp_list_init(IP4_NAPT_MAX_TCP_ENTRY);
  udp_list = ip4_napt_tcpudp_list_init(IP4_NAPT_MAX_UDP_ENTRY);

  ip4_napt_table.icmp = icmp_list;
  ip4_napt_table.tcp = tcp_list;
  ip4_napt_table.udp = udp_list;

  g_ip4_napt_clean_entry_timer = 10 * 1000;
}

struct netif *
ip4_napt_check_outif(struct ip_hdr *iphdr)
{
  struct netif *out_if = ip4_napt_bind_iface.out_if;

  if (out_if == NULL)
    goto end;

  /* is the netif up, does it have a link and a valid address? */
  if (netif_is_up(out_if) && netif_is_link_up(out_if) && !ip4_addr_isany_val(*netif_ip4_addr(out_if))) {
    /* network mask matches? */
    if (ip4_addr_netcmp(&(iphdr->dest), netif_ip4_addr(out_if), netif_ip4_netmask(out_if))) {
      /* return netif on which to forward IP packet */
      return out_if;
    }
  }

end:
  return NULL;
}

static void
ip4_napt_icmp_used_list_reset(void)
{
  ip4_napt_icmp_entry_t *used_head = ip4_napt_table.icmp->used_list;

  while (used_head != NULL) {
    if (used_head->next == NULL)
      break;
    used_head = used_head->next;
  }

  if (used_head != NULL) {
    used_head->next = ip4_napt_table.icmp->free_list;
    ip4_napt_table.icmp->free_list = ip4_napt_table.icmp->used_list;
    ip4_napt_table.icmp->used_list = NULL;
  }
}

static void
ip4_napt_tcpudp_used_list_reset(ip4_napt_tcpudp_list_t *list)
{
  ip4_napt_tcpudp_entry_t *used_head = list->used_list;

  while (used_head != NULL) {
    if (used_head->next == NULL)
      break;
    used_head = used_head->next;
  }

  if (used_head != NULL) {
    used_head->next = list->free_list;
    list->free_list = list->used_list;
    list->used_list = NULL;
  }
}

void
ip4_napt_icmp_list_add_head(ip4_napt_icmp_entry_t **head,
                           ip4_napt_icmp_entry_t *entry)
{
  entry->next = *head;
  *head = entry;
}

static ip4_napt_icmp_entry_t *
ip4_napt_icmp_list_remove_head(ip4_napt_icmp_entry_t **head)
{
  if (*head == NULL) {
    ip4_napt_icmp_used_list_reset();
    LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: icmp list is reset.\n"));
  }
  ip4_napt_icmp_entry_t *entry = *head;
  *head = (*head)->next;
  entry->next = NULL;
  return entry;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_tcp_list_search(ip4_napt_tcpudp_entry_t *head,
                           const struct ip_hdr *iphdr, const struct tcp_hdr *tcphdr)
{
  ip4_napt_tcpudp_entry_t *entry = head;
  while (entry != NULL) {
    if (ip4_addr_cmp(&iphdr->src, &entry->ip.source) &&
		  ip4_addr_cmp(&iphdr->dest, &entry->ip.destination) &&
          tcphdr->src == entry->port.sport &&
          tcphdr->dest == entry->port.dport)
      return entry;
	entry = entry->next;
  }
  return NULL;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_udp_list_search(ip4_napt_tcpudp_entry_t *head,
                           const struct ip_hdr *iphdr, const struct udp_hdr *udphdr)
{
  ip4_napt_tcpudp_entry_t *entry = head;
  while (entry != NULL) {
    if (ip4_addr_cmp(&iphdr->src, &entry->ip.source) &&
		  ip4_addr_cmp(&iphdr->dest, &entry->ip.destination) &&
          udphdr->src == entry->port.sport &&
          udphdr->dest == entry->port.dport)
      return entry;
	entry = entry->next;
  }
  return NULL;
}

void
ip4_napt_tcpudp_list_add_head(ip4_napt_tcpudp_entry_t **head,
                           ip4_napt_tcpudp_entry_t *entry)
{
  entry->next = *head;
  *head = entry;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_tcpudp_list_remove_head(ip4_napt_tcpudp_entry_t **head)
{
  if (*head == NULL) {
    LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: there is no entry in the free list.\n"));
	return NULL;
  }
  ip4_napt_tcpudp_entry_t *entry = *head;
  *head = (*head)->next;
  entry->next = NULL;
  return entry;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_tcp_lookup_list_before_send(const struct ip_hdr *iphdr,
                           const struct tcp_hdr *tcphdr)
{
  ip4_napt_tcpudp_entry_t *tcp_entry_found;

  tcp_entry_found = ip4_napt_tcp_list_search(ip4_napt_table.tcp->used_list, iphdr, tcphdr);
  if (tcp_entry_found == NULL) {
    /*pop an entry from the head of the free list*/
    tcp_entry_found = ip4_napt_tcpudp_list_remove_head(&ip4_napt_table.tcp->free_list);
    if (tcp_entry_found == NULL)
      return NULL;
    ip4_addr_set(&tcp_entry_found->ip.destination, &iphdr->dest);
    ip4_addr_set(&tcp_entry_found->ip.source, &iphdr->src);
    tcp_entry_found->port.sport = tcphdr->src;
    tcp_entry_found->port.dport = tcphdr->dest;
    tcp_entry_found->timestamp = GET_SYS_TIME;

    /*insert an entry to the head of the used list*/
    ip4_napt_tcpudp_list_add_head(&ip4_napt_table.tcp->used_list, tcp_entry_found);
  }
  return tcp_entry_found;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_udp_lookup_list_before_send(const struct ip_hdr *iphdr,
                           const struct udp_hdr *udphdr)
{
  ip4_napt_tcpudp_entry_t *udp_entry_found;

  udp_entry_found = ip4_napt_udp_list_search(ip4_napt_table.udp->used_list, iphdr, udphdr);
  if (udp_entry_found == NULL) {
    /*pop an entry from the head of the free list*/
    udp_entry_found = ip4_napt_tcpudp_list_remove_head(&ip4_napt_table.udp->free_list);
    if (udp_entry_found == NULL)
      return NULL;
    ip4_addr_set(&udp_entry_found->ip.destination, &iphdr->dest);
    ip4_addr_set(&udp_entry_found->ip.source, &iphdr->src);
    udp_entry_found->port.sport = udphdr->src;
    udp_entry_found->port.dport = udphdr->dest;
    udp_entry_found->timestamp = GET_SYS_TIME;

    /*insert an entry to the head of the used list*/
    ip4_napt_tcpudp_list_add_head(&ip4_napt_table.udp->used_list, udp_entry_found);
  }
  return udp_entry_found;
}

uint8_t
ip4_napt_send(struct pbuf *p)
{
  struct ip_hdr *iphdr = p->payload;
  struct netif *out_if = ip4_napt_bind_iface.out_if;
  struct icmp_echo_hdr *icmphdr;
  struct tcp_hdr *tcphdr;
  struct udp_hdr *udphdr;
  ip4_napt_tcpudp_entry_t *tcpudp_entry_found;
  ip4_napt_icmp_entry_t *icmp_entry_found;
  err_t err;

  out_if = ip4_napt_check_outif(iphdr);
  if (out_if == NULL) {
    LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: there is no out interface.\n"));
    goto fail;
  }

  /*process layer 4 header*/
  switch (IPH_PROTO(iphdr)) {
  case IP_PROTO_ICMP:
    icmphdr = (struct icmp_echo_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    if (ICMPH_TYPE(icmphdr) == ICMP_ECHO) {
      /*pop an entry from the head of the free list*/
      icmp_entry_found = ip4_napt_icmp_list_remove_head(&ip4_napt_table.icmp->free_list);
      if (icmp_entry_found == NULL)
        goto fail;
      ip4_addr_set(&icmp_entry_found->ip.destination, &iphdr->dest);
      ip4_addr_set(&icmp_entry_found->ip.source, &iphdr->src);
      icmp_entry_found->id = icmphdr->id;
      icmp_entry_found->seqno = icmphdr->seqno;
      /*insert an entry to the head of the used list*/
      ip4_napt_icmp_list_add_head(&ip4_napt_table.icmp->used_list, icmp_entry_found);
    }
    break;
  case IP_PROTO_TCP:
    tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    tcpudp_entry_found = ip4_napt_tcp_lookup_list_before_send(iphdr, tcphdr);
    if (tcpudp_entry_found == NULL)
      goto fail;
    tcphdr->src = tcpudp_entry_found->port.nport;
    /*TODO: adjust the checksum if need.*/
    break;

  case IP_PROTO_UDP:
    udphdr = (struct udp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    tcpudp_entry_found = ip4_napt_udp_lookup_list_before_send(iphdr, udphdr);
    if (tcpudp_entry_found == NULL)
      goto fail;
    udphdr->src = tcpudp_entry_found->port.nport;
    /*TODO: adjust the checksum if need.*/
    break;
  default:
    goto fail;
  }

  /*process layer 3 header*/
  //iphdr->src.addr = out_if->ip_addr.addr;
  iphdr->src.addr = ip4_addr_get_u32(ip_2_ip4(&out_if->ip_addr));
  /*TODO: adjust the checksum if need.*/
  err = out_if->output(out_if, p, (ip4_addr_t *)&(iphdr->dest));
  if (err != ERR_OK) {
    LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: failed to send the packet, error: %d.\n", err));
	goto fail;
  }

  return 1;
fail:
  return 0;
}

static ip4_napt_icmp_entry_t *
ip4_napt_icmp_list_mapping(ip4_napt_icmp_entry_t *head,
                           const struct ip_hdr *iphdr, const struct icmp_echo_hdr *icmphdr)
{
  ip4_napt_icmp_entry_t *entry = head;
  while (entry != NULL) {
    if (ip4_addr_cmp(&iphdr->src, &entry->ip.destination) &&
          icmphdr->id == entry->id &&
          icmphdr->seqno == entry->seqno)
      return entry;
	entry = entry->next;
  }
  return NULL;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_tcp_list_mapping(ip4_napt_tcpudp_entry_t *head,
                           const struct ip_hdr *iphdr, const struct tcp_hdr *tcphdr)
{
  ip4_napt_tcpudp_entry_t *entry = head;
  while (entry != NULL) {
    if (ip4_addr_cmp(&iphdr->src, &entry->ip.destination) &&
          tcphdr->src == entry->port.dport &&
          tcphdr->dest == entry->port.nport) {
      entry->timestamp = GET_SYS_TIME;
      return entry;
    }
	entry = entry->next;
  }
  return NULL;
}

static ip4_napt_tcpudp_entry_t *
ip4_napt_udp_list_mapping(ip4_napt_tcpudp_entry_t *head,
                           const struct ip_hdr *iphdr, const struct udp_hdr *udphdr)
{
  ip4_napt_tcpudp_entry_t *entry = head;
  while (entry != NULL) {
    if (ip4_addr_cmp(&iphdr->src, &entry->ip.destination) &&
          udphdr->src == entry->port.dport &&
          udphdr->dest == entry->port.nport) {
      entry->timestamp = GET_SYS_TIME;
      return entry;
    }
	entry = entry->next;
  }
  return NULL;
}

uint8_t
ip4_napt_receive(struct pbuf *p)
{
  struct ip_hdr *iphdr = p->payload;
  struct netif *in_if = ip4_napt_bind_iface.in_if;
  struct tcp_hdr *tcphdr;
  struct udp_hdr *udphdr;
  struct icmp_echo_hdr *icmphdr;
  ip4_napt_icmp_entry_t *icmp_entry_found;
  ip4_napt_tcpudp_entry_t *tcpudp_entry_found;
  err_t err;

  /* process layer 4 header*/
  switch (IPH_PROTO(iphdr)) {
    case IP_PROTO_ICMP:
      icmphdr = (struct icmp_echo_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
      if (ICMPH_TYPE(icmphdr) == ICMP_ER) {
        icmp_entry_found = ip4_napt_icmp_list_mapping(ip4_napt_table.icmp->used_list, iphdr, icmphdr);
        if (icmp_entry_found == NULL)
          goto fail;
	    iphdr->dest.addr = icmp_entry_found->ip.source.addr;
		/*TODO: adjust the checksum if need.*/
      } else
        goto fail;
      break;
	case IP_PROTO_TCP:
      tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
      tcpudp_entry_found = ip4_napt_tcp_list_mapping(ip4_napt_table.tcp->used_list, iphdr, tcphdr);
      if (tcpudp_entry_found == NULL)
        goto fail;
      tcphdr->dest = tcpudp_entry_found->port.sport;
      /*TODO: adjust the checksum if need.*/
	  iphdr->dest.addr = tcpudp_entry_found->ip.source.addr;
      /*TODO: adjust the checksum if need.*/
      break;
    case IP_PROTO_UDP:
      udphdr = (struct udp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
      tcpudp_entry_found = ip4_napt_udp_list_mapping(ip4_napt_table.udp->used_list, iphdr, udphdr);
      if (tcpudp_entry_found == NULL)
        goto fail;
	  udphdr->dest = tcpudp_entry_found->port.sport;
      /*TODO: adjust the checksum if need.*/
      iphdr->dest.addr = tcpudp_entry_found->ip.source.addr;
      /*TODO: adjust the checksum if need.*/
      break;
    default:
      goto fail;
  }

  err = in_if->output(in_if, p, (ip4_addr_t *)&(iphdr->dest));
  if (err != ERR_OK) {
    LWIP_DEBUGF(IP_DEBUG, ("ip4_napt: failed to send the packet, error: %d.\n", err));
	goto fail;
  }

  return 1;
fail:
  return 0;
}

#endif /* IP_NAPT */
#endif /* LWIP_IPV4 */
