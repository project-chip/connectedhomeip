/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <eth_bd.h>
#include <lwip/dhcp6.h>
#include <lwip/netifapi.h>

#include "EthernetInterface.h"
static struct dhcp6 dhcp6_val;
static netif_ext_callback_t netifExtCallback;

static void netif_status_callback(struct netif * netif)
{
    if ((netif->flags & NETIF_FLAG_UP) && !ip4_addr_isany(netif_ip4_addr(netif)))
    {
        printf("IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
        printf("MASK: %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
        printf("Gateway: %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif)));

        ethernetInterface_eventGotIP(netif);
    }
}

static int ethernet_callback(eth_link_state val)
{
    switch (val)
    {
    case ETH_INIT_STEP_LINKUP:
        break;
    case ETH_INIT_STEP_READY:
        netifapi_netif_set_default(&eth_mac);
        netifapi_netif_set_up(&eth_mac);

        // netifapi_netif_set_up((struct netif *)&obj->netif);
        netif_create_ip6_linklocal_address(&eth_mac, 1);
        eth_mac.ip6_autoconfig_enabled = 1;
        dhcp6_set_struct(&eth_mac, &dhcp6_val);
        dhcp6_enable_stateless(&eth_mac);

        /* start dhcp */
        netifapi_dhcp_start(&eth_mac);
        break;
    case ETH_INIT_STEP_LINKDOWN:
        break;
    }

    return 0;
}

void ethernetInterface_init(void)
{
    netif_add(&eth_mac, NULL, NULL, NULL, NULL, eth_init, ethernet_input);

    ethernet_init(ethernet_callback);

    /* Set callback to be called when interface is brought up/down or address is changed while up */
    netif_set_status_callback(&eth_mac, netif_status_callback);

    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);
}

struct netif * deviceInterface_getNetif(void)
{
    return &eth_mac;
}
