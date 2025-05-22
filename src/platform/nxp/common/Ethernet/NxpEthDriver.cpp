/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "NxpEthDriver.h"
#include "app_ethernet.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

/* lwIP Includes */
#include "fsl_silicon_id.h"

NETIF_DECLARE_EXT_CALLBACK(netif_callback)

using namespace ::chip;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

void NxpEthDriver::print_ip_addresses(struct netif * netif)
{
#if INET_CONFIG_ENABLE_IPV4
    ChipLogProgress(DeviceLayer, "IPv4 Address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
#endif
    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        const char * str_ip = "-";
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
        {
            str_ip = ip6addr_ntoa(netif_ip6_addr(netif, i));
        }
        ChipLogProgress(DeviceLayer, " IPv6 Address%d    : %s", i, str_ip);
    }
}

void NxpEthDriver::eth_netif_ext_status_callback(struct netif * netif, netif_nsc_reason_t reason,
                                                 const netif_ext_callback_args_t * args)
{
    if (((LWIP_NSC_IPV6_ADDR_STATE_CHANGED & reason) || (LWIP_NSC_IPV6_SET & reason) || (LWIP_NSC_IPV4_ADDRESS_CHANGED & reason) ||
         (LWIP_NSC_IPV4_ADDRESS_CHANGED & reason)) &&
        args)
    {
        ChipDeviceEvent event;
        print_ip_addresses(netif);
        event.Type = DeviceEventType::kPlatformNxpIpChangeEvent;

        CHIP_ERROR err = PlatformMgr().PostEvent(&event);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

CHIP_ERROR NxpEthDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    err_t err;
    ethernetif_config_t enet_config = {
        .phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
    };

    /* Set MAC address. */
    (void) SILICONID_ConvertToMacAddr(&enet_config.macAddress);

#if LWIP_IPV4
    err = netifapi_netif_add(&netif_app, NULL, NULL, NULL, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);
#else
    err = netifapi_netif_add(&netif_app, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);
#endif
    VerifyOrReturnError(err == ERR_OK, CHIP_ERROR_INTERNAL);
    netifapi_netif_set_default(&netif_app);
    netifapi_netif_set_up(&netif_app);

#if LWIP_IPV4
    netifapi_dhcp_start(&netif_app);
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
    LOCK_TCPIP_CORE();
    netif_create_ip6_linklocal_address(&netif_app, 1);
    UNLOCK_TCPIP_CORE();
#endif // LWIP_IPV6

    /* register for netif events when started on first netif */
    netif_add_ext_callback(&netif_callback, eth_netif_ext_status_callback);

    if (err != ERR_OK)
    {
        ChipLogProgress(
            DeviceLayer,
            "Ethernet interface initialization failed. Make sure there is internet conectivity on the board and reset!");
        chipDie();
    }
    ChipLogProgress(DeviceLayer, "Ethernet interface initialization done");
    return CHIP_NO_ERROR;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
