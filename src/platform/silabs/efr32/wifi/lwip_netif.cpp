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

#ifndef RS911X_SOCKETS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"

#include "wfx_host_events.h"
#include "wifi_config.h"

#include "AppConfig.h"
#include "dhcp_client.h"
#include "ethernetif.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include <platform/CHIPDeviceLayer.h>
using namespace ::chip;
using namespace ::chip::DeviceLayer;
static struct netif sta_netif;

#ifdef SL_WFX_CONFIG_SOFTAP
static struct netif ap_netif;
#endif

/****************************************************************************
 * @fn   static void netif_config(struct netif *sta_if, struct netif *ap_if)
 * @brief
 *      netif configuration
 * @param[in]  sta_if:
 * @param[in]  ap_if:
 * @return None
 *****************************************************************************/
static void netif_config(struct netif * sta_if, struct netif * ap_if)
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
    netif_config(&sta_netif, NULL);
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
#ifdef SL_WFX_CONFIG_SOFTAP
    else if (interface == SL_WFX_SOFTAP_INTERFACE)
    {
        return &ap_netif;
    }
#endif
    return (struct netif *) 0;
}

#endif /* RS911X_SOCKETS */
