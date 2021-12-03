/*
 * The interface in this file is used by WFXutils (and by ConnectivityManagerImpl.cpp)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "sl_status.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"


/*
 * Called from ConnectivityManagerImpl.cpp - to enable the device
 * Create the RSI task and let it deal with life.
 */
sl_status_t
wfx_wifi_start(void)
{
        if (wfx_rsi.dev_state & WFX_RSI_ST_STARTED) {
                WFX_RSI_LOG ("WIFI: Already started");
                return SL_STATUS_OK;
        }
        wfx_rsi.dev_state |= WFX_RSI_ST_STARTED;
	WFX_RSI_LOG ("WIFI: Starting..");
	/*
	 * Create the driver task
	 */
	if (xTaskCreate(wfx_rsi_task, "wfx_rsi",
			WFX_RSI_TASK_SZ, NULL, 1,
			&wfx_rsi.wlan_task) != pdPASS)
	{
		WFX_RSI_LOG("ERR: RSI task creat");
		return SL_STATUS_FAIL;
	}
	return SL_STATUS_OK;
}
void
wfx_enable_sta_mode (void)
{
	wfx_rsi.dev_state |= WFX_RSI_ST_STA_MODE;

}
bool
wfx_is_sta_mode_enabled (void)
{
        bool mode;
        mode =  !!(wfx_rsi.dev_state & WFX_RSI_ST_STA_MODE);

        //WFX_RSI_LOG ("WLAN: STA ENA: %s",  (mode ? "YES" : "NO"));
        return mode;
}

sl_wfx_state_t
wfx_get_wifi_state (void)
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_STA_DHCP_DONE) {
        return SL_WFX_STA_INTERFACE_CONNECTED;
    }
    if (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY) {
        return SL_WFX_STARTED;
    }
    return SL_WFX_NOT_INIT;
}
void
wfx_get_wifi_mac_addr (sl_wfx_interface_t interface, sl_wfx_mac_address_t *addr)
{
	sl_wfx_mac_address_t *mac;

#ifdef SL_WFX_CONFIG_SOFTAP
	mac = (interface == SL_WFX_SOFTAP_INTERFACE) ? &wfx_rsi.softap_mac : &wfx_rsi.sta_mac;
#else
	mac = &wfx_rsi.sta_mac;
#endif
	*addr = *mac;
	WFX_RSI_LOG ("WLAN:Get WiFi Mac addr %02x:%02x:%02x:%02x:%02x:%02x",
                     mac->octet [0], mac->octet [1],
                     mac->octet [2], mac->octet [3],
                     mac->octet [4], mac->octet [5]);
}
void
wfx_set_wifi_provision (wfx_wifi_provision_t *cfg)
{
	WFX_RSI_LOG ("WLAN:Provision:SSID=%s\n", &wfx_rsi.sec.ssid [0]);

	wfx_rsi.sec = *cfg;
	wfx_rsi.dev_state |= WFX_RSI_ST_STA_PROVISIONED;
}
bool
wfx_is_sta_provisioned (void)
{

        //WFX_RSI_LOG ("WLAN:WiFi %s provisioned(SSID=%s)",
        //           ((wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED) ? "IS" : "NOT"),
        //           &wfx_rsi.sec.ssid [0]);
	return (wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED) ? true : false;
}
void
wfx_clear_wifi_provision (void)
{
	memset (&wfx_rsi.sec, 0, sizeof (wfx_rsi.sec));
	wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_PROVISIONED;
	WFX_RSI_LOG ("WLAN:Clr WiFi provision");
}
/*
 * Start a JOIN command to the AP - Done by the wfx_rsi task
 */
sl_status_t
wfx_connect_to_ap (void)
{
	if (wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED) {
		WFX_RSI_LOG ("WLAN: Connecting to AP (%s==%s)",
                             &wfx_rsi.sec.ssid [0],
                             &wfx_rsi.sec.passkey [0]);
		xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_START_JOIN);
	} else {
		WFX_RSI_LOG ("*ERR* AP: Not provisioned");
		return SL_STATUS_INVALID_CONFIGURATION;
	}

	return SL_STATUS_OK;
}
void
wfx_setup_ip6_link_local (sl_wfx_interface_t whichif)
{
	WFX_RSI_LOG ("Setup-IP6: TODO");
}

bool
wfx_is_sta_connected (void)
{
        bool val;

        val = (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) ? true : false;
        WFX_RSI_LOG ("WLAN: STA %s connected", (val ? "IS" : "NOT"));

	return  val;
}

wifi_mode_t
wfx_get_wifi_mode ()
{
	if (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY) return WIFI_MODE_STA;
	return WIFI_MODE_NULL;
}
sl_status_t
wfx_sta_discon (void)
{
	WFX_RSI_LOG ("STA-Discon: TODO");
	/* TODO - Disconnect station mode from connected AP */

        return SL_STATUS_OK;
}
bool
wfx_have_ipv4_addr (sl_wfx_interface_t which_if)
{
	if (which_if == SL_WFX_STA_INTERFACE) {
		return (wfx_rsi.dev_state & WFX_RSI_ST_STA_DHCP_DONE) ? true : false;
	} else {
		return false; /* TODO */
	}
}
bool
wfx_hw_ready (void)
{
        return (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY) ? true : false;

}
