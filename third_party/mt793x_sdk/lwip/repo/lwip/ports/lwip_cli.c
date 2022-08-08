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

#if defined(MTK_MINICLI_ENABLE)

/* standard library */
#include <stdio.h>
#include <string.h>

#include <os.h>

/* other SDK module headers */
#include "nvdm.h"
#include <toi.h>

/* LwIP headers */
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/stats.h"
#include "lwip/netif.h"

#include "lwip_cli.h"
#ifdef MTK_HOMEKIT_ENABLE
extern void wifi_conf_get_ip_from_str(unsigned char *ip_dst, const char *ip_src);
#endif



#define REQ_IP_MODE_STATIC  0
#define REQ_IP_MODE_DHCP    1


//#define DHCP_STR(flags)  (flags & NETIF_FLAG_DHCP) ?    "dhcp" : "static"
#define IP_STR(flags)    (flags & NETIF_FLAG_UP) ?      "ip " : ""
#define UPDN_STR(flags)  (flags & NETIF_FLAG_LINK_UP) ? "up " : "down "


/**
 * Consumes one parameter. If there is no more parameter, exits as fail.
 */
#define CONSUME_OR_FAIL                                                     \
    do {                                                                    \
        len--;                                                              \
        if (len == 0) {                                                     \
            printf("missing parameter\n");                                  \
            return 1;                                                       \
        }                                                                   \
        param++;                                                            \
    } while (0)


/**
 * Consumes one parameter. If there is no more parameter, exits as fail.
 */
#define CONSUME_AND_RECURSIVE(ifname, iface, len, param)                    \
    do {                                                                    \
        len--;                                                              \
        param++;                                                            \
        if (len > 0 && _cli_ip_cmds(ifname, iface, len, param)) {           \
            return 1;                                                       \
        }                                                                   \
    } while (0)


/**
 * Macro to perform string comparison to make the code looks better.
 */
#define PARAM_IS(str)   (!strcmp(str, param[0]))


/****************************************************************************
 *
 * Types.
 *
 ****************************************************************************/


/****************************************************************************
 *
 * Static variables.
 *
 ****************************************************************************/


unsigned int lwip_debug_log = 0;

#if defined(MTK_LWIP_DYNAMIC_DEBUG_ENABLE)
struct lwip_debug_flags lwip_debug_flags[] =
{
	[LWIP_DEBUG_IDX(ETHARP_DEBUG)]     = { "ETHARP_DEBUG",     LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(NETIF_DEBUG)]      = { "NETIF_DEBUG",      LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(PBUF_DEBUG)]       = { "PBUF_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(API_LIB_DEBUG)]    = { "API_LIB_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(API_MSG_DEBUG)]    = { "API_MSG_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(SOCKETS_DEBUG)]    = { "SOCKETS_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(ICMP_DEBUG)]       = { "ICMP_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(IGMP_DEBUG)]       = { "IGMP_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(INET_DEBUG)]       = { "INET_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(IP_DEBUG)]         = { "IP_DEBUG",         LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(IP_REASS_DEBUG)]   = { "IP_REASS_DEBUG",   LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(RAW_DEBUG)]        = { "RAW_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(MEM_DEBUG)]        = { "MEM_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(MEMP_DEBUG)]       = { "MEMP_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(SYS_DEBUG)]        = { "SYS_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TIMERS_DEBUG)]     = { "TIMERS_DEBUG",     LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_DEBUG)]        = { "TCP_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_INPUT_DEBUG)]  = { "TCP_INPUT_DEBUG",  LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_FR_DEBUG)]     = { "TCP_FR_DEBUG",     LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_RTO_DEBUG)]    = { "TCP_RTO_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_CWND_DEBUG)]   = { "TCP_CWND_DEBUG",   LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_WND_DEBUG)]    = { "TCP_WND_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_OUTPUT_DEBUG)] = { "TCP_OUTPUT_DEBUG", LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_RST_DEBUG)]    = { "TCP_RST_DEBUG",    LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCP_QLEN_DEBUG)]   = { "TCP_QLEN_DEBUG",   LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(UDP_DEBUG)]        = { "UDP_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(TCPIP_DEBUG)]      = { "TCPIP_DEBUG",      LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(PPP_DEBUG)]        = { "PPP_DEBUG",        LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(SLIP_DEBUG)]       = { "SLIP_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(DHCP_DEBUG)]       = { "DHCP_DEBUG",       LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(AUTOIP_DEBUG)]     = { "AUTOIP_DEBUG",     LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(SNMP_MSG_DEBUG)]   = { "SNMP_MSG_DEBUG",   LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(SNMP_MIB_DEBUG)]   = { "SNMP_MIB_DEBUG",   LWIP_DBG_OFF },
	[LWIP_DEBUG_IDX(DNS_DEBUG)]        = { "DNS_DEBUG",        LWIP_DBG_OFF },
	{ NULL, 0 }
};
#endif

/****************************************************************************
 *
 * Local functions.
 *
 ****************************************************************************/
#if LWIP_STATS_DISPLAY
extern pbuf_table buf_tb[50];
extern int alloc_count, free_count;
#endif
uint8_t _show_lwip_stat(uint8_t len, char *param[])
{
#if LWIP_STATS_DISPLAY
    uint32_t    stats_config = 0;

    if (atoi(param[0]) == 0) {
        printf("1:    TCP\n");
        printf("2:    UDP\n");
        printf("4:    ICMP\n");
        printf("8:    IGMP\n");
        printf("16:   IPFRAG\n");
        printf("32:   IP\n");
        printf("64:   ETHARP\n");
        printf("128:  LINK\n");
        printf("256:  MEM\n");
        printf("512:  MEMP\n");
        printf("1024: SYS\n");
        printf("2048: PBUF\n");
    }

    stats_config = atoi(param[0]);
#if TCP_STATS
    if (stats_config & (1 << 0))
        TCP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 0))
        printf("Please open the TCP_STATS Macro before compile!!!\n");
#endif

#if UDP_STATS
    if (stats_config & (1 << 1))
        UDP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 1))
        printf("Please open the UDP_STATS Macro before compile!!!\n");
#endif

#if ICMP_STATS
    if (stats_config & (1 << 2))
        ICMP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 2))
        printf("Please open the ICMP_STATS Macro before compile!!!\n");
#endif

#if  IGMP_STATS
    if (stats_config & (1 << 3))
        IGMP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 3))
        printf("Please open the IGMP_STATS Macro before compile!!!\n");
#endif

#if IPFRAG_STATS
    if (stats_config & (1 << 4))
        IPFRAG_STATS_DISPLAY();
#else
    if (stats_config & (1 << 4))
        printf("Please open the IPFRAG_STATS Macro before compile!!!\n");
#endif

#if IP_STATS
    if (stats_config & (1 << 5))
        IP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 5))
        printf("Please open the IP_STATS Macro before compile!!!\n");
#endif

#if ETHARP_STATS
    if (stats_config & (1 << 6))
        ETHARP_STATS_DISPLAY();
#else
    if (stats_config & (1 << 6))
        printf("Please open the ETHARP_STATS Macro before compile!!!\n");
#endif

#if LINK_STATS
    if (stats_config & (1 << 7))
        LINK_STATS_DISPLAY();
#else
    if (stats_config & (1 << 7))
        printf("Please open the LINK_STATS Macro before compile!!!\n");
#endif

#if MEM_STATS
    if (stats_config & (1 << 8))
        MEM_STATS_DISPLAY();
#else
    if (stats_config & (1 << 8))
        printf("Please open the MEM_STATS Macro before compile!!!\n");
#endif

#if MEMP_STATS
    uint32_t    i;
    if (stats_config & (1 << 9)) {
        for (i = 0; i < MEMP_MAX; i++) {
            MEMP_STATS_DISPLAY(i);
        }
    }
#else
    if (stats_config & (1 << 9))
        printf("Please open the MEMP_STATS Macro before compile!!!\n");
#endif

#if SYS_STATS
    if (stats_config & (1 << 10))
        SYS_STATS_DISPLAY();
#else
    if (stats_config & (1 << 10))
        printf("Please open the SYS_STATS Macro before compile!!!\n");
#endif

    if (stats_config & (1 << 11)){
        printf("\nPBUF\n");
        //printf("      alloc_count: %d\n",__pbufAllocCount);
        //printf("      free_count: -%d\n",__pbufFreeCount);
        for(uint8_t i = 0; i<50; i++) {
            if(buf_tb[i].used == 1) {
                printf("BUF_TB-ID:%d,file:%s,line:%u,ptr:0x%x,size:%u\n",
                    buf_tb[i].id,buf_tb[i].file,(unsigned int)buf_tb[i].line,(unsigned int)buf_tb[i].ptr,(unsigned int)buf_tb[i].size);
            }
        }
        printf("BUF_TB-alloc:%u,free:%u.Done\n",(unsigned int)alloc_count, (unsigned int)free_count);
    }

#else /*LWIP_STATS_DISPLAY*/
    printf("Please open the LWIP_STATS_DISPLAY Macro before compile!!!\n");
#endif
    return 0;
}


static uint8_t _cli_ip_nvram_set_key(const char *ifname,
                                     const char *item,
                                     const char *value)
{
#ifdef MTK_NVDM_ENABLE
    const char *key;
    nvdm_status_t status;
    if (!strcmp(item, "ip")) {
        key = "IpAddr";
    } else if (!strcmp(item, "mask")) {
        key = "IpNetmask";
    } else if (!strcmp(item, "gw")) {
        key = "IpGateway";
    } else if (!strcmp(item, "mode")) {
        key = "IpMode";
    } else {
        printf("unknown config item %s\n", item);
        return 1;
    }
    if (!strcmp(ifname, "st2")) {
        status = nvdm_write_data_item("network", key, NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)value, strlen(value));
		if (NVDM_STATUS_OK != status) {
			printf("save config failed %s\n", item);
			return 1;
		}
    } else if (!strcmp(ifname, "ap1")) {
        status = nvdm_write_data_item("network", key, NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)value, strlen(value));
		if (NVDM_STATUS_OK != status) {
			printf("save config failed %s\n", item);
			return 1;
		}
    } else {
        return 1;
    }
#endif
    return 0;
}


static void _cli_ip_nvram_commit(const char *ifname)
{
	return;
}


static int8_t _cli_ip_show(char *ifname)
{
    struct netif    *iface = netif_find(ifname);

    if (!iface) {
        return -1;
    }

    printf("\ninterface: %s\n", ifname);
#if LWIP_DHCP
        if (dhcp_supplied_address(iface))
        	printf("mode:      dhcp\n");
        else
                printf("mode:      static\n");
#endif

#if LWIP_DHCP
    if (dhcp_supplied_address(iface)) {
        struct dhcp *d = netif_dhcp_data(iface);
        printf("dhcp:\n");
        printf("  ip      %s\n", ip4addr_ntoa(&d->offered_ip_addr));
        printf("  netmask %s\n", ip4addr_ntoa(&d->offered_sn_mask));
        printf("  gateway %s\n", ip4addr_ntoa(&d->offered_gw_addr));
    }
    else
#endif
    {
        printf("static:\n");
        printf("  ip      %s\n", ip4addr_ntoa(ip_2_ip4(&iface->ip_addr)));
        printf("  netmask %s\n", ip4addr_ntoa(ip_2_ip4(&iface->netmask)));
        printf("  gateway %s\n", ip4addr_ntoa(ip_2_ip4(&iface->gw)));
    }
    return 0;
}


/**
 * Process interface commands from user.
 *
 * This function is recursive such that user can input commands consecutively.
 *
 * @retval 1    if fail when processing following subcommands.
 * @retval 0    if succeeded when processing following subcommands.
 */
static uint8_t _cli_ip_cmds(
    char            *ifname,
    struct netif    *iface,
    uint8_t         len,
    char            *param[])
{
    if (PARAM_IS("mode")) {
        uint8_t     mode;
        const char  *item;

        item = param[0];

        CONSUME_OR_FAIL;

        if (!PARAM_IS("dhcp") && !PARAM_IS("static")) {
            printf("mode must be static/dhcp\n");
            return 1;
        }

        mode = PARAM_IS("dhcp") ? REQ_IP_MODE_DHCP : REQ_IP_MODE_STATIC;

        CONSUME_AND_RECURSIVE(ifname, iface, len, param);
    #if LWIP_DHCP
        if (mode == REQ_IP_MODE_DHCP) {
            err_t   err = dhcp_start(iface);
            if (err != ERR_OK) {
                printf("start DHCP client failed (%d)\n", err);
            }
        } else {
            dhcp_stop(iface);
        }
    #endif
        _cli_ip_nvram_set_key(ifname, item,
                              (mode == REQ_IP_MODE_DHCP) ? "dhcp" : "static");

    } else if (PARAM_IS("ip") || PARAM_IS("mask") || PARAM_IS("gw")) {

        ip4_addr_t   v4;
        void        (*_ip_set)(struct netif *netif, const ip4_addr_t *ipaddr);
        const char  *value;
        const char  *item;

        if (PARAM_IS("ip")) {
            _ip_set = netif_set_ipaddr;
        } else if (PARAM_IS("mask")) {
            _ip_set = netif_set_netmask;
        } else {
            _ip_set = netif_set_gw;
        }

        item = param[0];

        CONSUME_OR_FAIL;

        if (ip4addr_aton(param[0], &v4) == 0) {
            printf("invalid address: %s\n", param[0]);
            return 1;
        }

        value = param[0];

        CONSUME_AND_RECURSIVE(ifname, iface, len, param);

        _ip_set(iface, (const ip4_addr_t *)&v4);
        _cli_ip_nvram_set_key(ifname, item, value);

    } else {
        printf("invalid command: %s\n", param[0]);
        return 1;
    }

    return 0;
}

#if defined(MTK_LWIP_DYNAMIC_DEBUG_ENABLE)
static uint8_t _set_debug_flag(uint8_t len, char *param[])
{
	uint8_t i = 0;

	for (i = 0; i < len; i++)
	{
		lwip_debug_flags[atoi(param[i])].debug_flag |= LWIP_DBG_ON;
	}

	return 0;
}

static uint8_t _clear_debug_flag(uint8_t len, char *param[])
{
	uint8_t i = 0;

	cli_puts("clear debug flags:\n");

	for (i = 0; i < len; i++)
	{
		lwip_debug_flags[atoi(param[i])].debug_flag &= ~LWIP_DBG_ON;
		printf("%d: %s\n", atoi(param[i]), lwip_debug_flags[atoi(param[i])].debug_flag_name);
	}

	return 0;
}

static uint8_t _show_all_debug_flag(uint8_t len, char *param[])
{
	uint8_t i = 0;

	cli_puts("show all debug flag:\n");

	for (i = 0; lwip_debug_flags[i].debug_flag_name != NULL; i++) {
		printf("%d: %s %s\n", i,
                              lwip_debug_flags[i].debug_flag_name,
                              (lwip_debug_flags[i].debug_flag & LWIP_DBG_ON) ?
                                    "on" : "off");
	}

	return 0;
}
#else

static uint8_t _show_all_debug_flag(uint8_t len, char *param[]) {return 0;}
static uint8_t _set_debug_flag(uint8_t len, char *param[])  {return 0;}
static uint8_t _clear_debug_flag(uint8_t len, char *param[])  {return 0;}

#endif

#ifdef MTK_HOMEKIT_ENABLE
static uint8_t lwip_cli_set_ip(uint8_t len, char *param[])
{
    unsigned char addr_array[4];
    unsigned char mask_array[4];
    unsigned char gw_array[4];
	ip4_addr_t   ipaddr, netmask, gw;
	struct netif *net_if = netif_find("st2");

	if (len < 3)
	{
		printf("usage : l set addr mask gw\n");
		return -1;
	}

    wifi_conf_get_ip_from_str(addr_array, param[0]);
    wifi_conf_get_ip_from_str(mask_array, param[1]);
    wifi_conf_get_ip_from_str(gw_array, param[2]);
    IP4_ADDR(&ipaddr, addr_array[0], addr_array[1], addr_array[2], addr_array[3]);
    IP4_ADDR(&netmask, mask_array[0], mask_array[1], mask_array[2], mask_array[3]);
    IP4_ADDR(&gw, gw_array[0], gw_array[1], gw_array[2], gw_array[3]);

    netif_set_addr(net_if, (ip4_addr_t *)&addr_array, (ip4_addr_t *)&mask_array, (ip4_addr_t *)&gw_array);

	return 0;
}
#endif


/****************************************************************************
 *
 * API variable.
 *
 ****************************************************************************/


cmd_t   lwip_cli[] = {
    { "all",  "show all debug flag",                    _show_all_debug_flag },
    { "on",   "set all debug flag",                     _set_debug_flag      },
    { "off",  "clr all debug flag",                     _clear_debug_flag    },
    { "stat", "show statistics",                        _show_lwip_stat      },
#ifdef MTK_HOMEKIT_ENABLE
    { "set",  "set addr mask gw",                        lwip_cli_set_ip     },
#endif
    { NULL }
};


/****************************************************************************
 *
 * API functions.
 *
 ****************************************************************************/


/**
 * IP configuration handler function.
 *
 * @todo Using st0-2, ap0-2, lo0-2 hardcode now because LwIP does not have
 *       any API to get the list of netif. It must be searched with exact
 *       name.
 */
uint8_t lwip_ip_cli(uint8_t len, char *param[])
{
    if (len == 0) {
        _cli_ip_show("lo0");
        _cli_ip_show("st1");
        _cli_ip_show("ap2");
    } else if (len == 1) {
        if (_cli_ip_show(param[0]) < 0) {
            printf("interface %s not found\n", param[0]);
            return 1;
        }
    } else {
        char            *ifname = param[0];
        struct netif    *iface;

        iface = netif_find(ifname);
        if (!iface) {
            printf("interface %s not found\n", param[0]);
            return 1;
        }

        CONSUME_OR_FAIL;

        if (_cli_ip_cmds(ifname, iface, len, param) == 0) {
            _cli_ip_nvram_commit(ifname);
        }
    }

    return 0;
}

#ifdef MTK_TCP_PORT_MONITOR_ENABLE
#include "lwip/apps/port_monitor.h"

static uint8_t _port_cli_insert_port(uint8_t len, char *param[])
{
    u16_t block_port;
    int ret;

    if (len == 1) {
        block_port = (u16_t)atoi(param[0]);

        ret = port_monitor_config_insert_port(block_port);
        if (ret == 0) {
            printf("insert port <%d> to list success\r\n", block_port);
        }
        else {
            printf("insert port <%d> to list fail\r\n", block_port);
        }
    }
    else {
        printf("port add <port>\r\n");
    }

    return 0;
}

static uint8_t _port_cli_remove_port(uint8_t len, char *param[])
{
    u16_t block_port;
    int ret;
    
    if (len == 1) {
        block_port = (u16_t)atoi(param[0]);

        ret = port_monitor_config_remove_port(block_port);
        if (ret == 0) {
            printf("remove port <%d> success\r\n", block_port);
        }
        else {
            printf("remove port <%d> fail\r\n", block_port);
        }
    }
    else {
        printf("port del <port>\r\n");
    }

    return 0;
}

static uint8_t _port_cli_get_port(uint8_t len, char *param[])
{
    port_monitor_port_t *cur_port;
    int i = 0;

    cur_port = port_monitor_get_port_list();
    if (cur_port) {
        do {
            i++;
            printf("%02d: port <%d>\r\n", i, cur_port->port);
            cur_port = cur_port->next;
        }while (cur_port);
    }
    else {
        printf("port list empty\r\n");
    }

    return 0;
}

static uint8_t _port_cli_packet_logging_enable(uint8_t len, char *param[])
{
    u8_t logging_enable;
    int ret;

    if (len == 1) {
        logging_enable = (u8_t)atoi(param[0]);

        port_monitor_config_logging_enable(logging_enable);
        if (logging_enable) {
            printf("enable packet logging\r\n");
        }
        else {
            printf("disable packet logging\r\n");
        }
    }
    else {
        printf("port packet enable <0 or 1>\r\n");
    }

    return 0;
}

static uint8_t _port_cli_packets_clear(uint8_t len, char *param[])
{
    port_monitor_clear_logging_packets();
    printf("packets cleared\r\n");

    return 0;
}

static uint8_t _port_cli_packets_get(uint8_t len, char *param[])
{
    int i = 0;
    port_monitor_packet_t *cur_pkt;
    cur_pkt = port_monitor_get_logging_packets();
    if (cur_pkt) {
        do {
            i++;
            printf("%02d: port <%d>, syn <%d>, payload len <%d>\r\n", 
                i, 
                cur_pkt->dest_port,
                cur_pkt->syn_flag,
                cur_pkt->len);
            cur_pkt = cur_pkt->next;
        }while (cur_pkt);
    }
    else {
        printf("packets list empty\r\n");
    }

    return 0;
}

cmd_t port_packet_cli[] = {
    { "enable",   "enable or disable packet logging",  _port_cli_packet_logging_enable,  NULL },
    { "clear",   "clear packets",  _port_cli_packets_clear,  NULL },
    { "get",   "get packets",  _port_cli_packets_get,  NULL },
    { NULL, NULL, NULL, NULL}
};


cmd_t port_cli[] = {
    { "add",   "add new port to blocked port list",  _port_cli_insert_port,  NULL },
    { "del",   "remove a port from blocked port list",  _port_cli_remove_port,  NULL },
    { "get",   "get blocked port list",  _port_cli_get_port,  NULL },
    { "packet",   "port packet config, get or clear",  NULL,  port_packet_cli },
    { NULL, NULL, NULL, NULL}
};

#endif /* MTK_TCP_PORT_MONITOR_ENABLE */

#endif

