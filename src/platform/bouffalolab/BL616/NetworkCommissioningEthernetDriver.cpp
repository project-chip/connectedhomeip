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

#include <FreeRTOS.h>
#include <lwip/dhcp6.h>
#include <lwip/ethip6.h>
#include <lwip/netifapi.h>
#include <task.h>

extern "C" {
#include <eth_phy.h>
#include <lwip_netif_emac.h>
}

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/common/NetworkCommissioningDriver.h>

using namespace chip::DeviceLayer::Internal;

static struct netif gnetif;
static netif_ext_callback_t netifExtCallback;

extern "C" struct netif * deviceInterface_getNetif(void)
{
    return &gnetif;
}

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

void lwip_status_update_task(void * pvParameters)
{
    /* Infinite loop */
    while (1)
    {
        vTaskDelay(200);

        /* update link status */
        eth_link_state_update(&gnetif);

        LOCK_TCPIP_CORE();
        if (!gnetif.ip6_autoconfig_enabled && netif_is_link_up(&gnetif))
        {
            gnetif.flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_MLD6;
            gnetif.output_ip6 = ethip6_output;
            netif_create_ip6_linklocal_address(&gnetif, 1);
            gnetif.ip6_autoconfig_enabled = 1;
        }
        UNLOCK_TCPIP_CORE();

        /* update dhcp status */
        dhcp_sta_update(&gnetif);
    }
}

void network_netif_ext_callback(struct netif * nif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args)
{
    if (((LWIP_NSC_IPV6_ADDR_STATE_CHANGED | LWIP_NSC_IPV6_SET) & reason) && args)
    {
        if (args->ipv6_addr_state_changed.addr_index >= LWIP_IPV6_NUM_ADDRESSES ||
            ip6_addr_islinklocal(netif_ip6_addr(nif, args->ipv6_addr_state_changed.addr_index)))
        {
            return;
        }

        if (netif_ip6_addr_state(nif, args->ipv6_addr_state_changed.addr_index) != args->ipv6_addr_state_changed.old_state &&
            ip6_addr_ispreferred(netif_ip6_addr_state(nif, args->ipv6_addr_state_changed.addr_index)))
        {
            ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        }
    }

    if ((LWIP_NSC_IPV4_SETTINGS_CHANGED & reason) && args)
    {
        if (!ip4_addr_isany(netif_ip4_addr(nif)) && !ip4_addr_isany(netif_ip4_gw(nif)))
        {
            ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        }
    }
}

CHIP_ERROR BflbEthernetDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    VerifyOrDie(netif_add(&gnetif, NULL, NULL, NULL, NULL, &eth_emac_if_init, &tcpip_input));

    LOCK_TCPIP_CORE();
    netif_set_default(&gnetif);
    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);
    UNLOCK_TCPIP_CORE();

    xTaskCreate(lwip_status_update_task, (char *) "lwip_sta_update", 256, NULL, osPriorityHigh, NULL);

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
