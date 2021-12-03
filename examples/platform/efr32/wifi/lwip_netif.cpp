#ifndef RS911X_SOCKETS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"


#include "wifi_config.h"
#include "wfx_host_events.h"

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

static void
netif_config(struct netif *sta_if, struct netif *ap_if)
{
    if (sta_if != NULL) {
        ip_addr_t sta_ipaddr;
        ip_addr_t sta_netmask;
        ip_addr_t sta_gw;

        /* Initialize the Station information */
        ip_addr_set_zero_ip4(&sta_ipaddr);
        ip_addr_set_zero_ip4(&sta_netmask);
        ip_addr_set_zero_ip4(&sta_gw);

        /* Add station interfaces */
        netif_add(sta_if, (const ip4_addr_t *) &sta_ipaddr, (const ip4_addr_t *) &sta_netmask,
                  (const ip4_addr_t *) &sta_gw, NULL,
                  &sta_ethernetif_init, &tcpip_input);

        /* Registers the default network interface */
        netif_set_default (sta_if);
    }

#ifdef SL_WFX_CONFIG_SOFTAP
    if (ap_if != NULL)
    {
        // ip_addr_t ap_ipaddr;
        // ip_addr_t ap_netmask;
        // ip_addr_t ap_gw;
        // /* Initialize the SoftAP information */
        // IP_ADDR4(&ap_ipaddr, ap_ip_addr0, ap_ip_addr1, ap_ip_addr2, ap_ip_addr3);
        // IP_ADDR4(&ap_netmask, ap_netmask_addr0, ap_netmask_addr1, ap_netmask_addr2, ap_netmask_addr3);
        // IP_ADDR4(&ap_gw, ap_gw_addr0, ap_gw_addr1, ap_gw_addr2, ap_gw_addr3);
        // netif_add(apNetif, &ap_ipaddr, &ap_netmask, &ap_gw, NULL, &ap_ethernetif_init, &ethernet_input);
    }
#endif /* SL_WFX_CONFIG_SOFTAP */
}

/****************************************************************************
 * Set station link status to up.
 *****************************************************************************/
void
wfx_lwip_set_sta_link_up(void)
{
    netifapi_netif_set_up (&sta_netif);
    netifapi_netif_set_link_up(&sta_netif);
    dhcpclient_set_link_state (1);
    /*
     * Enable IPV6
     */
    netif_create_ip6_linklocal_address (&sta_netif, 1);

}

/***************************************************************************
 * Set station link status to down.
 *****************************************************************************/
void
wfx_lwip_set_sta_link_down(void)
{
    dhcpclient_set_link_state(0);
    netifapi_netif_set_link_down(&sta_netif);
    netifapi_netif_set_down(&sta_netif);
}

void
wfx_lwip_start (void)
{
        /* Initialize the LwIP stack */
        netif_config (&sta_netif, NULL);
}
struct netif *
wfx_get_netif (sl_wfx_interface_t interface)
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
    return (struct netif *)0;
}

#endif /* RS911X_SOCKETS */
