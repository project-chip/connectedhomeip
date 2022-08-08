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
//#include <strings.h>

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
#include "lwip/netif.h"

#ifdef MTK_TCPIP_FOR_EXTERNAL_MODULE_ENABLE
#include "psnetif.h"
#ifdef __CMUX_SUPPORT__
#include "cmux.h"
#endif
#include "sio_gprot.h"
#include "sio_uart_gprot.h"
#include "urc_app.h"
#include "timers.h"
#include "hal_gpt.h"
#include "semphr.h"

#define ATCMD_MAX_PKT_SIZE        1024
#define ATCMD_MAX_RAW_DATA_SIZE   552 //1000   //ATCMD_MAX_PKT_SIZE substract  //AT+EIPDATA=<CID>,<Length>,<
#define ATCMD_EIPDATA_HEADER      50

#define MAX_PSNETIF_HEX_LENGTH  (3)
#define MAX_PSNETIF_TEMP_LENGTH (5)
#define MAX_PSNETIF_CID_LENGTH  (3)
#define MAX_PSNETIF_DATA_LENGTH (4)
#define PS_NETIF_QUEUE_SIZE          (10)

//#define __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
//#define PS_DATA_DEBUG

log_create_module(psnetif, PRINT_LEVEL_INFO);
#define LOGE(fmt,arg...)   LOG_E(psnetif, "[PSNETIF]: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(psnetif, "[PSNETIF]: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(psnetif ,"[PSNETIF]: "fmt,##arg)

static SemaphoreHandle_t is_sending;
#ifndef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
static int32_t tcpip_app_id;
#endif
static QueueHandle_t ps_netif_queue_id;

struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};
static int32_t ps_netif_send_message(ps_netif_msg_id_t msg_id, void *data);

static int get_data_by_character(char *out, char *in, char c, int len);

static void low_level_input_callback(uint32_t event, void *data);

#ifdef __CMUX_SUPPORT__
static void low_level_input_cmux_data(cmux_event_ready_to_read_t *msg_data);
#endif

static void low_level_input_uart_data(sio_rx_data_to_read_struct *msg_data);

static void low_level_input(struct netif *netif, struct pbuf *p);

static int32_t low_level_input_urc_handler(uint8_t *payload, uint32_t length);

static int32_t low_level_output_response(uint8_t *payload, uint32_t length);

static int32_t low_level_output(struct netif *netif, struct pbuf *p);

static err_t ps_ip4_output(struct netif *netif, struct pbuf *p, const ip4_addr_t *ipaddr);
#if LWIP_IPV6
static err_t ps_ip6_output(struct netif *netif, struct pbuf *p, const ip6_addr_t *ipaddr);
#endif
static void low_level_ps_init_callback(struct netif *netif);

static err_t ps_netif_init_callback(struct netif *netif);


#ifdef PS_DATA_DEBUG
void
psdata_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
  unsigned char *pt;
  int x;

  pt = pSrcBufVA;
  printf("%s: %p, len = %d\n\r", str, pSrcBufVA, SrcBufLen);
  for (x=0; x<SrcBufLen; x++) {
    if (x % 16 == 0)
      printf("0x%04x : ", x);
    printf("%02x ", ((unsigned char)pt[x]));
    if (x%16 == 15) printf("\n\r");
  }
  printf("\n\r");
}
#endif


#ifdef PS_DATA_UT
void
ps_data_input_ut()
{
  const char *payload = "+EIPDATA:1,60,\"4500003c9ad4000040015c37c0a80101c0a80164000073570200e004000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\"";
  int length = strlen(payload);

  low_level_input_urc_handler(payload, length);
}
#endif


//After activated ps, need to set context information to lwip
s8_t
ps_netif_set_netinfo(u8_t context_id, gprs_net_info_struct *netinfo)
{
  struct netif *netif;
  netif_input_fn  input_fn;
  ip_addr_t dnsserver;
  ip4_addr_t ipaddr;
  int ret = 0;

  u32_t mtu = netinfo->mtu;
  input_fn = tcpip_input;
  if (netinfo->ipaddr == NULL) {
    return -1;
  }
  ret = ip4addr_aton(netinfo->ipaddr, &ipaddr);

  netif = mem_malloc(sizeof(struct netif));
  if (netif == NULL) {
    return -2;
  }
  memset(netif, 0, sizeof(struct netif));
  netif_add(netif, &ipaddr, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL,
              ps_netif_init_callback, input_fn);

  if (netinfo->dnsservser1 == NULL) {
    return -3;
  }
  ipaddr_aton(netinfo->dnsservser1, &dnsserver);
  dns_setserver(0, &dnsserver);

  if (netinfo->dnsservser2 != NULL) {
    ipaddr_aton(netinfo->dnsservser2, &dnsserver);
    dns_setserver(1, &dnsserver);
  }
  netif->context_id = context_id;
  //limitation by uart buffer 1024. substract  //AT+EIPDATA=<CID>,<Length>,<
  mtu = (mtu == 0) ? ATCMD_MAX_RAW_DATA_SIZE : mtu;
  netif->mtu = mtu > ATCMD_MAX_RAW_DATA_SIZE ? ATCMD_MAX_RAW_DATA_SIZE : mtu;

  netif_set_up(netif);
  return ret;
}


//After deactivated ps, need to inform lwip remove related context information
s8_t
ps_netif_remove_netinfo(u8_t context_id)
{
  struct netif *netif;
  netif = netif_find_by_context_id(context_id);
  if (netif != NULL) {
    netif_remove(netif);
    mem_free(netif);
    return 0;
  }
  return -1;
}


//format of <CID>,<Length>,<IP PACKAGE>, get data between ','
static int
get_data_by_character(char *out, char *in, char c, int len)
{
  int i;
  char ch;
  for(i = 0, ch = in[i]; ch != '\0'; i++) {
    ch = in[i];
    if (ch == c || i > len) {
      break;
    }
    out[i] = in[i];
  }
  out[i] = '\0';
  return i;
}


void
get_data_from_hex(char *out, char *in, int len)
{
  int i;
  unsigned long c;
  char temp[MAX_PSNETIF_HEX_LENGTH] = {0};

  for (i = 0; i < len; i++) {
    memcpy(temp, in + i * 2, 2);
    c = strtoul(temp, 0, 16);
    out[i] = (char)c;
  }
}


static int32_t
ps_netif_send_message(ps_netif_msg_id_t msg_id, void *data)
{
    ps_netif_msg_t msg;

    msg.msg_id = msg_id;
    msg.msg_data = data;

    if (xQueueSend(ps_netif_queue_id, &msg, portMAX_DELAY) != pdPASS) {
        LWIP_DEBUGF(NETIF_DEBUG, ("xQueueSend != pdPASS"));
        return 0;
    }
    return 1;
}

#ifdef __CMUX_SUPPORT__
static void
low_level_input_cmux_data(cmux_event_ready_to_read_t *msg_data)
{
  uint8_t *payload;
  uint32_t length;
#ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
  uint32_t channel_id = msg_data->channel_id;
#else
  uint32_t channel_id = sio_get_channel_id(tcpip_app_id);
#endif

  if (msg_data != NULL) {
  #ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
    LWIP_DEBUGF(NETIF_DEBUG, ("receive cmux data.chnl_id=%d\n", channel_id));
  #else
    LWIP_DEBUGF(NETIF_DEBUG, ("receive cmux data.chnl_id=%d, app chnl_id=%d\n", msg_data->channel_id, channel_id));
    configASSERT(msg_data->channel_id == channel_id);
  #endif
    length = cmux_get_available_read_length(channel_id);
    payload = pvPortMalloc(length);

  #ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
    length = sio_receive_data(channel_id, payload, length);
    p = pbuf_alloc(PBUF_IP, (ssize_t)ATCMD_MAX_PKT_SIZE, PBUF_POOL);
    if (p == NULL) {
      LWIP_DEBUGF(NETIF_DEBUG, ("not enough buffer to receive!\n"));
      vPortFree(payload);
      vPortFree(msg_data);
      return;
    }
    memcpy(p->payload, payload, length);
    p->len = p->tot_len= length;

    cid = cmux_get_pdp_context_id(channel_id);
    netif = netif_find_by_context_id(cid);
    if (netif == NULL) {
      LWIP_DEBUGF(NETIF_DEBUG, ("not find netif by cid, %d!\n", cid));
      vPortFree(payload);
      vPortFree(msg_data);
      return;
    }

    low_level_input(netif, p);
  #else /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
    length = sio_receive_data(tcpip_app_id, payload, length);
    low_level_output_response(payload, length);
  #endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
    vPortFree(payload);
    vPortFree(msg_data);
  }
}
#endif /* __CMUX_SUPPORT__ */


static void
low_level_input_uart_data(sio_rx_data_to_read_struct *msg_data)
{
  uint8_t *payload;
  uint32_t length;

#ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
  LWIP_DEBUGF(NETIF_DEBUG, ("-------------------------\ntemp_id=%d\n-------------------------\n", msg_data->app_id));
  if (msg_data != NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("receive uart data. app_id=%d\n", msg_data->app_id));
    length = msg_data->read_length;
    payload = pvPortMalloc(length);
    length = sio_receive_data(msg_data->app_id, payload, length);
    low_level_output_response(payload, length);
    vPortFree(payload);
    vPortFree(msg_data);
  }
#else /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
  LWIP_DEBUGF(NETIF_DEBUG, ("-------------------------\ntemp_id=%d, app_id=%d\n-------------------------\n", msg_data->app_id, tcpip_app_id));
  if (msg_data != NULL && tcpip_app_id == msg_data->app_id) {
    LWIP_DEBUGF(NETIF_DEBUG, ("receive uart data. app_id=%d,------app app_id=%d\n", msg_data->app_id, tcpip_app_id));
    length = msg_data->read_length;
    payload = pvPortMalloc(length);
    length = sio_receive_data(tcpip_app_id, payload, length);
    low_level_output_response(payload, length);
    vPortFree(payload);
    vPortFree(msg_data);
  }
#endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
}



static int32_t
low_level_input_urc_handler(uint8_t *payload, uint32_t length)
{
    const char atcmd_receive_header[] = "+EIPDATA:";
    u32_t len, cid_len, cid, data_len, pos = 0;
    int  ret = -1;
    struct pbuf *p;
    char *str;
    char temp[MAX_PSNETIF_TEMP_LENGTH] = {0};
    struct netif *netif;

    len = strlen(atcmd_receive_header);
    if (length < len) {
        LWIP_DEBUGF(NETIF_DEBUG, ("not ipdata,length is wrong, ignore!\n"));
        return 0;
    }
    //remove \r\n in header of at cmd response
    while(pos < length) {
      if (payload[pos] == '\r' || payload[pos] == '\n') {
        pos++;
      } else {
        break;
      }
    }

    //is ip data
    ret = strncasecmp((char *)(payload + pos), atcmd_receive_header, len);
    if (ret != 0) {
        LWIP_DEBUGF(NETIF_DEBUG, ("not ipdata, format is wrong, ignore!\n"));
        return 0; //not ipdata, ignore
    }

    pos = pos + len;
    str = (char *)(payload + pos);
    cid_len = get_data_by_character(temp, str, ',', MAX_PSNETIF_CID_LENGTH);
    str = str + cid_len;
    if (str[0] != ',') {
      LWIP_DEBUGF(NETIF_DEBUG, ("error cid format\n"));
      return 0;//ipdata format error, ignore
    }
    cid = atoi(temp);
    LWIP_DEBUGF(NETIF_DEBUG, ("cid = %d\n", cid));

    pos = pos + cid_len + 1;
    if (pos > length) {
        LWIP_DEBUGF(NETIF_DEBUG, ("not ipdata, format is wrong, ignore!\n"));
        return 0; //not ipdata, ignore
    }
    memset(temp, 0, MAX_PSNETIF_TEMP_LENGTH);
    str = (char *)(payload + pos);
    data_len = get_data_by_character(temp, str, ',', MAX_PSNETIF_DATA_LENGTH);
    str = str + data_len;
    if (str[0] != ',') {
      LWIP_DEBUGF(NETIF_DEBUG, ("error data length format\n"));
      return 0;//ipdata format error, ignore
    }
    len = atoi(temp);
    LWIP_DEBUGF(NETIF_DEBUG, ("len = %d\n", len));

    pos = pos + data_len + 1 + len * 2 + 2;
    if (pos > length) {
        LWIP_DEBUGF(NETIF_DEBUG, ("not ipdata, format is wrong, ignore!\n"));
        return 0; //not ipdata, ignore
    }

    str = str + 1;
    if (str[0] == ' ') {
      str = str + 1;
    }

    if (str[0] != '\"' || str[len * 2 + 1] != '\"') {
      LWIP_DEBUGF(NETIF_DEBUG, ("error data format\n"));
      return 0;//ipdata format error, ignore
    }

    str = str + 1;
    p = pbuf_alloc(PBUF_IP, (u16_t)ATCMD_MAX_PKT_SIZE, PBUF_POOL);
    if (p == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("not enough buffer to receive!\n"));
        return 0;
    }
    get_data_from_hex(p->payload, str, len);

    p->len = p->tot_len= len;

    netif = netif_find_by_context_id(cid);
    if (netif == NULL) {
      pbuf_free(p);
      p = NULL;
      return 0;
    }
    low_level_input(netif, p);
    return 1;
  }

//+EIPDATA: <CID>,<Length>,<IP PACKAGE>  ---1024
static void
low_level_input(struct netif *netif, struct pbuf *p)
{
  if (p == NULL)
    return;
  #ifdef PS_DATA_DEBUG
    psdata_hex_dump("low_level_input", p->payload,  p->len);
  #endif
  if (netif->input(p, netif) != ERR_OK) {
    LWIP_DEBUGF(NETIF_DEBUG, ("IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
}


static int32_t
low_level_output_response(uint8_t *payload, uint32_t length)
{
  LWIP_DEBUGF(NETIF_DEBUG, ("reset is_sending flag, is_sending=%d payload=%s\n", is_sending, payload));
  xSemaphoreGive(is_sending);
  return 0;
}


//AT+EIPDATA=<CID>,<Length>,<IP PACKAGE>  ---1024
static int32_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  uint32_t total_len = 0;
  int i, len = 0;
  char *snd_data = NULL;
  char *data = NULL;
  int ret_len;
  char *end_data = "\"\r\n";
#ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
  int32_t channel_id = 0;
#endif

  xSemaphoreTake(is_sending, portMAX_DELAY);
  for (q = p; q != NULL; q = q->next) {
    total_len = total_len + (q->len);
  }

  if (total_len > netif->mtu) {
    return ERR_MEM;
  }
  data = pvPortCalloc(1, ATCMD_EIPDATA_HEADER + total_len * 2 + 1);
  if (data == NULL) {
    return ERR_BUF;
  }
  snd_data = pvPortCalloc(1, total_len + 1);
  if (snd_data == NULL) {
    vPortFree(data);
    return ERR_BUF;
  }
  memset(data, 0, total_len * 2 + 1);
  memset(snd_data, 0, total_len + 1);
  sprintf(data, "AT+EIPDATA=%d,%d,\"", (int)netif->context_id, (int)total_len);
  if (total_len == p->len) {
    memcpy(snd_data, p->payload, p->len);
    #ifdef PS_DATA_DEBUG
    psdata_hex_dump("low_level_output", p->payload,  p->len);
    #endif
  } else {
    for (q = p; q != NULL; q = q->next) {
      memcpy(snd_data + len, q->payload, q->len);
      len = len + q->len;
      #ifdef PS_DATA_DEBUG
      psdata_hex_dump("low_level_output", q->payload,  q->len);
      #endif
    }
  }
  len = strlen(data);
  for (i = 0; i < total_len; i++) {
    sprintf(data + len + (i * 2), "%02x", snd_data[i]);
  }
  memcpy(data + len + (i * 2), end_data, 3);
  len = len + (i * 2) + 3;
  LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output:%s, %d\n", data, len));
#ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
  channel_id = cmux_get_channel_id(netif->context_id);
  LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output:netif->context_id = %d, channel_id = %d\n", netif->context_id, channel_id));
  ret_len = sio_send_data(channel_id, (const uint8_t*)data, len);
#else /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
  ret_len = sio_send_data(tcpip_app_id, (const uint8_t*)data, len);
#endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */

  LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output: sio_send_data ret_len=%d\n", ret_len));
  #ifdef PS_DATA_UT
  ps_data_input_ut();
  ret_len = data->len;
  #else
  if (ret_len != len) {
    xSemaphoreGive(is_sending);
    ret_len = ERR_BUF;
  } else {
    ret_len = ERR_OK;
  }
  #endif
  vPortFree(snd_data);
  vPortFree(data);
  return ret_len;
}


static err_t
ps_ip4_output(struct netif *netif, struct pbuf *p, const ip4_addr_t *ipaddr)
{
    return low_level_output(netif, p);
}


#if LWIP_IPV6
static err_t
ps_ip6_output(struct netif *netif, struct pbuf *p, const ip6_addr_t *ipaddr)
{
    return low_level_output(netif, p);
}
#endif

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_ps_init_callback(struct netif *netif)
{
#if 0 // init MAC address in upper layer
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x0c;
  netif->hwaddr[2] = 0x43;
  netif->hwaddr[3] = 0x76;
  netif->hwaddr[4] = 0x62;
  netif->hwaddr[5] = 0x02;
#endif

  /* maximum transfer unit */
  //netif->mtu = 1000;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_LINK_UP;

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
ps_netif_init_callback(struct netif *netif)
{
  //struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
#if 0
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ps_netif_init_callback: out of memory\n"));
    return ERR_MEM;
  }
#endif
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

  netif->state = NULL; //ethernetif;
  netif->name[0] = PSIFNAME00;
  netif->name[1] = PSIFNAME01;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = ps_ip4_output;
  #if LWIP_IPV6
  netif->output_ip6 = ps_ip6_output;
  #endif /* LWIP_IPV6 */

  netif->linkoutput = (netif_linkoutput_fn)low_level_output;

  //ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_ps_init_callback(netif);

  return ERR_OK;
}


urc_cb_ret ps_netif_urc_callback(uint8_t *payload, uint32_t length)
{
    LWIP_DEBUGF(NETIF_DEBUG, ("ps_netif_urc_callback:\n %s\n", payload));
    low_level_input_urc_handler(payload, length);
    return RET_OK_CONTINUE;
}


static void
low_level_input_callback(uint32_t event, void *data)
{
    LWIP_DEBUGF(NETIF_DEBUG, ("--------event=%d----------\n", event));
    switch (event) {
        case SIO_UART_EVENT_MODEM_READY: {
            netif_remove_all();
            break;
        }

        case SIO_UART_EVENT_MODEM_DATA_TO_READ: {
            sio_rx_data_to_read_struct *rx_data = (sio_rx_data_to_read_struct *)pvPortMalloc(sizeof(sio_rx_data_to_read_struct));
            memcpy(rx_data, (sio_rx_data_to_read_struct *)data, sizeof(sio_rx_data_to_read_struct));
          #ifndef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
            if (rx_data->app_id != tcpip_app_id) {
                vPortFree(rx_data);
                break;
            }
           #endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
            ps_netif_send_message(MSG_ID_PS_NETIF_UART_DATA, rx_data);
            break;
        }
    #ifdef __CMUX_SUPPORT__
        case CMUX_EVENT_READY_TO_READ: {
            cmux_event_ready_to_read_t *ready_to_read = (cmux_event_ready_to_read_t *)pvPortMalloc(sizeof(cmux_event_ready_to_read_t));
            memcpy(ready_to_read, (cmux_event_ready_to_read_t *)data, sizeof(cmux_event_ready_to_read_t));
            LWIP_DEBUGF(NETIF_DEBUG, ("channel_id = %d", ready_to_read->channel_id));
          #ifndef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
            if (ready_to_read->channel_id != sio_get_channel_id(tcpip_app_id)) {
                vPortFree(ready_to_read);
                break;
            }
          #endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
            ps_netif_send_message(MSG_ID_PS_NETIF_CMUX_DATA, ready_to_read);
            break;
        }
    #endif /* __CMUX_SUPPORT__ */
        default: {
            break;
        }
    }
}


void ps_netif_task_process(void *arg)
{
    ps_netif_msg_t msg_queue_data;

    /* Loop, processing httpd cmds. */
    for (;;) {
        if (xQueueReceive(ps_netif_queue_id, &msg_queue_data, portMAX_DELAY) == pdPASS) {
            LWIP_DEBUGF(NETIF_DEBUG, ("ps_netif_task_process() msg id = %d\r\n", msg_queue_data.msg_id));
            /* message handling */
            if (MSG_ID_PS_NETIF_UART_DATA == msg_queue_data.msg_id) {
                low_level_input_uart_data((sio_rx_data_to_read_struct *)(msg_queue_data.msg_data));
            }
        #ifdef __CMUX_SUPPORT__
            else if (MSG_ID_PS_NETIF_CMUX_DATA == msg_queue_data.msg_id) {
                low_level_input_cmux_data((cmux_event_ready_to_read_t *)(msg_queue_data.msg_data));
            }
        #endif
        }
    }
}



void
ps_netif_init(void)
{
  //tcpip_init(tcpip_init_done, tcpip_init_done_param);
  /* implicitly calls lwip_init();
       start new thread calls tcpip_init_done(tcpip_init_done_param);
       when tcpip init done tcpip_init_done and tcpip_init_done_param are user-defined (may be NULL).
   */
  urc_register_callback(ps_netif_urc_callback);

#ifdef __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__
  sio_register_event_notifier(SIO_APP_TYPE_CMUX_IP_DATA, low_level_input_callback);
  sio_register_event_notifier(SIO_APP_TYPE_CMUX_IP_DATA, low_level_input_callback);
  sio_register_event_notifier(SIO_APP_TYPE_CMUX_IP_DATA, low_level_input_callback);
#else /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
  tcpip_app_id = sio_register_event_notifier(SIO_APP_TYPE_CMUX_AT_CMD, low_level_input_callback);
#endif /* __PSNETIF_SUPPORT_CMUX_DATA_CHANNEL__ */
  //ps netif don't add here, it should be added after ps activated.

    ps_netif_queue_id = xQueueCreate(PS_NETIF_QUEUE_SIZE, sizeof(ps_netif_msg_t));
    vSemaphoreCreateBinary(is_sending);
    xTaskCreate(
        ps_netif_task_process,
        PS_NETIF_TASK_NAME,
        PS_NETIF_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
        NULL,
        PS_NETIF_TASK_PRIO,
        NULL);
}

#endif
