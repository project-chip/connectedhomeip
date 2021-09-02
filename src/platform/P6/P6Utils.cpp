/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          General utility methods for the P6 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <cy_lwip.h>
#include <cy_wcm.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/P6/P6Utils.h>

#include "lwip/icmp.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/prot/ip4.h"
#include "lwip/raw.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY 2000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID 0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 64
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 5000
#endif

/* Ping IP Header len for IPv4 */
#define IP_HDR_LEN 20

/* Ping Response length */
#define PING_RESPONSE_LEN 64

/* Enable Ping via Socket API or RAW API */
#define PING_USE_SOCKETS 1

namespace {
wifi_config_t wifi_conf;
wifi_mode_t WiFiMode;
bool wcm_init_done;
/* ping variables */
const ip_addr_t * ping_target;
u16_t ping_seq_num;
u32_t ping_time;
#if !PING_USE_SOCKETS
struct raw_pcb * ping_pcb;
#endif /* PING_USE_SOCKETS */
} // namespace

typedef struct
{
    struct icmp_echo_hdr hdr;
    uint8_t data[PING_DATA_SIZE];
} icmp_packet_t;

CHIP_ERROR P6Utils::IsAPEnabled(bool & apEnabled)
{
    apEnabled = (WiFiMode == WIFI_MODE_AP || WiFiMode == WIFI_MODE_APSTA);
    return CHIP_NO_ERROR;
}

bool P6Utils::IsStationProvisioned(void)
{
    wifi_config_t stationConfig;
    return (p6_wifi_get_config(WIFI_IF_STA, &stationConfig) == CY_RSLT_SUCCESS &&
            strlen((const char *) stationConfig.sta.ssid) != 0);
}

CHIP_ERROR P6Utils::IsStationConnected(bool & connected)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    connected      = cy_wcm_is_connected_to_ap();
    return err;
}

CHIP_ERROR P6Utils::StartWiFiLayer(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_wcm_config_t wcm_config;

    wcm_config.interface = CY_WCM_INTERFACE_TYPE_AP_STA;
    ChipLogProgress(DeviceLayer, "Starting P6 WiFi layer");

    if (wcm_init_done == false)
    {
        result = cy_wcm_init(&wcm_config);
        if (result != CY_RSLT_SUCCESS)
        {
            err = CHIP_ERROR_INTERNAL;
            ChipLogError(DeviceLayer, "StartWiFiLayer() P6 Wi-Fi Started Failed: %s", chip::ErrorStr(err));
            SuccessOrExit(err);
        }
        wcm_init_done = true;
    }

exit:
    return err;
}

CHIP_ERROR P6Utils::EnableStationMode(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "EnableStationMode");
    /* If Station Mode is already set , update Mode to APSTA Mode */
    if (WiFiMode == WIFI_MODE_AP)
    {
        WiFiMode = WIFI_MODE_APSTA;
    }
    else
    {
        WiFiMode = WIFI_MODE_STA;
    }
    return err;
}

CHIP_ERROR P6Utils::SetAPMode(bool enabled)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "SetAPMode");
    /* If AP Mode is already set , update Mode to APSTA Mode */
    if (enabled)
    {
        if (WiFiMode == WIFI_MODE_STA)
        {
            WiFiMode = WIFI_MODE_APSTA;
        }
        else
        {
            WiFiMode = WIFI_MODE_AP;
        }
    }
    else
    {
        if (WiFiMode == WIFI_MODE_APSTA)
        {
            WiFiMode = WIFI_MODE_STA;
        }
        else if (WiFiMode == WIFI_MODE_AP)
        {
            WiFiMode = WIFI_MODE_NULL;
        }
    }
    return err;
}

const char * P6Utils::WiFiModeToStr(wifi_mode_t wifiMode)
{
    switch (wifiMode)
    {
    case WIFI_MODE_NULL:
        return "NULL";
    case WIFI_MODE_STA:
        return "STA";
    case WIFI_MODE_AP:
        return "AP";
    case WIFI_MODE_APSTA:
        return "STA+AP";
    default:
        return "(unknown)";
    }
}
cy_rslt_t P6Utils::p6_wifi_set_config(wifi_interface_t interface, wifi_config_t * conf)
{
    if (interface == WIFI_IF_STA)
    {
        populate_wifi_config_t(&wifi_conf, interface, &conf->sta.ssid, &conf->sta.password, conf->sta.security);
        ChipLogProgress(DeviceLayer, "STA %s : [%s] [%s] \r\n", __func__, wifi_conf.sta.ssid, wifi_conf.sta.password);
    }
    else
    {
        populate_wifi_config_t(&wifi_conf, interface, &conf->ap.ssid, &conf->ap.password, conf->ap.security);
        wifi_conf.ap.channel                = conf->ap.channel;
        wifi_conf.ap.ip_settings.ip_address = conf->ap.ip_settings.ip_address;
        wifi_conf.ap.ip_settings.netmask    = conf->ap.ip_settings.netmask;
        wifi_conf.ap.ip_settings.gateway    = conf->ap.ip_settings.gateway;
        ChipLogProgress(DeviceLayer, "AP %s : [%s] [%s] channel %d\r\n", __func__, wifi_conf.ap.ssid, wifi_conf.ap.password,
                        wifi_conf.ap.channel);
    }
    return CY_RSLT_SUCCESS;
}

cy_rslt_t P6Utils::p6_wifi_get_config(wifi_interface_t interface, wifi_config_t * conf)
{
    if (interface == WIFI_IF_STA)
    {
        populate_wifi_config_t(conf, interface, &wifi_conf.sta.ssid, &wifi_conf.sta.password, wifi_conf.sta.security);
    }
    else
    {
        populate_wifi_config_t(conf, interface, &wifi_conf.ap.ssid, &wifi_conf.ap.password, wifi_conf.ap.security);
        conf->ap.channel                = wifi_conf.ap.channel;
        conf->ap.ip_settings.ip_address = wifi_conf.ap.ip_settings.ip_address;
        conf->ap.ip_settings.netmask    = wifi_conf.ap.ip_settings.netmask;
        conf->ap.ip_settings.gateway    = wifi_conf.ap.ip_settings.gateway;
        ChipLogProgress(DeviceLayer, "AP %s [%s] [%s] channel %d\r\n", __func__, wifi_conf.ap.ssid, wifi_conf.ap.password,
                        wifi_conf.ap.channel);
    }
    return CY_RSLT_SUCCESS;
}

CHIP_ERROR P6Utils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    wifi_config_t stationConfig;

    result = p6_wifi_get_config(WIFI_IF_STA, &stationConfig);
    VerifyOrExit(result == CY_RSLT_SUCCESS, err = CHIP_ERROR_INTERNAL);

    ChipLogProgress(DeviceLayer, "GetWiFiStationProvision");
    VerifyOrExit(strlen((const char *) stationConfig.sta.ssid) != 0, err = CHIP_ERROR_INCORRECT_STATE);

    netInfo.NetworkId              = kWiFiStationNetworkId;
    netInfo.FieldPresent.NetworkId = true;
    memcpy(netInfo.WiFiSSID, stationConfig.sta.ssid,
           min(strlen(reinterpret_cast<char *>(stationConfig.sta.ssid)) + 1, sizeof(netInfo.WiFiSSID)));

    // Enforce that netInfo wifiSSID is null terminated
    netInfo.WiFiSSID[kMaxWiFiSSIDLength] = '\0';

    if (includeCredentials)
    {
        static_assert(sizeof(netInfo.WiFiKey) < 255, "Our min might not fit in netInfo.WiFiKeyLen");
        netInfo.WiFiKeyLen = static_cast<uint8_t>(min(strlen((char *) stationConfig.sta.password), sizeof(netInfo.WiFiKey)));
        memcpy(netInfo.WiFiKey, stationConfig.sta.password, netInfo.WiFiKeyLen);
    }

exit:
    return err;
}

CHIP_ERROR P6Utils::SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    wifi_config_t wifiConfig;
    ChipLogProgress(DeviceLayer, "SetWiFiStationProvision");
    char wifiSSID[kMaxWiFiSSIDLength + 1];
    size_t netInfoSSIDLen = strlen(netInfo.WiFiSSID);

    // Ensure that P6 station mode is enabled.  This is required before p6_wifi_set_config
    // can be called.
    err = P6Utils::EnableStationMode();
    SuccessOrExit(err);

    // Enforce that wifiSSID is null terminated before copying it
    memcpy(wifiSSID, netInfo.WiFiSSID, min(netInfoSSIDLen + 1, sizeof(wifiSSID)));
    if (netInfoSSIDLen + 1 < sizeof(wifiSSID))
    {
        wifiSSID[netInfoSSIDLen] = '\0';
    }
    else
    {
        wifiSSID[kMaxWiFiSSIDLength] = '\0';
    }

    // Initialize an P6 wifi_config_t structure based on the new provision information.
    populate_wifi_config_t(&wifiConfig, WIFI_IF_STA, (cy_wcm_ssid_t *) wifiSSID, (cy_wcm_passphrase_t *) netInfo.WiFiKey);

    // Configure the P6 WiFi interface.
    result = p6_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "p6_wifi_set_config() failed ");
        err = CHIP_ERROR_INTERNAL;
    }
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "WiFi station provision set (SSID: %s)", netInfo.WiFiSSID);

exit:
    return err;
}

CHIP_ERROR P6Utils::ClearWiFiStationProvision(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t stationConfig;
    ChipLogProgress(DeviceLayer, "ClearWiFiStationProvision");
    // Clear the P6 WiFi station configuration.
    memset(&stationConfig.sta, 0, sizeof(stationConfig.sta));
    p6_wifi_set_config(WIFI_IF_STA, &stationConfig);
    return err;
}

CHIP_ERROR P6Utils::p6_wifi_disconnect(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    ChipLogProgress(DeviceLayer, "p6_wifi_disconnect");
    result = cy_wcm_disconnect_ap();
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "p6_wifi_disconnect() failed result %ld", result);
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}

CHIP_ERROR P6Utils::p6_wifi_connect(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    wifi_config_t stationConfig;
    cy_wcm_connect_params_t connect_param;
    cy_wcm_ip_address_t ip_addr;

    p6_wifi_get_config(WIFI_IF_STA, &stationConfig);
    memset(&connect_param, 0, sizeof(cy_wcm_connect_params_t));
    memset(&ip_addr, 0, sizeof(cy_wcm_ip_address_t));
    memcpy(&connect_param.ap_credentials.SSID, &stationConfig.sta.ssid, strlen((char *) stationConfig.sta.ssid));
    memcpy(&connect_param.ap_credentials.password, &stationConfig.sta.password, strlen((char *) stationConfig.sta.password));
    connect_param.ap_credentials.security = stationConfig.sta.security;

    ChipLogProgress(DeviceLayer, "p6_wifi_connect ssid %s pass %s sec %d \r\n", connect_param.ap_credentials.SSID,
                    connect_param.ap_credentials.password, connect_param.ap_credentials.security);

    result = cy_wcm_connect_ap(&connect_param, &ip_addr);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "p6_wifi_connect() failed result %ld", result);
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}

#define INITIALISER_IPV4_ADDRESS1(addr_var, addr_val) addr_var = { CY_WCM_IP_VER_V4, { .v4 = (uint32_t)(addr_val) } }
#define MAKE_IPV4_ADDRESS1(a, b, c, d) ((((uint32_t) d) << 24) | (((uint32_t) c) << 16) | (((uint32_t) b) << 8) | ((uint32_t) a))
static const cy_wcm_ip_setting_t ap_mode_ip_settings2 = {
    INITIALISER_IPV4_ADDRESS1(.ip_address, MAKE_IPV4_ADDRESS1(192, 168, 0, 2)),
    INITIALISER_IPV4_ADDRESS1(.gateway, MAKE_IPV4_ADDRESS1(192, 168, 0, 2)),
    INITIALISER_IPV4_ADDRESS1(.netmask, MAKE_IPV4_ADDRESS1(255, 255, 255, 0)),
};

CHIP_ERROR P6Utils::p6_start_ap(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    wifi_config_t stationConfig;
    memset(&stationConfig, 0, sizeof(stationConfig));
    p6_wifi_get_config(WIFI_IF_AP, &stationConfig);

    cy_wcm_ap_config_t ap_conf;
    memset(&ap_conf, 0, sizeof(cy_wcm_ap_config_t));
    memcpy(ap_conf.ap_credentials.SSID, &stationConfig.ap.ssid, strlen((const char *) stationConfig.ap.ssid));
    memcpy(ap_conf.ap_credentials.password, &stationConfig.ap.password, strlen((const char *) stationConfig.ap.password));
    memcpy(&ap_conf.ip_settings, &stationConfig.ap.ip_settings, sizeof(stationConfig.ap.ip_settings));
    ap_conf.ap_credentials.security = stationConfig.ap.security;
    ap_conf.channel                 = stationConfig.ap.channel;
    ChipLogProgress(DeviceLayer, "p6_start_ap %s %s \r\n", ap_conf.ap_credentials.SSID, ap_conf.ap_credentials.password);

    /* Start AP */
    result = cy_wcm_start_ap(&ap_conf);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "cy_wcm_start_ap() failed result %ld", result);
        err = CHIP_ERROR_INTERNAL;
    }
    /* Link Local IPV6 AP address for AP */
    cy_wcm_ip_address_t ipv6_addr;
    result = cy_wcm_get_ipv6_addr(CY_WCM_INTERFACE_TYPE_AP, CY_WCM_IPV6_LINK_LOCAL, &ipv6_addr, 1);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "cy_wcm_get_ipv6_addr() failed result %ld", result);
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}

CHIP_ERROR P6Utils::p6_stop_ap(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    /* Stop AP */
    result = cy_wcm_stop_ap();
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "cy_wcm_stop_ap failed result %ld", result);
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}

void P6Utils::populate_wifi_config_t(wifi_config_t * wifi_config, wifi_interface_t interface, const cy_wcm_ssid_t * ssid,
                                     const cy_wcm_passphrase_t * password, cy_wcm_security_t security)
{
    CY_ASSERT(wifi_config != NULL);

    // Use interface param to determine which config to fill out
    if (interface == WIFI_IF_STA || interface == WIFI_IF_STA_AP)
    {
        memset(&wifi_config->sta, 0, sizeof(wifi_config_sta_t));
        memcpy(wifi_config->sta.ssid, ssid, chip::min(strlen((char *) ssid) + 1, sizeof(cy_wcm_ssid_t)));
        memcpy(wifi_config->sta.password, password, chip::min(strlen((char *) password) + 1, sizeof(cy_wcm_ssid_t)));
        wifi_config->sta.security = security;
    }

    if (interface == WIFI_IF_AP || interface == WIFI_IF_STA_AP)
    {
        memset(&wifi_config->ap, 0, sizeof(wifi_config_ap_t));
        memcpy(wifi_config->ap.ssid, ssid, chip::min(strlen((char *) ssid) + 1, sizeof(cy_wcm_ssid_t)));
        memcpy(wifi_config->ap.password, password, chip::min(strlen((char *) password) + 1, sizeof(cy_wcm_ssid_t)));
        wifi_config->ap.security = security;
    }
}

/* Ping implementation
 *
 */

static void print_ip4(uint32_t ip)
{
    unsigned int bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    printf("Addr = %d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
}

static void ping_prepare_echo(icmp_packet_t * iecho, uint16_t len)
{
    int i;
    ICMPH_TYPE_SET(&iecho->hdr, ICMP_ECHO);
    ICMPH_CODE_SET(&iecho->hdr, 0);
    iecho->hdr.chksum = 0;
    iecho->hdr.id     = PING_ID;
    iecho->hdr.seqno  = htons(++(ping_seq_num));

    /* fill the additional data buffer with some data */
    for (i = 0; i < (int) sizeof(iecho->data); i++)
    {
        iecho->data[i] = (uint8_t) i;
    }

    iecho->hdr.chksum = inet_chksum(iecho, len);
}

/* Ping using socket API */
#if PING_USE_SOCKETS

static err_t ping_send(int s, const ip_addr_t * addr)
{
    int err;
    icmp_packet_t iecho;
    struct sockaddr_in to;

    ping_prepare_echo(&iecho, (u16_t) sizeof(icmp_packet_t));

    printf("\r\nPinging to Gateway ");
    print_ip4(addr->u_addr.ip4.addr);

    /* Send the ping request */
    to.sin_len    = sizeof(to);
    to.sin_family = AF_INET;
    inet_addr_from_ip4addr(&to.sin_addr, ip_2_ip4(addr));
    err = lwip_sendto(s, &iecho, sizeof(icmp_packet_t), 0, (struct sockaddr *) &to, sizeof(to));

    return (err ? ERR_OK : ERR_VAL);
}

static void ping_recv(int s)
{
    char buf[PING_RESPONSE_LEN];
    int fromlen;
    int len;
    struct sockaddr_in from;
    struct ip_hdr * iphdr;
    struct icmp_echo_hdr * iecho;

    do
    {
        len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &from, (socklen_t *) &fromlen);
        if (len >= (int) (sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
        {
            iphdr = (struct ip_hdr *) buf;
            iecho = (struct icmp_echo_hdr *) (buf + (IPH_HL(iphdr) * 4));

            if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)) && (ICMPH_TYPE(iecho) == ICMP_ER))
            {
                printf("Ping was successful Elapsed time : %" U32_F " ms\n", sys_now() - ping_time);
                return; /* Echo reply received - return success */
            }
        }
    } while (len > 0);

    if (len == 0)
    {
        printf("ping: recv - %" U32_F " ms - timeout\r\n", (sys_now() - ping_time));
    }
}

static void ping_socket()
{
    int s;
    int ret;

#if LWIP_SO_SNDRCVTIMEO_NONSTANDARD
    int timeout = PING_RCV_TIMEO;
#else
    struct timeval timeout;
    timeout.tv_sec  = PING_RCV_TIMEO / 1000;
    timeout.tv_usec = (PING_RCV_TIMEO % 1000) * 1000;
#endif

    s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
    if (s < 0)
    {
        return;
    }

    ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    LWIP_ASSERT("setting receive timeout failed", ret == 0);
    LWIP_UNUSED_ARG(ret);

    while (1)
    {
        if (ping_send(s, ping_target) == ERR_OK)
        {
            ping_time = sys_now();
            ping_recv(s);
        }
        else
        {
            printf("ping: send  error");
        }
        sys_msleep(PING_DELAY);
    }
}
#else

/* Ping using the raw ip */
static u8_t ping_recv_raw(void * arg, struct raw_pcb * pcb, struct pbuf * p, const ip_addr_t * addr)
{
    struct icmp_echo_hdr * iecho;
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(pcb);
    LWIP_UNUSED_ARG(addr);
    LWIP_ASSERT("p != NULL", p != NULL);

    if ((p->tot_len >= (IP_HDR_LEN + sizeof(struct icmp_echo_hdr))) && pbuf_header(p, -IP_HDR_LEN) == 0)
    {

        iecho = (struct icmp_echo_hdr *) p->payload;

        if ((iecho->id == PING_ID) && (iecho->seqno == lwip_htons(ping_seq_num)))
        {

            printf("Ping was successful Elapsed time : %" U32_F " ms\n", sys_now() - ping_time);

            /* do some ping result processing */
            pbuf_free(p);
            return 1; /* eat the packet */
        }
        /* not eaten, restore original packet */
        pbuf_header(p, IP_HDR_LEN);
    }

    return 0; /* don't eat the packet */
}

static void ping_send_raw(struct raw_pcb * raw, const ip_addr_t * addr)
{
    struct pbuf * p;
    icmp_packet_t * iecho;
    size_t ping_size = sizeof(icmp_packet_t);

    printf("\r\nPinging to Gateway ");
    print_ip4(addr->u_addr.ip4.addr);
    LWIP_ASSERT("ping_size <= 0xffff", ping_size <= 0xffff);

    p = pbuf_alloc(PBUF_IP, (u16_t) ping_size, PBUF_RAM);
    if (!p)
    {
        return;
    }
    if ((p->len == p->tot_len) && (p->next == NULL))
    {
        iecho = (icmp_packet_t *) p->payload;

        ping_prepare_echo(iecho, (u16_t) ping_size);

        raw_sendto(raw, p, addr);
        ping_time = sys_now();
    }
    pbuf_free(p);
}

static void ping_timeout(void * arg)
{
    struct raw_pcb * pcb = (struct raw_pcb *) arg;

    LWIP_ASSERT("ping_timeout: no pcb given!", pcb != NULL);

    ping_send_raw(pcb, ping_target);

    sys_timeout(PING_DELAY, ping_timeout, pcb);
}

void ping_raw(void)
{
    ping_pcb = raw_new(IP_PROTO_ICMP);
    LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);

    raw_recv(ping_pcb, ping_recv_raw, NULL);
    raw_bind(ping_pcb, IP_ADDR_ANY);
    ping_send_raw(ping_pcb, ping_target);
    sys_timeout(PING_DELAY, ping_timeout, ping_pcb);
}
#endif

CHIP_ERROR P6Utils::ping_init(void)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    struct netif * net_interface = NULL;
    net_interface                = cy_lwip_get_interface(CY_LWIP_STA_NW_INTERFACE);
    ping_target                  = &net_interface->gw;

    /* Ping to Gateway address */
    if (ping_target)
    {
#if PING_USE_SOCKETS
        ping_socket();
#else
        ping_raw();
#endif
    }
    else
    {
        ChipLogError(DeviceLayer, "ping_thread failed: Invalid IP address for Ping");
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}
