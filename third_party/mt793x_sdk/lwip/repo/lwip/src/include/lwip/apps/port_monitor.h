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

#ifndef LWIP_HDR_APPS_PORT_MONITOR_H
#define LWIP_HDR_APPS_PORT_MONITOR_H

#include "lwip/pbuf.h"
#include "lwip/apps/port_monitor_opts.h"

#ifdef __cplusplus
extern "C" {
#endif


/** Port monitor packet struct */
typedef struct port_monitor_packet_s {
  /** next packet */
  struct port_monitor_packet_s *next;
  /** dest port */
  u16_t dest_port;
  /** syn flag */
  u8_t syn_flag;
  /** length of payload */
  u16_t len;
  /** pointer to the actual data in the buffer */
  void *payload;  
}port_monitor_packet_t;

/** Port monitor port struct */
typedef struct port_monitor_port_s {
  /** next port */
  struct port_monitor_port_s *next;
  /** monitor port */
  u16_t port;
}port_monitor_port_t;

int port_monitor_check_port_in_list(u16_t block_port);
int port_monitor_config_insert_port(u16_t block_port);
int port_monitor_config_remove_port(u16_t block_port);
void port_monitor_config_logging_enable(u8_t logging_enable);
port_monitor_port_t * port_monitor_get_port_list(void);
port_monitor_packet_t * port_monitor_get_logging_packets(void);
void port_monitor_clear_logging_packets(void);
int port_monitor_packet_input(u16_t port, u8_t is_syn, struct pbuf *p);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_APPS_PORT_MONITOR_H */
