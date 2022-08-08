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
#include "lwip/timers.h"

#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
#include "nbnetif.h"
#include "mux_ap.h"
#include "ril_task.h"
#include "memory_attribute.h"
#include "hal_rtc_internal.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_platform.h"
#include "hal_rtc_external.h"
#include "syslog.h"
#include "apb_proxy.h"

/* Define those to better describe your network interface. Note: only 2-byte allowed */
#define IFNAME00 'p'
#define IFNAME01 's'
#define NB_NETIF_QUEUE_SIZE          (16)
#if 1
log_create_module(nbnetif, PRINT_LEVEL_INFO);
#define NBNETIF_LOGE(fmt,arg...)   LOG_E(nbnetif, "[NBNETIF]: "fmt,##arg)
#define NBNETIF_LOGW(fmt,arg...)   LOG_W(nbnetif, "[NBNETIF]: "fmt,##arg)
#define NBNETIF_LOGI(fmt,arg...)   LOG_I(nbnetif ,"[NBNETIF]: "fmt,##arg)
#else
#define NBNETIF_LOGI(fmt,args...)   printf("[NB NETIF] "fmt, ##args)
#endif

#define NB_NETIF_BACKUP_TOTAL_ITEM 1
typedef struct _nbiot_bearer_info_backup_struct
{
    u8_t is_activated; //activated (1) and deactivated(0)
    u8_t cid;
    u8_t type; //ipv4(0) or ipv6(1)
    u32_t mtu;
    u32_t channel_id;
    unsigned char ip_addr[64];
    unsigned char pri_dns[64];
} nbiot_bearer_info_backup_struct;

#if LWIP_IPV6
extern u8_t g_ip6_nd_timer_active;
#endif
nb_netif_context_struct *g_netif_context;
static ATTR_ZIDATA_IN_RETSRAM nbiot_bearer_info_backup_struct g_lwip_nbiot_bearer_retention[NB_NETIF_BACKUP_TOTAL_ITEM];

static void nb_netif_task_process(void *arg);
static void nb_low_level_mux_callback(mux_ap_event_t event, void *data);
static void nb_low_level_input(struct netif *netif, struct pbuf *p);
static err_t nb_low_level_output(struct netif *netif, struct pbuf *p);
static err_t nb_ip4_output(struct netif *netif, struct pbuf *p, const ip4_addr_t *ipaddr);
#if LWIP_IPV6
static err_t nb_ip6_output(struct netif *netif, struct pbuf *p, const ip6_addr_t *ipaddr);
#endif
static void nb_low_level_init_callback(struct netif *netif);
static err_t nb_netif_init_callback(struct netif *netif);
static int nb_netif_send_message(nb_netif_msg_id_t msg_id, void *data);
static int nb_netif_add_netinfo(nb_net_info_add_t *netinfo);
static int nb_netif_del_netinfo(u8_t context_id);
static int nb_cid_add(u8_t context_id, u32_t channel_id);
static int nb_cid_del(u8_t context_id, u32_t channel_id);
static u8_t nd_get_cid_by_channel(u32_t channel_id);
static u32_t nd_get_channel_id_by_cid(u8_t context_id);

#ifdef PS_DATA_DEBUG
void psdata_hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
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
void ps_data_input_ut()
{
    const char *payload = "+EIPDATA:1,60,\"4500003c9ad4000040015c37c0a80101c0a80164000073570200e004000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\"";
    int length = strlen(payload);

    low_level_input_urc_handler(payload, length);
}
#endif

static void nb_send_ip_address_urc(char *addr)
{
    char result_str[40] = {0};
    apb_proxy_at_cmd_result_t response = {0};

    sprintf(result_str,"+IP: %s", addr);

    response.pdata = result_str;
    response.result_code = APB_PROXY_RESULT_UNSOLICITED;
    response.length = strlen((char *)response.pdata);
    apb_proxy_send_at_cmd_result(&response);
}


int nb_parse_ipv6_addr(char *from_ip_addr, char *to_ip6addr)
{
    char *p = NULL;
    int i = 0;
    p = strtok(from_ip_addr, ".");
    while (p != NULL && i < 16) {
        int a = atoi(p);
        if(a < 256) {
            to_ip6addr[i] = (char)a;
            p = strtok(NULL, ".");
            i++;
        } else {
            return 0;
        }
    }
    if (i != 16) {
        return 0;
    }
    return 1;
}


void nb_backup_bearer_info(nbiot_bearer_info_backup_struct *backup_item, nbiot_bearer_info_struct *bearer_info)
{
    backup_item->is_activated = bearer_info->is_activated;
    backup_item->channel_id = bearer_info->channel_id;
    backup_item->cid = bearer_info->cid;
    backup_item->mtu = bearer_info->mtu;
    backup_item->type = bearer_info->type;
    memcpy(backup_item->ip_addr, bearer_info->ip_addr, 64);
    memcpy(backup_item->pri_dns, bearer_info->pri_dns, 64);
}


nbiot_bearer_info_backup_struct *nb_find_empty_backup_item()
{
    int i;
    for(i = 0; i < NB_NETIF_BACKUP_TOTAL_ITEM; i++) {
        NBNETIF_LOGI("find item %d, is_activated %d\n", i, g_lwip_nbiot_bearer_retention[i].is_activated);
        if(g_lwip_nbiot_bearer_retention[i].is_activated == 0) {
            return &g_lwip_nbiot_bearer_retention[i];
        }
    }
    return NULL;
}


nbiot_bearer_info_backup_struct *nb_find_backup_item_by_cid(u8_t cid)
{
    int i;
    for(i = 0; i < NB_NETIF_BACKUP_TOTAL_ITEM; i++) {
        NBNETIF_LOGI("backup item %d, backup cid %d, deactivated cid %d\n", i, g_lwip_nbiot_bearer_retention[i].cid, cid);
        if(g_lwip_nbiot_bearer_retention[i].cid == cid) {
            return &g_lwip_nbiot_bearer_retention[i];
        }
    }
    return NULL;
}

void nb_retention_backup_item()
{
    int i;
    for(i = 0; i < NB_NETIF_BACKUP_TOTAL_ITEM; i++) {
        NBNETIF_LOGI("retention item %d, activated %d\n", i, g_lwip_nbiot_bearer_retention[i].is_activated);
        if(g_lwip_nbiot_bearer_retention[i].is_activated == 1) {
            //activated before sleep.
            nbiot_bearer_info_backup_struct *netinfo;
            ip_addr_t dnsserver;
            nb_net_info_add_t *nbnetif;


            netinfo = &g_lwip_nbiot_bearer_retention[i];
            nbnetif = pvPortMalloc(sizeof(nb_net_info_add_t));
            memset(nbnetif, 0, sizeof(nb_net_info_add_t));
            if (netinfo->pri_dns[0] != 0) {
                if(ipaddr_aton(netinfo->pri_dns, &dnsserver) != 0)
                    dns_setserver(0, &dnsserver);
            }

            //ipv4 netif
            if (netinfo->ip_addr[0] != 0) {
                nbnetif->type = netinfo->type;
                if (netinfo->type == 0) {
                    ip4addr_aton(netinfo->ip_addr, &(nbnetif->ipaddr));
                } else if (netinfo->type == 1) {
                #if LWIP_IPV6
                    nb_parse_ipv6_addr((char *)netinfo->ip_addr, (char *)nbnetif->ip6addr);
                #endif
                }

                nbnetif->cid = netinfo->cid;
                nbnetif->channel_id = netinfo->channel_id;
                nbnetif->mtu = netinfo->mtu;
                nb_netif_send_message(MSG_ID_BEARER_INFO_ADD, (void *)nbnetif);
            }
        }
    }
}

//After activated ps, connection manager will call this API to notify lwip
int nb_netif_bearer_info_ind(nbiot_bearer_info_struct *netinfo)
{
    int ret = -1;
    nbiot_bearer_info_backup_struct *backup_item = NULL;
    NBNETIF_LOGI("cnmgr notify tcpip bearer is activated: %d, channel id: %d, cid %d\n",
                netinfo->is_activated, (int)netinfo->channel_id, netinfo->cid);

    if(netinfo->is_activated == 0) {
        nb_net_info_del_t *nbnetif;
        u8_t cid = netinfo->cid;
        u32_t channel_id = netinfo->channel_id;
        //change to ril callback
        mux_ap_change_callback(channel_id, ril_channel_mux_callback);
        nbnetif = pvPortMalloc(sizeof(nb_net_info_del_t));
        nbnetif->channel_id = channel_id;
        nbnetif->cid = cid;
        backup_item = nb_find_backup_item_by_cid(cid);
        if(backup_item != NULL) {
            backup_item->is_activated = 0;
        }
        ret = nb_netif_send_message(MSG_ID_BEARER_INFO_DEL, (void *)nbnetif);
        if(ret < 0) {
            NBNETIF_LOGI("can't detele netif\n");
            vPortFree(nbnetif);
        } else {
            if (netinfo->type == 1) {
            #if LWIP_IPV6
                g_ip6_nd_timer_active = 0;
            #endif
            }
        }
    } else {
        ip_addr_t dnsserver;
        nb_net_info_add_t *nbnetif;

        backup_item = nb_find_empty_backup_item();
        if(backup_item != NULL) {
            nb_backup_bearer_info(backup_item, netinfo);
        }
        nbnetif = pvPortMalloc(sizeof(nb_net_info_add_t));
        memset(nbnetif, 0, sizeof(nb_net_info_add_t));
        if (netinfo->pri_dns[0] != 0) {
            if(ipaddr_aton(netinfo->pri_dns, &dnsserver) != 0)
                dns_setserver(0, &dnsserver);
        }
        if (netinfo->snd_dns[0] != 0) {
            if(ipaddr_aton(netinfo->snd_dns, &dnsserver) != 0)
                dns_setserver(1, &dnsserver);
        }
        //ipv4 netif
        if (netinfo->ip_addr[0] != 0) {
            nbnetif->type = netinfo->type;
            if (netinfo->type == 0) {
                ret = ip4addr_aton(netinfo->ip_addr, &(nbnetif->ipaddr));
            } else if (netinfo->type == 1) {
            #if LWIP_IPV6
                ret = nb_parse_ipv6_addr(netinfo->ip_addr, nbnetif->ip6addr);
                //ret = ip6addr_aton(netinfo->ip_addr, &(nbnetif->ip6addr));
            #endif
            } else {
                NBNETIF_LOGI("Wrong IPaddr type\n");
            }

            if(ret > 0) {
                nbnetif->cid = netinfo->cid;
                nbnetif->channel_id = netinfo->channel_id;
                nbnetif->mtu = netinfo->mtu;
                ret = nb_netif_send_message(MSG_ID_BEARER_INFO_ADD, (void *)nbnetif);
            } else {
                NBNETIF_LOGI("Wrong IPaddr\n");
                ret = -2;
            }
        } else {
            NBNETIF_LOGI("IPaddr is empty\n");
            ret = -1;
        }

        if(ret < 0) {
            NBNETIF_LOGI("can't add netif\n");
            if(backup_item != NULL) {
                backup_item->is_activated = 0;
            }
            vPortFree(nbnetif);
        } else {
            if(netinfo->type == 0) {
                nb_send_ip_address_urc(netinfo->ip_addr);
            } else {
            #if LWIP_IPV6
                ip6_nd_timer_needed();
            #endif
            }
        }
    }

    return ret;
}

//After activated ps, need to set context information to lwip
static int nb_netif_add_netinfo(nb_net_info_add_t *netinfo)
{
    int ret = 0;
    struct netif *netif;
    netif_input_fn  input_fn;

    input_fn = tcpip_input;

    netif = mem_malloc(sizeof(struct netif));
    if (netif == NULL) {
        return -2;
    }
    memset(netif, 0, sizeof(struct netif));
    netif_add(netif, &(netinfo->ipaddr), IP4_ADDR_ANY, IP4_ADDR_ANY, NULL,
              nb_netif_init_callback, input_fn);
#if LWIP_IPV6
    if (netinfo->type == 1) {
    #if LWIP_IPV6_SEND_ROUTER_SOLICIT
        netif->rs_count = ND6_RS_COUNT;
        netif->rs_interval = ND6_RS_INTERVAL;
    #endif
        netif->ip6_autoconfig_enabled = IPV6_AUTOCONFIG_ENABLED;
        netif_create_ip6_linklocal_address(netif, 0, netinfo->ip6addr);
    }
#endif
    netif->context_id = netinfo->cid;
    netif->mtu = netinfo->mtu;

    netif_set_default(netif);
    netif_set_up(netif);
    return ret;
}

//After deactivated ps, need to inform lwip remove related context information
static int nb_netif_del_netinfo(u8_t context_id)
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

//After deactivated ps, need to remove cid and channel id mapping
static int nb_cid_del(u8_t context_id, u32_t channel_id)
{
    int ret = -1;
    nb_netif_cid_list_t *q = NULL;
    nb_netif_cid_list_t *p = g_netif_context->cid_list;
    while(p != NULL) {
        if(p->cid == context_id && p->channel_id == channel_id) {
            if(p == g_netif_context->cid_list) {
                g_netif_context->cid_list= p->next;
            } else {
                q->next = p->next;
            }
            vPortFree(p);
            ret = 0;
            break;
        }
        q = p;
        p = p->next;
    }
    return ret;
}

//Need to keep context id and channel id mapping
static int nb_cid_add(u8_t context_id, u32_t channel_id)
{
    nb_netif_cid_list_t *q = g_netif_context->cid_list;
    nb_netif_cid_list_t *p = pvPortMalloc(sizeof(nb_netif_cid_list_t));

    if(p == NULL) {
        NBNETIF_LOGI("no memory, can't add cid\n");
        return -2;
    }
    p->channel_id = channel_id;
    p->cid = context_id;
    p->next = NULL;
    if (q == NULL) {
        g_netif_context->cid_list = p;
    } else {
        while(q->next != NULL) {
            q = q->next;
        }
        q->next = p;
    }
    return 1;
}

//get cid by channel id from mapping table
static u8_t nd_get_cid_by_channel(u32_t channel_id)
{
    u8_t context_id = 0;
    nb_netif_cid_list_t *q = g_netif_context->cid_list;
    while(q != NULL) {
        if(q->channel_id == channel_id) {
            context_id = q->cid;
            break;
        }
        q = q->next;
    }
    return context_id;
}

//get channel id by context id from mapping table
static u32_t nd_get_channel_id_by_cid(u8_t context_id)
{
    u32_t channel_id = 0;
    nb_netif_cid_list_t *q = g_netif_context->cid_list;
    while(q != NULL) {
        if(q->cid== context_id) {
            channel_id = q->channel_id;
            break;
        }
        q = q->next;
    }
    return channel_id;
}

//send message to task to handle
static int nb_netif_send_message(nb_netif_msg_id_t msg_id, void *data)
{
    nb_netif_msg_t msg;

    msg.msg_id = msg_id;
    msg.msg_data = data;

    if (xQueueSend(g_netif_context->qid, &msg, portMAX_DELAY) != pdPASS) {
        NBNETIF_LOGI("queue send fail");
        return -1;
    }
    return 1;
}

//if lwip cant provide memory, need to save channel id when can receive will notify mux
int nb_resume_channel_id(void)
{
    int ret = -1;
    ret = nb_netif_send_message(MSG_ID_RESUME_CHID_ID, NULL);
    return ret;
}

//check channel id is in resume list or not
int nb_channel_is_stop_recv(u32_t channel_id)
{
    nb_resume_chid_list_t *q = g_netif_context->chid_list;
    while(q != NULL) {
        if(q->channel_id == channel_id) {
            return 1;
        }
        q = q->next;
    }
    return 0;
}

//after remove context id, will remove channel id from resume list
static int nb_rm_chid_from_resume_list(u32_t channel_id)
{
    int ret = -1;
    nb_resume_chid_list_t *q = NULL;
    nb_resume_chid_list_t *p = g_netif_context->chid_list;
    while(p != NULL) {
        if(p->channel_id == channel_id) {
            if(p == g_netif_context->chid_list) {
                g_netif_context->chid_list= p->next;
            } else {
                q->next = p->next;
            }
            vPortFree(p);
            ret = 0;
            break;
        }
        q = p;
        p = p->next;
    }
    return ret;
}


static void nb_low_level_input(struct netif *netif, struct pbuf *p)
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


static err_t nb_low_level_output(struct netif *netif, struct pbuf *p)
{
    u8_t cid = 0;
    u32_t channel_id = 0;
    mux_ap_status_t ret = MUX_AP_STATUS_OK;

    cid = netif->context_id;
    channel_id = nd_get_channel_id_by_cid(cid);
    pbuf_ref(p);
    if(p->next != NULL) {
        configASSERT(0);
    }
    ret = mux_ap_send_data(channel_id, p->payload, p->len, (void *)p);
    NBNETIF_LOGI("sent=%p,cid=%d,ret=%d", p, (int)channel_id, ret);
    if (ret != MUX_AP_STATUS_OK) {
        pbuf_free(p);
        return ERR_IF;
    }
    return ERR_OK;
}


static err_t nb_ip4_output(struct netif *netif, struct pbuf *p, const ip4_addr_t *ipaddr)
{
    return nb_low_level_output(netif, p);
}


#if LWIP_IPV6
static err_t nb_ip6_output(struct netif *netif, struct pbuf *p, const ip6_addr_t *ipaddr)
{
    return nb_low_level_output(netif, p);
}
#endif

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void nb_low_level_init_callback(struct netif *netif)
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
static err_t nb_netif_init_callback(struct netif *netif)
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
        * of bits per second.*/
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = NULL; //ethernetif;
    netif->name[0] = IFNAME00;
    netif->name[1] = IFNAME01;
    /* We directly use etharp_output() here to save a function call.
        * You can instead declare your own function an call etharp_output()
        * from it if you have to do some checks before sending (e.g. if link
        * is available...) */
    netif->output = nb_ip4_output;
#if LWIP_IPV6
    netif->output_ip6 = nb_ip6_output;
#endif /* LWIP_IPV6 */

    netif->linkoutput = (netif_linkoutput_fn)nb_low_level_output;

    //ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

    /* initialize the hardware */
    nb_low_level_init_callback(netif);

    return ERR_OK;
}

//mux callback handle
static void nb_low_level_mux_callback(mux_ap_event_t event_id, void *data)
{
    NBNETIF_LOGI("mux_id=%d", event_id);
    switch (event_id) {
        case MUX_AP_EVENT_CHANNEL_ENABLED: {
            struct netif *netif;
            u8_t cid;
            mux_ap_event_channel_enabled_t *info = (mux_ap_event_channel_enabled_t *)data;
            mux_ap_channel_id_t channel_id = info->channel_id;
            cid = nd_get_cid_by_channel(channel_id);
            if(cid != 0) {
                netif = netif_find_by_context_id(cid);
                netif_set_up(netif);
            }
            break;
        }
        case MUX_AP_EVENT_CHANNEL_DISABLED: {
            //handle channel disable by other task can't send now
            struct netif *netif;
            u8_t cid;
            mux_ap_event_channel_disabled_t *info = (mux_ap_event_channel_disabled_t *)data;
            mux_ap_channel_id_t channel_id = info->channel_id;
            cid = nd_get_cid_by_channel(channel_id);
            if(cid != 0) {
                netif = netif_find_by_context_id(cid);
                netif_set_down(netif);
            }
            break;
        }
        case MUX_AP_EVENT_SEND_COMPLETED: {
            //need to release memory
            struct pbuf *p;
            nb_resume_chid_list_t *q = g_netif_context->chid_list;
            mux_ap_event_send_completed_t *info = (mux_ap_event_send_completed_t *)data;
            p = (struct pbuf *)info->user_data;
            NBNETIF_LOGI("sent release=%p", p);
            pbuf_free(p);
            //need to check memory is enough,  need to resume mux to receive
            if(q != NULL) {
                nb_resume_channel_id();
            }
            break;
        }
        case MUX_AP_EVENT_PREPARE_TO_RECEIVE: {
            //need to allocate memory for data
            struct pbuf *p = NULL;
            if(g_netif_context->need_delay == 1) {
                //fix issue with udp get data before app create socket and drop packet
                NBNETIF_LOGI("wakeup from deep sleep, wait app ready or not\n");
                vTaskDelay(20);
                g_netif_context->need_delay = 0;
            }
            mux_ap_event_prepare_to_receive_t *info = (mux_ap_event_prepare_to_receive_t *)data;
            p = pbuf_alloc(PBUF_IP, info->buffer_length, PBUF_RAM);
            if(p != NULL) {
                NBNETIF_LOGI("recv=%p", p);
                info->data_buffer = p->payload;
                info->user_data = (void *)p;
            } else {
                int *channel_id = NULL;
                mux_ap_stop_to_receive(info->channel_id);
                if (nb_channel_is_stop_recv(info->channel_id) == 0) {
                    channel_id = pvPortMalloc(sizeof(int));
                    *channel_id = info->channel_id;
                    nb_netif_send_message(MSG_ID_RESUME_CHID_ADD,(void *)channel_id);
                }
            }
            break;
        }
        case MUX_AP_EVENT_RECEIVE_COMPLETED: {
            //send message to lwip task to handle data
            u32_t channel_id;
            u8_t cid = 0;
            struct pbuf *p;

            mux_ap_event_receive_completed_t *info = (mux_ap_event_receive_completed_t *)data;
            p = (struct pbuf *)info->user_data;
            channel_id = info->channel_id;
            cid = nd_get_cid_by_channel(channel_id);
            NBNETIF_LOGI("recved=%p,chid=%d, cid=%d", p, channel_id, cid);
            if(cid != 0) {
                struct netif *net_if = NULL;
                net_if = netif_find_by_context_id(cid);
                if(net_if != NULL) {
                    nb_low_level_input(net_if, p);
                } else {
                    pbuf_free(p);
                }
            } else {
                pbuf_free(p);
            }
            break;
        }
        default: {
            break;
        }
    }
}


static void nb_netif_task_process(void *arg)
{
    nb_netif_msg_t msg_queue_data;
    /* Loop, processing httpd cmds. */
    for (;;) {
        if (xQueueReceive(g_netif_context->qid, &msg_queue_data, portMAX_DELAY) == pdPASS) {
            NBNETIF_LOGI("msg_id=%d", msg_queue_data.msg_id);
            /* message handling */
            if (MSG_ID_BEARER_INFO_DEL == msg_queue_data.msg_id) {
                nb_net_info_del_t *data = (nb_net_info_del_t *)msg_queue_data.msg_data;
                nb_cid_del(data->cid, data->channel_id);
                nb_rm_chid_from_resume_list(data->channel_id);
                nb_netif_del_netinfo(data->cid);
                vPortFree(data);
            } else if (MSG_ID_BEARER_INFO_ADD == msg_queue_data.msg_id) {
                nb_net_info_add_t *data = (nb_net_info_add_t *)msg_queue_data.msg_data;
                nb_cid_add(data->cid, data->channel_id);
                nb_netif_add_netinfo(data);
                mux_ap_change_callback(data->channel_id, nb_low_level_mux_callback);
                vPortFree(data);
            } else if(MSG_ID_RESUME_CHID_ADD == msg_queue_data.msg_id) {
                nb_resume_chid_list_t *p = NULL;
                nb_resume_chid_list_t *q = g_netif_context->chid_list;
                int *channel_id  = (int *)msg_queue_data.msg_data;
                p = pvPortMalloc(sizeof(nb_resume_chid_list_t));
                p->channel_id = *channel_id;
                p->next = NULL;
                if(q == NULL) {
                    g_netif_context->chid_list = p;
                } else {
                    while(q->next != NULL) {
                        q = q->next;
                    }
                    q->next = p;
                }
                vPortFree(channel_id);
            } else if(MSG_ID_RESUME_CHID_ID == msg_queue_data.msg_id) {
                nb_resume_chid_list_t *q = g_netif_context->chid_list;
                while(q != NULL) {
                    nb_resume_chid_list_t *p = q;
                    mux_ap_resume_to_receive(q->channel_id);
                    q = q->next;
                    vPortFree(p);
                }
                g_netif_context->chid_list = NULL;
            }
        }
    }
}


void nb_netif_init(void)
{
    g_netif_context = pvPortMalloc(sizeof(nb_netif_context_struct));
    memset(g_netif_context, 0, sizeof(nb_netif_context_struct));
    g_netif_context->qid = xQueueCreate(NB_NETIF_QUEUE_SIZE, sizeof(nb_netif_msg_t));

    xTaskCreate(
        nb_netif_task_process,
        NB_NETIF_TASK_NAME,
        NB_NETIF_TASK_STACKSIZE / sizeof(portSTACK_TYPE),
        NULL,
        NB_NETIF_TASK_PRIO,
        NULL);

    if (rtc_power_on_result_external() == 1) {
        /* DEEP-SLEEP case: data retention process */
        NBNETIF_LOGI("wakeup from deep sleep, data retention\n");
        g_netif_context->need_delay = 1;
        nb_retention_backup_item();
    }
}

#endif
