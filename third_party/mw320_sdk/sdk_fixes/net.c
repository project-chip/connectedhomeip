/** @file net.c
 *
 *  @brief  This file provides network porting code
 *
 *  Copyright 2008-2020 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code ("Materials") are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

#include <inttypes.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netifapi.h>
#include <lwip/opt.h>
#include <lwip/prot/autoip.h>
#include <lwip/stats.h>
#include <lwip/tcpip.h>
#include <wifi.h>
#include <wlan.h>
#include <wm_net.h>
#include <wm_os.h>
#include <wmlog.h>

#ifdef CONFIG_IPV6
#define IPV6_ADDR_STATE_TENTATIVE "Tentative"
#define IPV6_ADDR_STATE_PREFERRED "Preferred"
#define IPV6_ADDR_STATE_INVALID "Invalid"
#define IPV6_ADDR_STATE_VALID "Valid"
#define IPV6_ADDR_STATE_DEPRECATED "Deprecated"
#define IPV6_ADDR_TYPE_LINKLOCAL "Link-Local"
#define IPV6_ADDR_TYPE_GLOBAL "Global"
#define IPV6_ADDR_TYPE_UNIQUELOCAL "Unique-Local"
#define IPV6_ADDR_TYPE_SITELOCAL "Site-Local"
#define IPV6_ADDR_UNKNOWN "Unknown"
#endif

#define DNS_PORT 0x35
#define DHCPD_PORT 0x43
#define DHCPC_PORT 0x44

#define net_e(...) wmlog_e("net", ##__VA_ARGS__)

#define net_l(...) wmlog("net", ##__VA_ARGS__)

#ifdef CONFIG_NET_DEBUG
#define net_d(...) wmlog("net", ##__VA_ARGS__)
#else
#define net_d(...)
#endif /* ! CONFIG_NET_DEBUG */

struct interface
{
    struct netif netif;
    ip_addr_t ipaddr;
    ip_addr_t nmask;
    ip_addr_t gw;
};
typedef struct interface interface_t;

static interface_t g_mlan;
static interface_t g_uap;

err_t lwip_netif_init(struct netif * netif);
err_t lwip_netif_uap_init(struct netif * netif);
void handle_data_packet(const t_u8 interface, const t_u8 * rcvdata, const t_u16 datalen);
void handle_amsdu_data_packet(t_u8 interface, t_u8 * rcvdata, t_u16 datalen);
void handle_deliver_packet_above(t_u8 interface, t_void * lwip_pbuf);
bool wrapper_net_is_ip_or_ipv6(const t_u8 * buffer);

extern void stats_udp_bcast_display();

#ifdef CONFIG_IPV6
char * ipv6_addr_state_to_desc(unsigned char addr_state)
{
    if (ip6_addr_istentative(addr_state))
        return IPV6_ADDR_STATE_TENTATIVE;
    else if (ip6_addr_ispreferred(addr_state))
        return IPV6_ADDR_STATE_PREFERRED;
    else if (ip6_addr_isinvalid(addr_state))
        return IPV6_ADDR_STATE_INVALID;
    else if (ip6_addr_isvalid(addr_state))
        return IPV6_ADDR_STATE_VALID;
    else if (ip6_addr_isdeprecated(addr_state))
        return IPV6_ADDR_STATE_DEPRECATED;
    else
        return IPV6_ADDR_UNKNOWN;
}

char * ipv6_addr_type_to_desc(struct ipv6_config * ipv6_conf)
{
    if (ip6_addr_islinklocal((ip6_addr_t *) ipv6_conf->address))
        return IPV6_ADDR_TYPE_LINKLOCAL;
    else if (ip6_addr_isglobal((ip6_addr_t *) ipv6_conf->address))
        return IPV6_ADDR_TYPE_GLOBAL;
    else if (ip6_addr_isuniquelocal((ip6_addr_t *) ipv6_conf->address))
        return IPV6_ADDR_TYPE_UNIQUELOCAL;
    else if (ip6_addr_issitelocal((ip6_addr_t *) ipv6_conf->address))
        return IPV6_ADDR_TYPE_SITELOCAL;
    else
        return IPV6_ADDR_UNKNOWN;
}
#endif /* CONFIG_IPV6 */

int net_dhcp_hostname_set(char * hostname)
{
    netif_set_hostname(&g_mlan.netif, hostname);
    return WM_SUCCESS;
}

void net_ipv4stack_init()
{
    static bool tcpip_init_done;

    if (tcpip_init_done)
        return;

    tcpip_init(NULL, NULL);
    tcpip_init_done = true;

    net_d("Initialized TCP/IP v4 stack");
}

#ifdef CONFIG_IPV6
void net_ipv6stack_init(struct netif * netif)
{
    uint8_t mac[6];

    /* Set Multicast filter for IPV6 link local address
     * It contains first three bytes: 0x33 0x33 0xff (fixed)
     * and last three bytes as last three bytes of device mac */
    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = 0xff;
    mac[3] = netif->hwaddr[3];
    mac[4] = netif->hwaddr[4];
    mac[5] = netif->hwaddr[5];
    wifi_add_mcast_filter(mac);

    netif_create_ip6_linklocal_address(netif, 1);

    /* IPv6 routers use multicast IPv6 ff02::1 and MAC address
       33:33:00:00:00:01 for router advertisements */
    mac[0] = 0x33;
    mac[1] = 0x33;
    mac[2] = 0x00;
    mac[3] = 0x00;
    mac[4] = 0x00;
    mac[5] = 0x01;
    wifi_add_mcast_filter(mac);

    net_d("Initialized TCP/IP v6 stack");
}

static void wm_netif_ipv6_status_callback(struct netif * n)
{
    /* TODO: Implement appropriate functionality here*/
    net_d("Received callback on IPv6 address state change");
    wlan_wlcmgr_send_msg(WIFI_EVENT_NET_IPV6_CONFIG, WIFI_EVENT_REASON_SUCCESS, NULL);
}
#endif /* CONFIG_IPV6 */

int net_wlan_init(void)
{
    static int net_wlan_init_done;
    int ret;
    if (!net_wlan_init_done)
    {
        net_ipv4stack_init();

        wifi_register_data_input_callback(&handle_data_packet);
        wifi_register_amsdu_data_input_callback(&handle_amsdu_data_packet);
        wifi_register_deliver_packet_above_callback(&handle_deliver_packet_above);
        wifi_register_wrapper_net_is_ip_or_ipv6_callback(&wrapper_net_is_ip_or_ipv6);

        ip_2_ip4(&g_mlan.ipaddr)->addr = INADDR_ANY;
        ret = netifapi_netif_add(&g_mlan.netif, ip_2_ip4(&g_mlan.ipaddr), ip_2_ip4(&g_mlan.ipaddr), ip_2_ip4(&g_mlan.ipaddr), NULL,
                                 lwip_netif_init, tcpip_input);
        if (ret != 0)
        {
            net_e("MLAN interface add failed");
            return -WM_FAIL;
        }
#ifdef CONFIG_IPV6
        net_ipv6stack_init(&g_mlan.netif);
#endif /* CONFIG_IPV6 */

        ret = netifapi_netif_add(&g_uap.netif, ip_2_ip4(&g_uap.ipaddr), ip_2_ip4(&g_uap.ipaddr), ip_2_ip4(&g_uap.ipaddr), NULL,
                                 lwip_netif_uap_init, tcpip_input);
        if (ret != 0)
        {
            net_e("UAP interface add failed");
            return -WM_FAIL;
        }
#ifdef CONFIG_IPV6
        net_ipv6stack_init(&g_uap.netif);
#endif /* CONFIG_IPV6 */
        net_wlan_init_done = 1;

        net_l("Initialized TCP/IP networking stack");
    }

    wlan_wlcmgr_send_msg(WIFI_EVENT_NET_INTERFACE_CONFIG, WIFI_EVENT_REASON_SUCCESS, NULL);
    return WM_SUCCESS;
}

static void wm_netif_status_callback(struct netif * n)
{
    /* Variables to hold whether dhcp address has been supplied */
    /* is_dhcp_address: true when an dhcp address is available */
    bool is_dhcp_address = ((n->flags & NETIF_FLAG_UP) && (dhcp_supplied_address(n)));
    /* is_default_dhcp_address: true when the ip address of the passed
     *                          interface is the same as INADDR_ANY
     */
    bool is_default_dhcp_address = (ip_2_ip4(&(n->ip_addr))->addr == INADDR_ANY);
    /* State variables to be assigned to the event flag
     * a value of -1 represents failed state while the value of
     * 1 represents a successful connection event
     * The value of zero is default
     */
    enum connection_states
    {
        DHCP_STATE_FAILED  = -1,
        DHCP_STATE_RUNNING = 0,
        DHCP_STATE_SUCCESS = 1
    } event_flag_dhcp_connection;
    /* A flag to keep the status of dhcp connection
     * This flag is in state 1, if the connection has succeeded
     * This flag is in state -1, if the connection has failed
     * The flag value is zero, if we are still in process of connection
     * establishment
     */
    int wifi_event_reason;
    event_flag_dhcp_connection = DHCP_STATE_RUNNING;

    if (is_dhcp_address && !(is_default_dhcp_address))
    {
        /* If a valid non-default dhcp address is provided */
        event_flag_dhcp_connection = DHCP_STATE_SUCCESS;
    }
    else if (is_dhcp_address && (is_default_dhcp_address))
    {
        /* If the supplied dhcp address is the default address */
        event_flag_dhcp_connection = DHCP_STATE_FAILED;
    }
    else if (!dhcp_supplied_address(n))
    {
        /* If no ip address is supplied */
        event_flag_dhcp_connection = DHCP_STATE_FAILED;
    }
    else
    { /* Do Nothing */
    }

    /* Based on the value of status flag send corresponding message
     * to wlcmgr
     */
    switch (event_flag_dhcp_connection)
    {
    case DHCP_STATE_SUCCESS:
        wifi_event_reason = WIFI_EVENT_REASON_SUCCESS;
        break;
    case DHCP_STATE_FAILED:
        wifi_event_reason = WIFI_EVENT_REASON_FAILURE;
        break;
    default:
        break;
    }
    if (event_flag_dhcp_connection != DHCP_STATE_RUNNING)
        wlan_wlcmgr_send_msg(WIFI_EVENT_NET_DHCP_CONFIG, wifi_event_reason, NULL);
}

static int check_iface_mask(void * handle, uint32_t ipaddr)
{
    uint32_t interface_ip, interface_mask;
    net_get_if_ip_addr(&interface_ip, handle);
    net_get_if_ip_mask(&interface_mask, handle);
    if (interface_ip > 0)
        if ((interface_ip & interface_mask) == (ipaddr & interface_mask))
            return WM_SUCCESS;
    return -WM_FAIL;
}

void * net_ip_to_interface(uint32_t ipaddr)
{
    int ret;
    void * handle;
    /* Check mlan handle */
    handle = net_get_mlan_handle();
    ret    = check_iface_mask(handle, ipaddr);
    if (ret == WM_SUCCESS)
        return handle;

    /* Check uap handle */
    handle = net_get_uap_handle();
    ret    = check_iface_mask(handle, ipaddr);
    if (ret == WM_SUCCESS)
        return handle;

    /* If more interfaces are added then above check needs to done for
     * those newly added interfaces
     */
    return NULL;
}

void * net_sock_to_interface(int sock)
{
    struct sockaddr_in peer;
    socklen_t peerlen = sizeof(peer);
    void * req_iface  = NULL;
    int ret;

    ret = getpeername(sock, (struct sockaddr *) &peer, &peerlen);
    if (ret < 0)
        net_e("Failed to get peer name");
    req_iface = net_ip_to_interface(peer.sin_addr.s_addr);
    return req_iface;
}

void * net_get_sta_handle(void)
{
    return &g_mlan;
}

void * net_get_uap_handle(void)
{
    return &g_uap;
}

void net_interface_up(void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;
    netifapi_netif_set_up(&if_handle->netif);
}

void net_interface_down(void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;
    netifapi_netif_set_down(&if_handle->netif);
}

void net_interface_dhcp_stop(void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;
    netifapi_dhcp_stop(&if_handle->netif);
    netif_set_status_callback(&if_handle->netif, NULL);
}

int net_configure_address(struct wlan_ip_config * addr, void * intrfc_handle)
{
    if (!addr)
        return -WM_E_INVAL;
    if (!intrfc_handle)
        return -WM_E_INVAL;

    interface_t * if_handle = (interface_t *) intrfc_handle;

    net_d("configuring interface %s (with %s)", (if_handle == &g_mlan) ? "mlan" : "uap",
          (addr->ipv4.addr_type == ADDR_TYPE_DHCP) ? "DHCP client" : "Static IP");
    netifapi_netif_set_down(&if_handle->netif);

    /* De-register previously registered DHCP Callback for correct
     * address configuration.
     */
    netif_set_status_callback(&if_handle->netif, NULL);
#ifdef CONFIG_IPV6
    if (if_handle == &g_mlan)
    {
        /* Explicitly call this function so that the linklocal address
         * gets updated even if the interface does not get any IPv6
         * address in its lifetime */
        wm_netif_ipv6_status_callback(&if_handle->netif);
    }
#endif
    if (if_handle == &g_mlan || if_handle == &g_uap)
        netifapi_netif_set_default(&if_handle->netif);
    switch (addr->ipv4.addr_type)
    {
    case ADDR_TYPE_STATIC:
        ip_2_ip4(&(if_handle->ipaddr))->addr = addr->ipv4.address;
        ip_2_ip4(&(if_handle->nmask))->addr  = addr->ipv4.netmask;
        ip_2_ip4(&(if_handle->gw))->addr     = addr->ipv4.gw;
        netifapi_netif_set_addr(&if_handle->netif, ip_2_ip4(&if_handle->ipaddr), ip_2_ip4(&if_handle->nmask),
                                ip_2_ip4(&if_handle->gw));
        netifapi_netif_set_up(&if_handle->netif);
        break;

    case ADDR_TYPE_DHCP:
        /* Reset the address since we might be
           transitioning from static to DHCP */
        (void) memset(&if_handle->ipaddr, 0, sizeof(ip_addr_t));
        (void) memset(&if_handle->nmask, 0, sizeof(ip_addr_t));
        (void) memset(&if_handle->gw, 0, sizeof(ip_addr_t));
        netifapi_netif_set_addr(&if_handle->netif, ip_2_ip4(&if_handle->ipaddr), ip_2_ip4(&if_handle->nmask),
                                ip_2_ip4(&if_handle->gw));
        netifapi_netif_set_up(&if_handle->netif);
        netif_set_status_callback(&if_handle->netif, wm_netif_status_callback);
        netifapi_dhcp_start(&if_handle->netif);
        break;
    case ADDR_TYPE_LLA:
        /* For dhcp, instead of netifapi_netif_set_up, a
           netifapi_dhcp_start() call will be used */
        net_e("Not supported as of now...");
        break;
    default:
        break;
    }
    /* Finally this should send the following event. */
    if ((if_handle == &g_mlan))
    {
        wlan_wlcmgr_send_msg(WIFI_EVENT_NET_STA_ADDR_CONFIG, WIFI_EVENT_REASON_SUCCESS, NULL);

        /* XXX For DHCP, the above event will only indicate that the
         * DHCP address obtaining process has started. Once the DHCP
         * address has been obtained, another event,
         * WD_EVENT_NET_DHCP_CONFIG, should be sent to the wlcmgr.
         */
    }
    else if ((if_handle == &g_uap))
    {
        wlan_wlcmgr_send_msg(WIFI_EVENT_UAP_NET_ADDR_CONFIG, WIFI_EVENT_REASON_SUCCESS, NULL);
    }
    else
    { /* Do Nothing */
    }

    return WM_SUCCESS;
}

int net_get_if_addr(struct wlan_ip_config * addr, void * intrfc_handle)
{
    const ip_addr_t * tmp;
    interface_t * if_handle = (interface_t *) intrfc_handle;

    addr->ipv4.address = ip_2_ip4(&(if_handle->netif.ip_addr))->addr;
    addr->ipv4.netmask = ip_2_ip4(&(if_handle->netif.netmask))->addr;
    addr->ipv4.gw      = ip_2_ip4(&(if_handle->netif.gw))->addr;

    tmp             = dns_getserver(0);
    addr->ipv4.dns1 = ip_2_ip4(tmp)->addr;
    tmp             = dns_getserver(1);
    addr->ipv4.dns2 = ip_2_ip4(tmp)->addr;

    return WM_SUCCESS;
}

#ifdef CONFIG_IPV6
int net_get_if_ipv6_addr(struct wlan_ip_config * addr, void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;
    int i;

    for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
    {
        (void) memcpy(addr->ipv6[i].address, ip_2_ip6(&(if_handle->netif.ip6_addr[i]))->addr, 16);
        addr->ipv6[i].addr_state = if_handle->netif.ip6_addr_state[i];
    }
    /* TODO carry out more processing based on IPv6 fields in netif */
    return 0;
}

int net_get_if_ipv6_pref_addr(struct wlan_ip_config * addr, void * intrfc_handle)
{
    int i, ret = 0;
    interface_t * if_handle = (interface_t *) intrfc_handle;

    for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
    {
        if (if_handle->netif.ip6_addr_state[i] == IP6_ADDR_PREFERRED)
        {
            (void) memcpy(addr->ipv6[ret++].address, ip_2_ip6(&(if_handle->netif.ip6_addr[i]))->addr, 16);
        }
    }
    return ret;
}
#endif /* CONFIG_IPV6 */

int net_get_if_ip_addr(uint32_t * ip, void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;

    *ip = ip_2_ip4(&(if_handle->netif.ip_addr))->addr;
    return WM_SUCCESS;
}

int net_get_if_ip_mask(uint32_t * nm, void * intrfc_handle)
{
    interface_t * if_handle = (interface_t *) intrfc_handle;

    *nm = ip_2_ip4(&(if_handle->netif.netmask))->addr;
    return WM_SUCCESS;
}

void net_configure_dns(struct wlan_ip_config * ip, enum wlan_bss_role role)
{
    ip4_addr_t tmp;

    if (ip->ipv4.addr_type == ADDR_TYPE_STATIC)
    {
        if (role != WLAN_BSS_ROLE_UAP)
        {
            if (ip->ipv4.dns1 == 0)
                ip->ipv4.dns1 = ip->ipv4.gw;
            if (ip->ipv4.dns2 == 0)
                ip->ipv4.dns2 = ip->ipv4.dns1;
        }
        tmp.addr = ip->ipv4.dns1;
        dns_setserver(0, (ip_addr_t *) &tmp);
        tmp.addr = ip->ipv4.dns2;
        dns_setserver(1, (ip_addr_t *) &tmp);
    }

    /* DNS MAX Retries should be configured in lwip/dns.c to 3/4 */
    /* DNS Cache size of about 4 is sufficient */
}

void net_stat()
{
    stats_display();
}
