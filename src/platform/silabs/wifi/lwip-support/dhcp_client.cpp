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

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_DHCP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp_client.h"
#include "lwip/dhcp.h"
#include <platform/silabs/wifi/WifiInterface.h>

#include <lib/support/logging/CHIPLogging.h>

#define MAX_DHCP_TRIES (4)
#define NETIF_IPV4_ADDRESS(X, Y) (((X) >> (8 * Y)) & 0xFF)

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
        ChipLogProgress(DeviceLayer, "DHCP: Starting");
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
    struct dhcp * dhcp;

    switch (dhcp_state)
    {
    case DHCP_START:
        ChipLogProgress(DeviceLayer, "DHCP: Wait addr");
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
        }
        else
        {
            dhcp = (struct dhcp *) netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

            /* DHCP timeout */
            if (dhcp->tries > MAX_DHCP_TRIES)
            {
                dhcp_state = DHCP_TIMEOUT;

                ChipLogProgress(DeviceLayer, "*ERR*DHCP: Failed");
                /* Stop DHCP */
                dhcp_stop(netif);
            }
        }
        break;

    case DHCP_LINK_DOWN:
        /* Stop DHCP */
        ChipLogProgress(DeviceLayer, "*ERR*DHCP Link down");
        dhcp_stop(netif);
        dhcp_state = DHCP_OFF;
        break;
    default:
        break;
    }
    return dhcp_state;
}

#endif /* LWIP_IPV4 && LWIP_DHCP */
