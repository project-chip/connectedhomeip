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

#include "FreeRTOS.h"
#include "dhcp_client.h"
#include "ethernetif.h"
#include "event_groups.h"
#include "task.h"
#include "wfx_host_events.h"
#include <platform/CHIPDeviceLayer.h>

#include "sl_net.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_driver.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"

#include "errno.h"
#include "sl_board_configuration.h"
#include "sl_net_si91x.h"
#include "sl_si91x_socket.h"
#include "sl_wifi_callback_framework.h"
#include "socket.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_m4_ps.h"
#endif

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace {

struct netif sta_netif;

/**
 * @brief TODO
 *
 * @param sta_if
 * @param ap_if
 */
void netif_config(struct netif * sta_if, struct netif * ap_if)
{
    if (sta_if != NULL)
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
        netif_add(sta_if,
#if LWIP_IPV4
                  (const ip4_addr_t *) &sta_ipaddr, (const ip4_addr_t *) &sta_netmask, (const ip4_addr_t *) &sta_gw,
#endif /* LWIP_IPV4 */
                  NULL, &sta_ethernetif_init, &tcpip_input);

        /* Registers the default network interface */
        netif_set_default(sta_if);
    }
}

} // namespace

/****************************************************************************
 * @fn  void wfx_lwip_set_sta_link_up(void)
 * @brief
 * Set station link status to up.
 * @param[in]  None
 * @return None
 *****************************************************************************/
void wfx_lwip_set_sta_link_up(void)
{
    netifapi_netif_set_up(&sta_netif);
    netifapi_netif_set_link_up(&sta_netif);

#if LWIP_IPV4 && LWIP_DHCP
    dhcpclient_set_link_state(LINK_UP);
#endif /* LWIP_IPV4 && LWIP_DHCP */

    /*
     * Enable IPV6
     */
#if LWIP_IPV6_AUTOCONFIG
    sta_netif.ip6_autoconfig_enabled = 1;
#endif /* LWIP_IPV6_AUTOCONFIG */
    netif_create_ip6_linklocal_address(&sta_netif, MAC_48_BIT_SET);
}

/***************************************************************************
 * @fn  void wfx_lwip_set_sta_link_down(void)
 * @brief
 * Set station link status to down.
 * @param[in] None
 * @return None
 *****************************************************************************/
void wfx_lwip_set_sta_link_down(void)
{
#if LWIP_IPV4 && LWIP_DHCP
    dhcpclient_set_link_state(LINK_DOWN);
#endif /* LWIP_IPV4 && LWIP_DHCP */

    netifapi_netif_set_link_down(&sta_netif);
    netifapi_netif_set_down(&sta_netif);
}

/***************************************************************************
 * @fn  void wfx_lwip_start(void)
 * @brief
 * Initialize the LwIP stack
 * @param[in] None
 * @return None
 *****************************************************************************/
void wfx_lwip_start(void)
{
    /* Initialize the LwIP stack */
    // netif_config(&sta_netif, NULL);

    sl_net_init((1 << 3), &client_init_configuration, nullptr, nullptr);
}

/***************************************************************************
 * @fn   struct netif *wfx_get_netif(sl_wfx_interface_t interface)
 * @brief
 * get the netif
 * @param[in] interface:
 * @return None
 *****************************************************************************/
struct netif * wfx_get_netif(sl_wfx_interface_t interface)
{
    if (interface == SL_WFX_STA_INTERFACE)
    {
        return &sta_netif;
    }

    return (struct netif *) 0;
}
