/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

extern "C" {
#include <eth_bd.h>
}
#include <lwip/dhcp6.h>
#include <lwip/netifapi.h>

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/common/NetworkCommissioningDriver.h>

#include <EthernetInterface.h>

using namespace chip::DeviceLayer::Internal;

extern "C" struct netif * deviceInterface_getNetif(void)
{
    return &eth_mac;
}

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

static struct dhcp6 dhcp6_val;
static netif_ext_callback_t netifExtCallback;

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

CHIP_ERROR BflbEthernetDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    netif_add(&eth_mac, NULL, NULL, NULL, NULL, eth_init, ethernet_input);

    ethernet_init(ethernet_callback);

    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);

    return CHIP_NO_ERROR;
}

NetworkIterator * BflbEthernetDriver::GetNetworks()
{
    auto ret = new EthernetNetworkIterator();
    memset(ret->interfaceName, 0, sizeof(ret->interfaceName));
    if (netif_index_to_name(0, SafePointerCast<char *>(ret->interfaceName)))
    {
        ret->interfaceNameLen = NETIF_NAMESIZE;
    }
    else
    {
        ret->interfaceNameLen = 0;
    }
    return ret;
}

void BflbEthernetDriver::Shutdown()
{
    netif_remove_ext_callback(&netifExtCallback);
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
