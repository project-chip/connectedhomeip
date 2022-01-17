/** @file wlan_tests.c
 *
 *  @brief  This file provides WLAN Test API
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

#include <cli.h>
#include <cli_utils.h>
#include <string.h>
#include <wifi.h>
#include <wlan.h>
#include <wlan_tests.h>
#include <wm_net.h> /* for net_inet_aton */

/*
 * NXP Test Framework (MTF) functions
 */

static void print_address(struct wlan_ip_config * addr, enum wlan_bss_role role)
{
    struct in_addr ip, gw, nm, dns1, dns2;
    char addr_type[10] = { 0 };

    /* If the current network role is STA and ipv4 is not connected then do
     * not print the addresses */
    if (role == WLAN_BSS_ROLE_STA && !is_sta_ipv4_connected())
        goto out;
    ip.s_addr   = addr->ipv4.address;
    gw.s_addr   = addr->ipv4.gw;
    nm.s_addr   = addr->ipv4.netmask;
    dns1.s_addr = addr->ipv4.dns1;
    dns2.s_addr = addr->ipv4.dns2;
    if (addr->ipv4.addr_type == ADDR_TYPE_STATIC)
        strncpy(addr_type, "STATIC", strlen("STATIC"));
    else if (addr->ipv4.addr_type == ADDR_TYPE_STATIC)
        strncpy(addr_type, "AUTO IP", strlen("AUTO IP"));
    else
        strncpy(addr_type, "DHCP", strlen("DHCP"));

    PRINTF("\r\n\tIPv4 Address\r\n");
    PRINTF("\taddress: %s", addr_type);
    PRINTF("\r\n\t\tIP:\t\t%s", inet_ntoa(ip));
    PRINTF("\r\n\t\tgateway:\t%s", inet_ntoa(gw));
    PRINTF("\r\n\t\tnetmask:\t%s", inet_ntoa(nm));
    PRINTF("\r\n\t\tdns1:\t\t%s", inet_ntoa(dns1));
    PRINTF("\r\n\t\tdns2:\t\t%s", inet_ntoa(dns2));
    PRINTF("\r\n");
out:
#ifdef CONFIG_IPV6
    if (role == WLAN_BSS_ROLE_STA || role == WLAN_BSS_ROLE_UAP)
    {
        int i;
        (void) PRINTF("\r\n\tIPv6 Addresses\r\n");
        for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
        {
            if (addr->ipv6[i].addr_state != IP6_ADDR_INVALID)
            {
                (void) PRINTF("\t%-13s:\t%s (%s)\r\n", ipv6_addr_type_to_desc(&addr->ipv6[i]), inet6_ntoa(addr->ipv6[i].address),
                              ipv6_addr_state_to_desc(addr->ipv6[i].addr_state));
            }
        }
        (void) PRINTF("\r\n");
    }
#endif
    return;
}

static const char * print_role(enum wlan_bss_role role)
{
    switch (role)
    {
    case WLAN_BSS_ROLE_STA:
        return "Infra";
    case WLAN_BSS_ROLE_UAP:
        return "uAP";
    case WLAN_BSS_ROLE_ANY:
        return "any";
    }

    return "unknown";
}

static void print_network(struct wlan_network * network)
{
    PRINTF("\"%s\"\r\n\tSSID: %s\r\n\tBSSID: ", network->name, network->ssid[0] ? network->ssid : "(hidden)");
    print_mac(network->bssid);
    if (network->channel)
        PRINTF("\r\n\tchannel: %d", network->channel);
    else
        PRINTF("\r\n\tchannel: %s", "(Auto)");
    PRINTF("\r\n\trole: %s\r\n", print_role(network->role));

    char * sec_tag = "\tsecurity";
    if (!network->security_specific)
    {
        sec_tag = "\tsecurity [Wildcard]";
    }
    switch (network->security.type)
    {
    case WLAN_SECURITY_NONE:
        PRINTF("%s: none\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WEP_OPEN:
        PRINTF("%s: WEP (open)\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WEP_SHARED:
        PRINTF("%s: WEP (shared)\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WPA:
        PRINTF("%s: WPA\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WPA2:
        PRINTF("%s: WPA2\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WPA_WPA2_MIXED:
        PRINTF("%s: WPA/WPA2 Mixed\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WPA3_SAE:
        PRINTF("%s: WPA3 SAE\r\n", sec_tag);
        break;
    case WLAN_SECURITY_WPA2_WPA3_SAE_MIXED:
        PRINTF("%s: WPA2/WPA3 SAE Mixed\r\n", sec_tag);
        break;
    default:
        break;
    }

    print_address(&network->ip, network->role);
}

/* Parse the 'arg' string as "ip:ipaddr,gwaddr,netmask,[dns1,dns2]" into
 * a wlan_ip_config data structure */
static int get_address(char * arg, struct wlan_ip_config * ip)
{
    char *ipaddr = NULL, *gwaddr = NULL, *netmask = NULL;
    char *dns1 = NULL, *dns2 = NULL;

    ipaddr = strstr(arg, "ip:");
    if (ipaddr == NULL)
        return -1;
    ipaddr += 3;

    gwaddr = strstr(ipaddr, ",");
    if (gwaddr == NULL)
        return -1;
    *gwaddr++ = 0;

    netmask = strstr(gwaddr, ",");
    if (netmask == NULL)
        return -1;
    *netmask++ = 0;

    dns1 = strstr(netmask, ",");
    if (dns1 != NULL)
    {
        *dns1++ = 0;
        dns2    = strstr(dns1, ",");
    }
    ip->ipv4.address = net_inet_aton(ipaddr);
    ip->ipv4.gw      = net_inet_aton(gwaddr);
    ip->ipv4.netmask = net_inet_aton(netmask);

    if (dns1 != NULL)
        ip->ipv4.dns1 = net_inet_aton(dns1);

    if (dns2 != NULL)
        ip->ipv4.dns2 = net_inet_aton(dns2);

    return 0;
}

int get_security(int argc, char ** argv, enum wlan_security_type type, struct wlan_network_security * sec)
{
    if (argc < 1)
        return 1;

    switch (type)
    {
    case WLAN_SECURITY_WPA:
    case WLAN_SECURITY_WPA2:
        if (argc < 1)
            return 1;
        /* copy the PSK phrase */
        sec->psk_len = strlen(argv[0]);
        if (sec->psk_len < sizeof(sec->psk))
            strcpy(sec->psk, argv[0]);
        else
            return 1;
        sec->type = type;
        break;
    default:
        return 1;
    }

    return 0;
}

static int get_role(char * arg, enum wlan_bss_role * role)
{
    if (!arg)
        return 1;

    if (string_equal(arg, "sta"))
    {
        *role = WLAN_BSS_ROLE_STA;
        return 0;
    }
    else if (string_equal(arg, "uap"))
    {
        *role = WLAN_BSS_ROLE_UAP;
        return 0;
    }
    return 1;
}

/*
 * MTF Shell Commands
 */
static void dump_wlan_add_usage()
{
    PRINTF("Usage:\r\n");
    PRINTF("For Station interface\r\n");
    PRINTF("  For DHCP IP Address assignment:\r\n");
    PRINTF("    wlan-add <profile_name> ssid <ssid> [wpa2 <secret>]"
           "\r\n");
    PRINTF("      If using WPA2 security, set the PMF configuration if required.\r\n");
    PRINTF("    wlan-add <profile_name> ssid <ssid> [wpa3 sae <secret> mfpc <1> mfpr <0/1>]"
           "\r\n");
    PRINTF("      If using WPA3 SAE security, always set the PMF configuration.\r\n");

    PRINTF("  For static IP address assignment:\r\n");
    PRINTF("    wlan-add <profile_name> ssid <ssid>\r\n"
           "    ip:<ip_addr>,<gateway_ip>,<netmask>\r\n");
    PRINTF("    [bssid <bssid>] [channel <channel number>]\r\n"
           "    [wpa2 <secret>]"
           "\r\n");

    PRINTF("For Micro-AP interface\r\n");
    PRINTF("    wlan-add <profile_name> ssid <ssid>\r\n"
           "    ip:<ip_addr>,<gateway_ip>,<netmask>\r\n");
    PRINTF("    role uap [bssid <bssid>]\r\n"
           "    [channel <channelnumber>]\r\n");
    PRINTF("    [wpa2 <secret>] [wpa3 sae <secret>]\r\n");
    PRINTF("    [mfpc <0/1>] [mfpr <0/1>]\r\n");
}

void test_wlan_add(int argc, char ** argv)
{
    struct wlan_network network;
    int ret = 0;
    int arg = 1;
    struct
    {
        unsigned ssid : 1;
        unsigned bssid : 1;
        unsigned channel : 1;
        unsigned address : 2;
        unsigned security : 1;
        unsigned security2 : 1;
        unsigned role : 1;
        unsigned mfpc : 1;
        unsigned mfpr : 1;
    } info;

    memset(&info, 0, sizeof(info));
    memset(&network, 0, sizeof(struct wlan_network));

    if (argc < 4)
    {
        dump_wlan_add_usage();
        PRINTF("Error: invalid number of arguments\r\n");
        return;
    }

    if (strlen(argv[arg]) >= WLAN_NETWORK_NAME_MAX_LENGTH)
    {
        PRINTF("Error: network name too long\r\n");
        return;
    }

    memcpy(network.name, argv[arg], strlen(argv[arg]));
    arg++;
    info.address = ADDR_TYPE_DHCP;
    do
    {
        if (!info.ssid && string_equal("ssid", argv[arg]))
        {
            if (strlen(argv[arg + 1]) > IEEEtypes_SSID_SIZE)
            {
                PRINTF("Error: SSID is too long\r\n");
                return;
            }
            memcpy(network.ssid, argv[arg + 1], strlen(argv[arg + 1]));
            arg += 2;
            info.ssid = 1;
        }
        else if (!info.bssid && string_equal("bssid", argv[arg]))
        {
            ret = get_mac(argv[arg + 1], network.bssid, ':');
            if (ret)
            {
                PRINTF("Error: invalid BSSID argument"
                       "\r\n");
                return;
            }
            arg += 2;
            info.bssid = 1;
        }
        else if (!info.channel && string_equal("channel", argv[arg]))
        {
            if (arg + 1 >= argc || get_uint(argv[arg + 1], &network.channel, strlen(argv[arg + 1])))
            {
                PRINTF("Error: invalid channel"
                       " argument\n");
                return;
            }
            arg += 2;
            info.channel = 1;
        }
        else if (!strncmp(argv[arg], "ip:", 3))
        {
            ret = get_address(argv[arg], &network.ip);
            if (ret)
            {
                PRINTF("Error: invalid address"
                       " argument\n");
                return;
            }
            arg++;
            info.address = ADDR_TYPE_STATIC;
        }
        else if (!info.security && string_equal("wpa", argv[arg]))
        {
            ret = get_security(argc - arg - 1, argv + arg + 1, WLAN_SECURITY_WPA, &network.security);
            if (ret)
            {
                PRINTF("Error: invalid WPA security"
                       " argument\r\n");
                return;
            }
            arg += 2;
            info.security++;
        }
        else if (!info.security && string_equal("wpa2", argv[arg]))
        {
            ret = get_security(argc - arg - 1, argv + arg + 1, WLAN_SECURITY_WPA2, &network.security);
            if (ret)
            {
                PRINTF("Error: invalid WPA2 security"
                       " argument\r\n");
                return;
            }
            arg += 2;
            info.security++;
        }
        else if (!info.security2 && string_equal("wpa3", argv[arg]))
        {
            if (string_equal(argv[arg + 1], "sae"))
            {
                network.security.type = WLAN_SECURITY_WPA3_SAE;
                /* copy the PSK phrase */
                network.security.password_len = strlen(argv[arg + 2]);
                if (!network.security.password_len)
                {
                    PRINTF("Error: invalid WPA3 security"
                           " argument\r\n");
                    return;
                }
                if (network.security.password_len < sizeof(network.security.password))
                    strcpy(network.security.password, argv[arg + 2]);
                else
                {
                    PRINTF("Error: invalid WPA3 security"
                           " argument\r\n");
                    return;
                }
                arg += 3;
            }
            else
            {
                PRINTF("Error: invalid WPA3 security"
                       " argument\r\n");
                return;
            }
            info.security2++;
        }
        else if (!info.role && string_equal("role", argv[arg]))
        {
            if (arg + 1 >= argc || get_role(argv[arg + 1], &network.role))
            {
                PRINTF("Error: invalid wireless"
                       " network role\r\n");
                return;
            }
            arg += 2;
            info.role++;
        }
        else if (!info.mfpc && string_equal("mfpc", argv[arg]))
        {
            network.security.mfpc = atoi(argv[arg + 1]);
            if (arg + 1 >= argc || (network.security.mfpc != 0 && network.security.mfpc != 1))
            {
                PRINTF("Error: invalid wireless"
                       " network mfpc\r\n");
                return;
            }
            arg += 2;
            info.mfpc++;
        }
        else if (!info.mfpr && string_equal("mfpr", argv[arg]))
        {
            network.security.mfpr = atoi(argv[arg + 1]);
            if (arg + 1 >= argc || (network.security.mfpr != 0 && network.security.mfpr != 1))
            {
                PRINTF("Error: invalid wireless"
                       " network mfpr\r\n");
                return;
            }
            arg += 2;
            info.mfpr++;
        }
        else if (!strncmp(argv[arg], "autoip", 6))
        {
            info.address = ADDR_TYPE_LLA;
            arg++;
        }
        else
        {
            dump_wlan_add_usage();
            PRINTF("Error: argument %d is invalid\r\n", arg);
            return;
        }
    } while (arg < argc);

    if (!info.ssid && !info.bssid)
    {
        dump_wlan_add_usage();
        PRINTF("Error: specify at least the SSID or BSSID\r\n");
        return;
    }

    if ((network.security.type == WLAN_SECURITY_WPA2) || (network.security.type == WLAN_SECURITY_WPA3_SAE))
    {
        if (network.security.psk_len && network.security.password_len)
            network.security.type = WLAN_SECURITY_WPA2_WPA3_SAE_MIXED;
    }

    network.ip.ipv4.addr_type = info.address;

    ret = wlan_add_network(&network);
    switch (ret)
    {
    case WM_SUCCESS:
        PRINTF("Added \"%s\"\r\n", network.name);
        break;
    case -WM_E_INVAL:
        PRINTF("Error: network already exists or invalid arguments\r\n");
        break;
    case -WM_E_NOMEM:
        PRINTF("Error: network list is full\r\n");
        break;
    case WLAN_ERROR_STATE:
        PRINTF("Error: can't add networks in this state\r\n");
        break;
    default:
        PRINTF("Error: unable to add network for unknown"
               " reason\r\n");
        break;
    }
}

int __scan_cb(unsigned int count)
{
    struct wlan_scan_result res;
    int i;
    int err;

    if (count == 0)
    {
        PRINTF("no networks found\r\n");
        return 0;
    }

    PRINTF("%d network%s found:\r\n", count, count == 1 ? "" : "s");

    for (i = 0; i < count; i++)
    {
        err = wlan_get_scan_result(i, &res);
        if (err)
        {
            PRINTF("Error: can't get scan res %d\r\n", i);
            continue;
        }

        print_mac(res.bssid);

        if (res.ssid[0])
            PRINTF(" \"%s\" %s\r\n", res.ssid, print_role(res.role));
        else
            PRINTF(" (hidden) %s\r\n", print_role(res.role));

        PRINTF("\tchannel: %d\r\n", res.channel);
        PRINTF("\trssi: -%d dBm\r\n", res.rssi);
        PRINTF("\tsecurity: ");
        if (res.wep)
            PRINTF("WEP ");
        if (res.wpa && res.wpa2)
            PRINTF("WPA/WPA2 Mixed ");
        else
        {
            if (res.wpa)
                PRINTF("WPA ");
            if (res.wpa2)
                PRINTF("WPA2 ");
            if (res.wpa3_sae)
                PRINTF("WPA3 SAE ");
            if (res.wpa2_entp)
                PRINTF("WPA2 Enterprise");
        }
        if (!(res.wep || res.wpa || res.wpa2 || res.wpa3_sae || res.wpa2_entp))
            PRINTF("OPEN ");
        PRINTF("\r\n");

        PRINTF("\tWMM: %s\r\n", res.wmm ? "YES" : "NO");
        if (res.trans_ssid_len)
        {
            PRINTF("\tOWE BSSID: ");
            print_mac(res.trans_bssid);
            PRINTF("\r\n\tOWE SSID:");
            if (res.trans_ssid_len)
                PRINTF(" \"%s\"\r\n", res.trans_ssid);
        }
    }

    return 0;
}

void test_wlan_scan(int argc, char ** argv)
{
    if (wlan_scan(__scan_cb))
        PRINTF("Error: scan request failed\r\n");
    else
        PRINTF("Scan scheduled...\r\n");
}

static void dump_wlan_scan_opt_usage()
{
    PRINTF("Usage:\r\n");
    PRINTF("    wlan-scan-opt ssid <ssid> bssid <bssid> "
           "channel <channel> probes <probes>"
           "\r\n");
}

void test_wlan_scan_opt(int argc, char ** argv)
{
    wlan_scan_params_v2_t wlan_scan_param;
    int ret = 0;
    int arg = 1;
    struct
    {
        unsigned ssid : 1;
        unsigned bssid : 1;
        unsigned channel : 1;
        unsigned probes : 1;
    } info;

    memset(&info, 0, sizeof(info));
    memset(&wlan_scan_param, 0, sizeof(wlan_scan_params_v2_t));

    if (argc < 3)
    {
        dump_wlan_scan_opt_usage();
        PRINTF("Error: invalid number of arguments\r\n");
        return;
    }
    do
    {
        if (!info.ssid && string_equal("ssid", argv[arg]))
        {
            if (strlen(argv[arg + 1]) > IEEEtypes_SSID_SIZE)
            {
                PRINTF("Error: SSID is too long\r\n");
                return;
            }
            memcpy(wlan_scan_param.ssid, argv[arg + 1], strlen(argv[arg + 1]));
            arg += 2;
            info.ssid = 1;
        }
        else if (!info.bssid && string_equal("bssid", argv[arg]))
        {
            ret = get_mac(argv[arg + 1], (char *) wlan_scan_param.bssid, ':');
            if (ret)
            {
                PRINTF("Error: invalid BSSID argument"
                       "\r\n");
                return;
            }
            arg += 2;
            info.bssid = 1;
        }
        else if (!info.channel && string_equal("channel", argv[arg]))
        {
            if (arg + 1 >= argc ||
                get_uint(argv[arg + 1], (unsigned int *) &wlan_scan_param.chan_list[0].chan_number, strlen(argv[arg + 1])))
            {
                PRINTF("Error: invalid channel"
                       " argument\n");
                return;
            }
            wlan_scan_param.num_channels           = 1;
            wlan_scan_param.chan_list[0].scan_type = 1;
            wlan_scan_param.chan_list[0].scan_time = 120;
            arg += 2;
            info.channel = 1;
        }
        else if (!info.probes && string_equal("probes", argv[arg]))
        {
            if (arg + 1 >= argc || get_uint(argv[arg + 1], (unsigned int *) &wlan_scan_param.num_probes, strlen(argv[arg + 1])))
            {
                PRINTF("Error: invalid probes"
                       " argument\n");
                return;
            }
            if (wlan_scan_param.num_probes > 4)
            {
                PRINTF("Error: invalid number of probes"
                       "\r\n");
                return;
            }
            arg += 2;
            info.probes = 1;
        }
        else
        {
            dump_wlan_scan_opt_usage();
            PRINTF("Error: argument %d is invalid\r\n", arg);
            return;
        }
    } while (arg < argc);

    if (!info.ssid && !info.bssid)
    {
        dump_wlan_scan_opt_usage();
        PRINTF("Error: specify at least the SSID or BSSID\r\n");
        return;
    }

    wlan_scan_param.cb = __scan_cb;

    if (wlan_scan_with_opt(wlan_scan_param))
        PRINTF("Error: scan request failed\r\n");
    else
    {
        PRINTF("Scan for ");
        if (info.ssid)
            PRINTF("ssid \"%s\" ", wlan_scan_param.ssid);
        if (info.bssid)
        {
            PRINTF("bssid ");
            print_mac((const char *) wlan_scan_param.bssid);
        }
        if (info.probes)
            PRINTF("with %d probes ", wlan_scan_param.num_probes);
        PRINTF("scheduled...\r\n");
    }
}

static void test_wlan_remove(int argc, char ** argv)
{
    int ret;

    if (argc < 2)
    {
        PRINTF("Usage: %s <profile_name>\r\n", argv[0]);
        PRINTF("Error: specify network to remove\r\n");
        return;
    }

    ret = wlan_remove_network(argv[1]);
    switch (ret)
    {
    case WM_SUCCESS:
        PRINTF("Removed \"%s\"\r\n", argv[1]);
        break;
    case -WM_E_INVAL:
        PRINTF("Error: network not found\r\n");
        break;
    case WLAN_ERROR_STATE:
        PRINTF("Error: can't remove network in this state\r\n");
        break;
    default:
        PRINTF("Error: unable to remove network\r\n");
        break;
    }
}

static void test_wlan_connect(int argc, char ** argv)
{
    int ret = wlan_connect(argc >= 2 ? argv[1] : NULL);

    if (ret == WLAN_ERROR_STATE)
    {
        PRINTF("Error: connect manager not running\r\n");
        return;
    }

    if (ret == -WM_E_INVAL)
    {
        PRINTF("Usage: %s <profile_name>\r\n", argv[0]);
        PRINTF("Error: specify a network to connect\r\n");
        return;
    }
    PRINTF("Connecting to network...\r\nUse 'wlan-stat' for "
           "current connection status.\r\n");
}

static void test_wlan_start_network(int argc, char ** argv)
{
    int ret;

    if (argc < 2)
    {
        PRINTF("Usage: %s <profile_name>\r\n", argv[0]);
        PRINTF("Error: specify a network to start\r\n");
        return;
    }

    ret = wlan_start_network(argv[1]);
    if (ret != WM_SUCCESS)
        PRINTF("Error: unable to start network\r\n");
}

void test_wlan_stop_network(int argc, char ** argv)
{
    int ret;
    struct wlan_network network;

    wlan_get_current_uap_network(&network);
    ret = wlan_stop_network(network.name);
    if (ret != WM_SUCCESS)
        PRINTF("Error: unable to stop network\r\n");
}

void test_wlan_disconnect(int argc, char ** argv)
{
    if (wlan_disconnect() != WM_SUCCESS)
        PRINTF("Error: unable to disconnect\r\n");
}

static void test_wlan_stat(int argc, char ** argv)
{
    enum wlan_connection_state state;
    enum wlan_ps_mode ps_mode;
    char ps_mode_str[15];

    if (wlan_get_ps_mode(&ps_mode))
    {
        PRINTF("Error: unable to get power save"
               " mode\r\n");
        return;
    }

    switch (ps_mode)
    {
    case WLAN_IEEE:
        strcpy(ps_mode_str, "IEEE ps");
        break;
    case WLAN_DEEP_SLEEP:
        strcpy(ps_mode_str, "Deep sleep");
        break;
    case WLAN_ACTIVE:
    default:
        strcpy(ps_mode_str, "Active");
        break;
    }

    if (wlan_get_connection_state(&state))
    {
        PRINTF("Error: unable to get STA connection"
               " state\r\n");
    }
    else
    {
        switch (state)
        {
        case WLAN_DISCONNECTED:
            PRINTF("Station disconnected (%s)\r\n", ps_mode_str);
            break;
        case WLAN_SCANNING:
            PRINTF("Station scanning (%s)\r\n", ps_mode_str);
            break;
        case WLAN_ASSOCIATING:
            PRINTF("Station associating (%s)\r\n", ps_mode_str);
            break;
        case WLAN_ASSOCIATED:
            PRINTF("Station associated (%s)\r\n", ps_mode_str);
            break;
        case WLAN_CONNECTING:
            PRINTF("Station connecting (%s)\r\n", ps_mode_str);
            break;
        case WLAN_CONNECTED:
            PRINTF("Station connected (%s)\r\n", ps_mode_str);
            break;
        default:
            PRINTF("Error: invalid STA state"
                   " %d\r\n",
                   state);
        }
    }
    if (wlan_get_uap_connection_state(&state))
    {
        PRINTF("Error: unable to get uAP connection"
               " state\r\n");
    }
    else
    {
        switch (state)
        {
        case WLAN_UAP_STARTED:
            strcpy(ps_mode_str, "Active");
            PRINTF("uAP started (%s)\r\n", ps_mode_str);
            break;
        case WLAN_UAP_STOPPED:
            PRINTF("uAP stopped\r\n");
            break;
        default:
            PRINTF("Error: invalid uAP state"
                   " %d\r\n",
                   state);
        }
    }
}

static void test_wlan_list(int argc, char ** argv)
{
    struct wlan_network network;
    unsigned int count;
    int i;

    if (wlan_get_network_count(&count))
    {
        PRINTF("Error: unable to get number of networks\r\n");
        return;
    }

    PRINTF("%d network%s%s\r\n", count, count == 1 ? "" : "s", count > 0 ? ":" : "");
    for (i = 0; i < count; i++)
    {
        if (wlan_get_network(i, &network) == WM_SUCCESS)
            print_network(&network);
    }
}

static void test_wlan_info(int argc, char ** argv)
{
    enum wlan_connection_state state;
    struct wlan_network sta_network;
    struct wlan_network uap_network;
    int sta_found = 0;

    if (wlan_get_connection_state(&state))
    {
        PRINTF("Error: unable to get STA connection"
               " state\r\n");
    }
    else
    {
        switch (state)
        {
        case WLAN_CONNECTED:
            if (!wlan_get_current_network(&sta_network))
            {
                PRINTF("Station connected to:\r\n");
                print_network(&sta_network);
                sta_found = 1;
            }
            else
                PRINTF("Station not connected\r\n");
            break;
        default:
            PRINTF("Station not connected\r\n");
            break;
        }
    }

    if (wlan_get_current_uap_network(&uap_network))
        PRINTF("uAP not started\r\n");
    else
    {
        /* Since uAP automatically changes the channel to the one that
         * STA is on */
        if (sta_found == 1)
            uap_network.channel = sta_network.channel;

        if (uap_network.role == WLAN_BSS_ROLE_UAP)
            PRINTF("uAP started as:\r\n");

        print_network(&uap_network);
    }
}

static void test_wlan_address(int argc, char ** argv)
{
    struct wlan_network network;

    if (wlan_get_current_network(&network))
    {
        PRINTF("not connected\r\n");
        return;
    }
    print_address(&network.ip, network.role);
}

static void test_wlan_get_uap_channel(int argc, char ** argv)
{
    int channel;
    int rv = wifi_get_uap_channel(&channel);
    if (rv != WM_SUCCESS)
        PRINTF("Unable to get channel: %d\r\n", rv);
    else
        PRINTF("uAP channel: %d\r\n", channel);
}

static void test_wlan_get_uap_sta_list(int argc, char ** argv)
{
    int i;
    wifi_sta_list_t * sl = NULL;

    wifi_uap_bss_sta_list(&sl);

    if (!sl)
    {
        PRINTF("Failed to get sta list\n\r");
        return;
    }

    wifi_sta_info_t * si = (wifi_sta_info_t *) (&sl->count + 1);

    PRINTF("Number of STA = %d \r\n\r\n", sl->count);
    for (i = 0; i < sl->count; i++)
    {
        PRINTF("STA %d information:\r\n", i + 1);
        PRINTF("=====================\r\n");
        PRINTF("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", si[i].mac[0], si[i].mac[1], si[i].mac[2], si[i].mac[3],
               si[i].mac[4], si[i].mac[5]);
        PRINTF("Power mfg status: %s\r\n", (si[i].power_mgmt_status == 0) ? "active" : "power save");
        PRINTF("Rssi : %d dBm\r\n\r\n", (signed char) si[i].rssi);
    }

    os_mem_free(sl);
}

static struct cli_command tests[] = {
    { "wlan-scan", NULL, test_wlan_scan },
    { "wlan-scan-opt", "ssid <ssid> bssid ...", test_wlan_scan_opt },
    { "wlan-add", "<profile_name> ssid <ssid> bssid...", test_wlan_add },
    { "wlan-remove", "<profile_name>", test_wlan_remove },
    { "wlan-list", NULL, test_wlan_list },
    { "wlan-connect", "<profile_name>", test_wlan_connect },
    { "wlan-start-network", "<profile_name>", test_wlan_start_network },
    { "wlan-stop-network", NULL, test_wlan_stop_network },
    { "wlan-disconnect", NULL, test_wlan_disconnect },
    { "wlan-stat", NULL, test_wlan_stat },
    { "wlan-info", NULL, test_wlan_info },
    { "wlan-address", NULL, test_wlan_address },
    { "wlan-get-uap-channel", NULL, test_wlan_get_uap_channel },
    { "wlan-get-uap-sta-list", NULL, test_wlan_get_uap_sta_list },
};

/* Register our commands with the MTF. */
int wlan_cli_init(void)
{
    int i;

    i = wlan_basic_cli_init();
    if (i != WLAN_ERROR_NONE)
        return i;

    if (cli_register_commands(tests, sizeof(tests) / sizeof(struct cli_command)))
        return -WM_FAIL;

    return WM_SUCCESS;
}
