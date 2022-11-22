/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#if LWIP_IPV4 && LWIP_DHCP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"

#include "dhcp_client.h"
#include "lwip/dhcp.h"
#include "wfx_host_events.h"
#include "wifi_config.h"

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "event_groups.h"

#define MAX_DHCP_TRIES 4
#define NETIF_IPV4_ADDRESS(X, Y) (((X) >> (8 * Y)) & 0xFF)

/* Station IP address */
uint8_t sta_ip_addr0      = STA_IP_ADDR0_DEFAULT;
uint8_t sta_ip_addr1      = STA_IP_ADDR1_DEFAULT;
uint8_t sta_ip_addr2      = STA_IP_ADDR2_DEFAULT;
uint8_t sta_ip_addr3      = STA_IP_ADDR3_DEFAULT;
uint8_t sta_netmask_addr0 = STA_NETMASK_ADDR0_DEFAULT;
uint8_t sta_netmask_addr1 = STA_NETMASK_ADDR1_DEFAULT;
uint8_t sta_netmask_addr2 = STA_NETMASK_ADDR2_DEFAULT;
uint8_t sta_netmask_addr3 = STA_NETMASK_ADDR3_DEFAULT;
uint8_t sta_gw_addr0      = STA_GW_ADDR0_DEFAULT;
uint8_t sta_gw_addr1      = STA_GW_ADDR1_DEFAULT;
uint8_t sta_gw_addr2      = STA_GW_ADDR2_DEFAULT;
uint8_t sta_gw_addr3      = STA_GW_ADDR3_DEFAULT;

/// Current DHCP state machine state.
static volatile uint8_t dhcp_state = DHCP_OFF;

/*****************************************************************************
 * @fn  void dhcpclient_set_link_state(int link_up)
 * @brief
 * Notify DHCP client task about the wifi status
 * @param link_up link status
 * @return None
 ******************************************************************************/
void dhcpclient_set_link_state(int link_up)
{
    if (link_up)
    {
        dhcp_state = DHCP_START;
        SILABS_LOG("DHCP: Starting");
    }
    else
    {
        /* Update DHCP state machine */
        dhcp_state = DHCP_LINK_DOWN;
    }
}

/**********************************************************************************
 * @fn  uint8_t dhcpclient_poll(void *arg)
 * @brief
 * Don't need a task here. We get polled every 250ms
 * @return  None
 ************************************************************************************/
uint8_t dhcpclient_poll(void * arg)
{
    struct netif * netif = (struct netif *) arg;
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    struct dhcp * dhcp;

    switch (dhcp_state)
    {
    case DHCP_START:
        SILABS_LOG("DHCP: Wait addr");
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);
        dhcp_start(netif);
        dhcp_state = DHCP_WAIT_ADDRESS;
        break;

    case DHCP_WAIT_ADDRESS:
        if (dhcp_supplied_address(netif))
        {
            dhcp_state = DHCP_ADDRESS_ASSIGNED;

            uint64_t addr = netif->ip_addr.u_addr.ip4.addr;
            SILABS_LOG("DHCP IP: %d.%d.%d.%d", NETIF_IPV4_ADDRESS(addr, 0), NETIF_IPV4_ADDRESS(addr, 1),
                       NETIF_IPV4_ADDRESS(addr, 2), NETIF_IPV4_ADDRESS(addr, 3));
        }
        else
        {
            dhcp = (struct dhcp *) netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

            /* DHCP timeout */
            if (dhcp->tries > MAX_DHCP_TRIES)
            {
                dhcp_state = DHCP_TIMEOUT;

                SILABS_LOG("*ERR*DHCP: Failed");
                /* Stop DHCP */
                dhcp_stop(netif);

                /* TODO - I am not sure that this is best */
                /* Static address used */
                IP_ADDR4(&ipaddr, sta_ip_addr0, sta_ip_addr1, sta_ip_addr2, sta_ip_addr3);
                IP_ADDR4(&netmask, sta_netmask_addr0, sta_netmask_addr1, sta_netmask_addr2, sta_netmask_addr3);
                IP_ADDR4(&gw, sta_gw_addr0, sta_gw_addr1, sta_gw_addr2, sta_gw_addr3);
                netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
            }
        }
        break;

    case DHCP_LINK_DOWN:
        /* Stop DHCP */
        SILABS_LOG("*ERR*DHCP Link down");
        dhcp_stop(netif);
        dhcp_state = DHCP_OFF;
        break;
    default:
        break;
    }
    return dhcp_state;
}

#endif /* LWIP_IPV4 && LWIP_DHCP */
