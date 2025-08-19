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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/netifapi.h"
#include "lwip/tcpip.h"

#include <platform/silabs/wifi/lwip-support/dhcp_client.h>
#include <platform/silabs/wifi/lwip-support/ethernetif.h>
#include <platform/silabs/wifi/lwip-support/lwip_netif.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>

namespace {

constexpr uint8_t kLinkUp      = 1;
constexpr uint8_t kLinkDown    = 0;
constexpr uint8_t kMac48BitSet = 1;

struct netif sStationNetworkInterface;

/**
 * @brief Configures the provided network interface
 *
 * @param[in] interface interface to be configured
 */
void ConfigureNetworkInterface(struct netif & interface)
{

#if LWIP_IPV4
    ip_addr_t sta_ipaddr;
    ip_addr_t sta_netmask;
    ip_addr_t sta_gw;

    /* Initialize the Station information */
    ip_addr_set_zero_ip4(&sta_ipaddr);
    ip_addr_set_zero_ip4(&sta_netmask);
    ip_addr_set_zero_ip4(&sta_gw);
#endif /* LWIP_IPV4 */

    /* Add station interfaces */
    netif_add(&interface,
#if LWIP_IPV4
              (const ip4_addr_t *) &sta_ipaddr, (const ip4_addr_t *) &sta_netmask, (const ip4_addr_t *) &sta_gw,
#endif /* LWIP_IPV4 */
              NULL, &sta_ethernetif_init, &tcpip_input);

    /* Registers the default network interface */
    netif_set_default(&interface);
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Lwip {

void SetLwipStationLinkUp(void)
{
    netifapi_netif_set_up(&sStationNetworkInterface);
    netifapi_netif_set_link_up(&sStationNetworkInterface);
#if LWIP_IPV4 && LWIP_DHCP
    dhcpclient_set_link_state(kLinkUp);
#endif /* LWIP_IPV4 && LWIP_DHCP */
    /*
     * Enable IPV6
     */

#if LWIP_IPV6_AUTOCONFIG
    sStationNetworkInterface.ip6_autoconfig_enabled = 1;
#endif /* LWIP_IPV6_AUTOCONFIG */
    netif_create_ip6_linklocal_address(&sStationNetworkInterface, kMac48BitSet);
}

void SetLwipStationLinkDown(void)
{
#if LWIP_IPV4 && LWIP_DHCP
    dhcpclient_set_link_state(kLinkDown);
#endif /* LWIP_IPV4 && LWIP_DHCP */
    netifapi_netif_set_link_down(&sStationNetworkInterface);
    netifapi_netif_set_down(&sStationNetworkInterface);
}

void InitializeLwip(void)
{
    /* Initialize the LwIP stack */
    ConfigureNetworkInterface(sStationNetworkInterface);
}

struct netif * GetNetworkInterface(sl_wfx_interface_t interface)
{
    if (interface == SL_WFX_STA_INTERFACE)
    {
        return &sStationNetworkInterface;
    }

    return nullptr;
}

} // namespace Lwip
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
