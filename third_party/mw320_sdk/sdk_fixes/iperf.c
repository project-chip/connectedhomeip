/** @file iperf.c
 *
 *  @brief  This file provides the support for network utility iperf
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

/* iperf.c: This file contains the support for network utility iperf */

#include <cli.h>
#include <cli_utils.h>
#include <string.h>
#include <wlan.h>
#include <wm_net.h>
#include <wm_os.h>

#include "lwip/tcpip.h"
#include "lwiperf.h"

#ifndef IPERF_UDP_CLIENT_RATE
#define IPERF_UDP_CLIENT_RATE (100 * 1024 * 1024) /* 100 Mbit/s */
#endif

#ifndef IPERF_CLIENT_AMOUNT
#define IPERF_CLIENT_AMOUNT (-1000) /* 10 seconds */
#endif

struct iperf_test_context
{
    bool server_mode;
    bool tcp;
    enum lwiperf_client_type client_type;
    void * iperf_session;
};

static struct iperf_test_context ctx;
TimerHandle_t timer;
ip_addr_t server_address;
ip_addr_t bind_address;
bool multicast;
#ifdef CONFIG_IPV6
bool ipv6;
#endif
int amount = IPERF_CLIENT_AMOUNT;
uint8_t mcast_mac[6];
bool mcast_mac_valid;

static void timer_poll_udp_client(TimerHandle_t timer);

/* Report state => string */
const char * report_type_str[] = {
    "TCP_DONE_SERVER (RX)",        /* LWIPERF_TCP_DONE_SERVER,*/
    "TCP_DONE_CLIENT (TX)",        /* LWIPERF_TCP_DONE_CLIENT,*/
    "TCP_ABORTED_LOCAL",           /* LWIPERF_TCP_ABORTED_LOCAL, */
    "TCP_ABORTED_LOCAL_DATAERROR", /* LWIPERF_TCP_ABORTED_LOCAL_DATAERROR, */
    "TCP_ABORTED_LOCAL_TXERROR",   /* LWIPERF_TCP_ABORTED_LOCAL_TXERROR, */
    "TCP_ABORTED_REMOTE",          /* LWIPERF_TCP_ABORTED_REMOTE, */
    "UDP_DONE_SERVER (RX)",        /* LWIPERF_UDP_DONE_SERVER, */
    "UDP_DONE_CLIENT (TX)",        /* LWIPERF_UDP_DONE_CLIENT, */
    "UDP_ABORTED_LOCAL",           /* LWIPERF_UDP_ABORTED_LOCAL, */
    "UDP_ABORTED_LOCAL_DATAERROR", /* LWIPERF_UDP_ABORTED_LOCAL_DATAERROR, */
    "UDP_ABORTED_LOCAL_TXERROR",   /* LWIPERF_UDP_ABORTED_LOCAL_TXERROR, */
    "UDP_ABORTED_REMOTE",          /* LWIPERF_UDP_ABORTED_REMOTE, */
};

/** Prototype of a report function that is called when a session is finished.
    This report function shows the test results. */
static void lwiperf_report(void * arg, enum lwiperf_report_type report_type, const ip_addr_t * local_addr, u16_t local_port,
                           const ip_addr_t * remote_addr, u16_t remote_port, u64_t bytes_transferred, u32_t ms_duration,
                           u32_t bandwidth_kbitpsec)
{
    (void) PRINTF("-------------------------------------------------\r\n");
    if (report_type < (sizeof(report_type_str) / sizeof(report_type_str[0])))
    {
        (void) PRINTF(" %s \r\n", report_type_str[report_type]);
        if (local_addr && remote_addr)
        {
#ifdef CONFIG_IPV6
            if (ipv6)
                (void) PRINTF(" Local address : %s ", inet6_ntoa(local_addr->u_addr.ip6));
            else
#endif
                (void) PRINTF(" Local address : %u.%u.%u.%u ", ((u8_t *) local_addr)[0], ((u8_t *) local_addr)[1],
                              ((u8_t *) local_addr)[2], ((u8_t *) local_addr)[3]);
            (void) PRINTF(" Port %d \r\n", local_port);
#ifdef CONFIG_IPV6
            if (ipv6)
                (void) PRINTF(" Remote address : %s ", inet6_ntoa(remote_addr->u_addr.ip6));
            else
#endif
                (void) PRINTF(" Remote address : %u.%u.%u.%u ", ((u8_t *) remote_addr)[0], ((u8_t *) remote_addr)[1],
                              ((u8_t *) remote_addr)[2], ((u8_t *) remote_addr)[3]);
            (void) PRINTF(" Port %d \r\n", remote_port);
            (void) PRINTF(" Bytes Transferred %llu \r\n", bytes_transferred);
            (void) PRINTF(" Duration (ms) %d \r\n", ms_duration);
            (void) PRINTF(" Bandwidth (Mbitpsec) %d \r\n", bandwidth_kbitpsec / 1000);
        }
    }
    else
    {
        (void) PRINTF(" IPERF Report error\r\n");
    }
    (void) PRINTF("\r\n");
}

/*!
 * @brief Function to start iperf test.
 */
static void iperf_test_start(void * arg)
{
    struct iperf_test_context * ctx = (struct iperf_test_context *) arg;

    if (ctx->iperf_session != NULL)
    {
        (void) PRINTF("Abort ongoing IPERF session\r\n");
        lwiperf_abort(ctx->iperf_session);
        ctx->iperf_session = NULL;
    }

    if (!(ctx->tcp) && ctx->client_type == LWIPERF_DUAL)
    {
        /* Reducing udp Tx timer interval for rx to be served and start the timer */
        xTimerChangePeriod(timer, os_msec_to_ticks(4), 100);
    }
    else
    {
        /* Returning original timer settings of 1 ms interval and start the timer */
        xTimerChangePeriod(timer, 1 / portTICK_PERIOD_MS, 100);
    }

    if (ctx->server_mode)
    {
        if (ctx->tcp)
        {
#ifdef CONFIG_IPV6
            if (ipv6)
                ctx->iperf_session =
                    lwiperf_start_tcp_server(netif_ip_addr6(netif_default, 0), LWIPERF_TCP_PORT_DEFAULT, lwiperf_report, 0);
            else
#endif
                ctx->iperf_session = lwiperf_start_tcp_server(IP_ADDR_ANY, LWIPERF_TCP_PORT_DEFAULT, lwiperf_report, 0);
        }
        else
        {
            if (multicast)
            {
#ifdef CONFIG_IPV6
                wifi_get_ipv4_multicast_mac(ntohl(bind_address.u_addr.ip4.addr), mcast_mac);
#else
                wifi_get_ipv4_multicast_mac(ntohl(bind_address.addr), mcast_mac);
#endif
                if (wifi_add_mcast_filter(mcast_mac) != WM_SUCCESS)
                {
                    (void) PRINTF("IPERF session init failed\r\n");
                    lwiperf_abort(ctx->iperf_session);
                    ctx->iperf_session = NULL;
                    return;
                }
                mcast_mac_valid = true;
            }
#ifdef CONFIG_IPV6
            if (ipv6)
                ctx->iperf_session =
                    lwiperf_start_udp_server(netif_ip_addr6(netif_default, 0), LWIPERF_TCP_PORT_DEFAULT, lwiperf_report, 0);
            else
#endif
                ctx->iperf_session = lwiperf_start_udp_server(&bind_address, LWIPERF_TCP_PORT_DEFAULT, lwiperf_report, 0);
        }
    }
    else
    {
        if (ctx->tcp)
        {
#ifdef CONFIG_IPV6
            if (ipv6)
                ip6_addr_assign_zone(ip_2_ip6(&server_address), IP6_UNICAST, netif_default);
#endif
            ctx->iperf_session =
                lwiperf_start_tcp_client(&server_address, LWIPERF_TCP_PORT_DEFAULT, ctx->client_type, amount, lwiperf_report, 0);
        }
        else
        {
            if (IP_IS_V4(&server_address) && ip_addr_ismulticast(&server_address))
            {
#ifdef CONFIG_IPV6
                wifi_get_ipv4_multicast_mac(ntohl(server_address.u_addr.ip4.addr), mcast_mac);
#else
                wifi_get_ipv4_multicast_mac(ntohl(server_address.addr), mcast_mac);
#endif
                wifi_add_mcast_filter(mcast_mac);
                mcast_mac_valid = true;
            }
#ifdef CONFIG_IPV6
            if (ipv6)
            {
                ctx->iperf_session =
                    lwiperf_start_udp_client(netif_ip_addr6(netif_default, 0), LWIPERF_TCP_PORT_DEFAULT, &server_address,
                                             LWIPERF_TCP_PORT_DEFAULT, ctx->client_type, amount, IPERF_UDP_CLIENT_RATE,
#ifdef CONFIG_WMM
                                             qos,
#else
                                             0,
#endif

                                             lwiperf_report, NULL);
            }
            else
            {
#endif
                ctx->iperf_session =
                    lwiperf_start_udp_client(&bind_address, LWIPERF_TCP_PORT_DEFAULT, &server_address, LWIPERF_TCP_PORT_DEFAULT,
                                             ctx->client_type, amount, IPERF_UDP_CLIENT_RATE,
#ifdef CONFIG_WMM
                                             qos,
#else
                                         0,
#endif
                                             lwiperf_report, NULL);
#ifdef CONFIG_IPV6
            }
#endif
        }
    }

    if (ctx->iperf_session == NULL)
    {
        (void) PRINTF("IPERF initialization failed!\r\n");
    }
    else
    {
        (void) PRINTF("IPERF initialization successful\r\n");
    }
}

/*!
 * @brief Function to abort iperf test.
 */
static void iperf_test_abort(void * arg)
{
    struct iperf_test_context * test_ctx = (struct iperf_test_context *) arg;

    (void) xTimerStop(timer, 0);
    if (test_ctx->iperf_session != NULL)
    {
        lwiperf_abort(test_ctx->iperf_session);
        test_ctx->iperf_session = NULL;
    }

    (void) memset(&ctx, 0, sizeof(struct iperf_test_context));
}

/*!
 * @brief Invokes UDP polling, to be run on tcpip_thread.
 */
static void poll_udp_client(void * arg)
{
    LWIP_UNUSED_ARG(arg);

    lwiperf_poll_udp_client();
}

/*!
 * @brief Invokes UDP polling on tcpip_thread.
 */
static void timer_poll_udp_client(TimerHandle_t timer)
{
    LWIP_UNUSED_ARG(timer);

    tcpip_try_callback(poll_udp_client, NULL);
}

static void TESTAbort(void)
{
    iperf_test_abort((void *) &ctx);
}

static void TCPServer(void)
{
    ctx.server_mode = true;
    ctx.tcp         = true;
    ctx.client_type = LWIPERF_CLIENT;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void TCPClient(void)
{
    ctx.server_mode = false;
    ctx.tcp         = true;
    ctx.client_type = LWIPERF_CLIENT;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void TCPClientDual(void)
{
    ctx.server_mode = false;
    ctx.tcp         = true;
    ctx.client_type = LWIPERF_DUAL;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void TCPClientTradeOff(void)
{
    ctx.server_mode = false;
    ctx.tcp         = true;
    ctx.client_type = LWIPERF_TRADEOFF;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void UDPServer(void)
{
    ctx.server_mode = true;
    ctx.tcp         = false;
    ctx.client_type = LWIPERF_CLIENT;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void UDPClient(void)
{
    ctx.server_mode = false;
    ctx.tcp         = false;
    ctx.client_type = LWIPERF_CLIENT;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void UDPClientDual(void)
{
    ctx.server_mode = false;
    ctx.tcp         = false;
    ctx.client_type = LWIPERF_DUAL;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

static void UDPClientTradeOff(void)
{
    ctx.server_mode = false;
    ctx.tcp         = false;
    ctx.client_type = LWIPERF_TRADEOFF;

    tcpip_callback(iperf_test_start, (void *) &ctx);
}

/* Display the usage of iperf */
static void display_iperf_usage()
{
    (void) PRINTF("Usage:\r\n");
    (void) PRINTF("\tiperf [-s|-c <host>|-a] [options]\r\n");
    (void) PRINTF("\tiperf [-h]\r\n");
    (void) PRINTF("\r\n");
    (void) PRINTF("\tClient/Server:\r\n");
    (void) PRINTF("\t   -u             use UDP rather than TCP\r\n");
    (void) PRINTF("\t   -B    <host>   bind to <host> (including multicast address)\r\n");
    (void) PRINTF("\t   -a             abort ongoing iperf session\r\n");
    (void) PRINTF("\tServer specific:\r\n");
    (void) PRINTF("\t   -s             run in server mode\r\n");
    (void) PRINTF("\tClient specific:\r\n");
    (void) PRINTF("\t   -c    <host>   run in client mode, connecting to <host>\r\n");
    (void) PRINTF("\t   -d             Do a bidirectional test simultaneously\r\n");
    (void) PRINTF("\t   -r             Do a bidirectional test individually\r\n");
    (void) PRINTF("\t   -t    #        time in seconds to transmit for (default 10 secs)\r\n");
#ifdef CONFIG_IPV6
    (void) PRINTF("\t   -V             Set the domain to IPv6 (send packets over IPv6)\r\n");
#endif
}

void cmd_iperf(int argc, char ** argv)
{
    int arg = 1;
    char ip_addr[128];

    struct
    {
        unsigned help : 1;
        unsigned udp : 1;
        unsigned bind : 1;
        unsigned bhost : 1;
        unsigned abort : 1;
        unsigned server : 1;
        unsigned client : 1;
        unsigned chost : 1;
        unsigned dual : 1;
        unsigned tradeoff : 1;
        unsigned time : 1;
#ifdef CONFIG_IPV6
        unsigned ipv6 : 1;
#endif
    } info;

    amount    = IPERF_CLIENT_AMOUNT;
    multicast = false;
#ifdef CONFIG_IPV6
    ipv6 = false;
#endif
    if (mcast_mac_valid)
    {
        wifi_remove_mcast_filter(mcast_mac);
        mcast_mac_valid = false;
    }

    (void) memset(&info, 0, sizeof(info));

    if (argc < 2)
    {
        (void) PRINTF("Incorrect usage\r\n");
        display_iperf_usage();
        return;
    }

    do
    {
        if (!info.help && string_equal("-h", argv[arg]))
        {
            display_iperf_usage();
            return;
        }
        else if (!info.udp && string_equal("-u", argv[arg]))
        {
            arg += 1;
            info.udp = 1;
        }
        else if (!info.abort && string_equal("-a", argv[arg]))
        {
            arg += 1;
            info.abort = 1;
        }
        else if (!info.server && string_equal("-s", argv[arg]))
        {
            arg += 1;
            info.server = 1;
        }
        else if (!info.client && string_equal("-c", argv[arg]))
        {
            arg += 1;
            info.client = 1;

            if (!info.chost && argv[arg] != NULL)
            {
                strncpy(ip_addr, argv[arg], strlen(argv[arg]));

                arg += 1;
                info.chost = 1;
            }
        }
        else if (!info.bind && string_equal("-B", argv[arg]))
        {
            arg += 1;
            info.bind = 1;

            if (!info.bhost && argv[arg] != NULL)
            {
                inet_aton(argv[arg], &bind_address);

                if (IP_IS_V4(&bind_address))
                    info.bhost = 1;

                if (ip_addr_ismulticast(&bind_address))
                {
                    multicast = true;
                    // info.bhost = 1;
                }

                arg += 1;
            }
        }
        else if (!info.time && string_equal("-t", argv[arg]))
        {
            arg += 1;
            info.time = 1;
            errno     = 0;
            amount    = -(100 * strtoul(argv[arg], NULL, 10));
            if (errno != 0)
                (void) PRINTF("Error during strtoul errno:%d", errno);
            arg += 1;
        }
#ifdef CONFIG_IPV6
        else if (!info.ipv6 && string_equal("-V", argv[arg]))
        {
            arg += 1;
            info.ipv6 = 1;
            ipv6      = true;
        }
#endif
        else if (!info.dual && string_equal("-d", argv[arg]))
        {
            arg += 1;
            info.dual = 1;
        }
        else if (!info.tradeoff && string_equal("-r", argv[arg]))
        {
            arg += 1;
            info.tradeoff = 1;
        }
        else
        {
            (void) PRINTF("Incorrect usage\r\n");
            display_iperf_usage();
            (void) PRINTF("Error: argument %d is invalid\r\n", arg);
            return;
        }
    } while (arg < argc);

#ifdef CONFIG_IPV6
    if (ipv6)
    {
        inet6_aton(ip_addr, ip_2_ip6(&server_address));
        server_address.type = IPADDR_TYPE_V6;
    }
    else
    {
#endif
        inet_aton(ip_addr, ip_2_ip4(&server_address));
#ifdef CONFIG_IPV6
        server_address.type = IPADDR_TYPE_V4;
    }
#endif

    if ((!info.abort && !info.server && !info.client) || (info.client && !info.chost) || (info.server && info.client) ||
        (info.udp
#ifdef CONFIG_IPV6
         && !info.ipv6
#endif
         && (!info.bind || !info.bhost)) ||
        ((info.dual || info.tradeoff) && !info.client) || (info.dual && info.tradeoff)
#ifdef CONFIG_IPV6
        || (info.ipv6 && info.bind)
#endif
    )
    {
        (void) PRINTF("Incorrect usage\r\n");
        display_iperf_usage();
        return;
    }

    if (info.abort != 0U)
    {
        TESTAbort();
    }
    else if (info.server != 0U)
    {
        if (info.udp != 0U)
            UDPServer();
        else
            TCPServer();
    }
    else if (info.client != 0U)
    {
        if (info.udp != 0U)
        {
            if (info.dual != 0U)
                UDPClientDual();
            else if (info.tradeoff != 0U)
                UDPClientTradeOff();
            else
                UDPClient();
        }
        else
        {
            if (info.dual != 0U)
                TCPClientDual();
            else if (info.tradeoff != 0U)
                TCPClientTradeOff();
            else
                TCPClient();
        }
    }
    else
    { /* Do Nothing */
    }
}

static struct cli_command iperf[] = {
    { "iperf", "[-s|-c <host>|-a|-h] [options]", cmd_iperf },
};

int iperf_cli_init(void)
{
    int i;
    for (i = 0; i < sizeof(iperf) / sizeof(struct cli_command); i++)
        if (cli_register_command(&iperf[i]) != 0)
            return -WM_FAIL;

    (void) memset(&ctx, 0, sizeof(struct iperf_test_context));

    timer = xTimerCreate("UDP Poll Timer", 1 / portTICK_PERIOD_MS, pdTRUE, (void *) 0, timer_poll_udp_client);
    if (timer == NULL)
    {
        (void) PRINTF("Timer creation failed!\r\n");
        while (true)
            ;
    }

    return WM_SUCCESS;
}

int iperf_cli_deinit(void)
{
    int i;

    for (i = 0; i < sizeof(iperf) / sizeof(struct cli_command); i++)
        if (cli_unregister_command(&iperf[i]) != 0)
            return -WM_FAIL;
    return WM_SUCCESS;
}
