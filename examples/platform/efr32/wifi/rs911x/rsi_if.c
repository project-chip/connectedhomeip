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

#include "rsi_driver.h"
#include "rsi_wlan_non_rom.h"


#include "rsi_wlan_config.h"
#include "rsi_data_types.h"
#include "rsi_common_apis.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan.h"
#include "rsi_utils.h"
#include "rsi_socket.h"
#include "rsi_nwk.h"
//#include "rsi_wlan_non_rom.h"
#include "rsi_bootup_config.h"
#include "rsi_error.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"
#include "dhcp_client.h"


//#include "rsi_wlan_config.h"

/*
 * This file implements the interface to the RSI SAPIs
 */
static uint8_t wfx_rsi_drv_buf [WFX_RSI_BUF_SZ];
static void
wfx_rsi_join_cb (uint16_t status, const uint8_t *buf, const uint16_t len)
{
	WFX_RSI_LOG("WLAN: Join Sts=%d", status);
	wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTING;
	if (status != RSI_SUCCESS) {
		/*
		 * We should enable retry.. (Need config variable for this)
		 */
                WFX_RSI_LOG ("WFX:Join fail, retry=%d", wfx_rsi.join_retries);
#if (WFX_RSI_CONFIG_MAX_JOIN != 0)
		if (++wfx_rsi.join_retries < WFX_RSI_CONFIG_MAX_JOIN)
#endif
		{
			xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_START_JOIN);

		}
	} else {
		/*
		 * Join was complete - Do the DHCP
		 */
                WFX_RSI_LOG ("WFX:Join done");
#ifdef RS911X_SOCKETS
		xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_DO_DHCP);
#else
		xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_CONN);
#endif
	}
}
static void
wfx_rsi_join_fail_cb (uint16_t status, uint8_t *buf, uint32_t len)
{
	WFX_RSI_LOG("ERR: Join WLAN");
}
#ifdef RS911X_SOCKETS
/*
 * DHCP should end up here.
 */
static void
wfx_rsi_ipchange_cb (uint16_t status, uint8_t *buf, uint32_t len)
{

	WFX_RSI_LOG("ERR: IP Change Sts=%d", status);
	if (status != RSI_SUCCESS) {
		/* Restart DHCP? */
		xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_DO_DHCP);
	} else {
		wfx_rsi.dev_state |= WFX_RSI_ST_STA_DHCP_DONE;
		xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_DHCP_DONE);
	}
}
#else
/*
 * Got RAW WLAN data pkt
 */
static void
wfx_rsi_wlan_pkt_cb (uint16_t status, uint8_t *buf, uint32_t len)
{
        //WFX_RSI_LOG("WLAN:in pkt:sts=%d, Len=%d", status, len);
	if (status != RSI_SUCCESS) {
                return;
        }
        wfx_host_received_sta_frame_cb (buf, len);
}
#endif /* !Socket support */
static int32_t
wfx_rsi_init (void)
{
	int32_t status;
	uint8_t buf [128];
        extern void rsi_hal_board_init (void);

	/* Get the GPIOs/PINs set-up
	 */
        //rsi_hal_board_init ();
	WFX_RSI_LOG ("WFX:RSI:Starting(HEAP_SZ=%d)..", SL_HEAP_SIZE);
	//! Driver initialization
	status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
	if ((status < 0) || (status > WFX_RSI_BUF_SZ)) {
		WFX_RSI_LOG("ERR: RSI drv init");
		return status;
	}

	WFX_RSI_LOG ("WFX:RSI:Dev init..");
	//! Redpine module intialisation
	if ((status = rsi_device_init(LOAD_NWP_FW)) != RSI_SUCCESS) {
		WFX_RSI_LOG("ERR: RSI dev init");
		return status;
	}
        WFX_RSI_LOG ("WFX:Start wireless drv task");
	/*
	 * Create the driver task
	 */
	if (xTaskCreate((TaskFunction_t)rsi_wireless_driver_task,
			"rsi_drv",
			WFX_RSI_WLAN_TASK_SZ, NULL, 1,
			&wfx_rsi.drv_task) != pdPASS)
	{
		WFX_RSI_LOG("ERR: RSI task creat");
		return RSI_ERROR_INVALID_PARAM;
	}

	WFX_RSI_LOG ("RSI: Wireless init");
	if ((status = rsi_wireless_init(0, 0)) != RSI_SUCCESS) {

		WFX_RSI_LOG("ERR: RSI wireless init");
		return status;
	}
	WFX_RSI_LOG ("RSI: Get FW Ver");
	/*
	 * Get the MAC and other info to let the user know about it.
	 */
	if (rsi_wlan_get (RSI_FW_VERSION, buf, sizeof (buf)) != RSI_SUCCESS) {
		WFX_RSI_LOG("ERR: RSI FW-Ver");
		return status;
	}
	buf [sizeof (buf) -1] = 0;
	WFX_RSI_LOG ("RSI-FW: Ver=%s", buf);
	//! Send feature frame
	if ((status = rsi_send_feature_frame()) != RSI_SUCCESS) {
		WFX_RSI_LOG("ERR: RSI feat frame");
		return status;
	}
        WFX_RSI_LOG ("WFX: Sent RSI Feature frame");
        (void)rsi_wlan_radio_init (); /* Required so we can get MAC address */
	if ((status = rsi_wlan_get (RSI_MAC_ADDRESS, &wfx_rsi.sta_mac.octet [0], 6)) != RSI_SUCCESS) {
		WFX_RSI_LOG("ERR: RSI MAC-Addr");
		return status;
        }
	WFX_RSI_LOG ("WLAN: MAC %02x:%02x:%02x %02x:%02x:%02x",
		   wfx_rsi.sta_mac.octet [0], wfx_rsi.sta_mac.octet [1],
		   wfx_rsi.sta_mac.octet [2], wfx_rsi.sta_mac.octet [3],
		   wfx_rsi.sta_mac.octet [4], wfx_rsi.sta_mac.octet [5]);
	wfx_rsi.events = xEventGroupCreate();
	/*
	 * Register callbacks - We are only interested in the connectivity CBs
	 */
#if 0 /* missing in sapi library */
	if ((status = rsi_wlan_register_callbacks (RSI_WLAN_JOIN_RESPONSE_HANDLER, wfx_rsi_join_cb)) != RSI_SUCCESS) {
                WFX_RSI_LOG ("*ERR*RSI CB register join cb");
                return status;
        }
#endif /* missing in sapi */
	if ((status = rsi_wlan_register_callbacks (RSI_JOIN_FAIL_CB, wfx_rsi_join_fail_cb)) != RSI_SUCCESS) {
                WFX_RSI_LOG ("*ERR*RSI CB register join fail");
                return status;
        }
#ifdef RS911X_SOCKETS
	(void)rsi_wlan_register_callbacks (RSI_IP_CHANGE_NOTIFY_CB, wfx_rsi_ipchange_cb);
#else
	if ((status = rsi_wlan_register_callbacks (RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, wfx_rsi_wlan_pkt_cb)) != RSI_SUCCESS)
        {
                WFX_RSI_LOG ("*ERR*RSI CB register data-notify");
                return status;
        }
#endif
	wfx_rsi.dev_state |= WFX_RSI_ST_DEV_READY;
	WFX_RSI_LOG ("WFX:RSI: OK");

	return RSI_SUCCESS;
}
void wfx_show_err (char *msg)
{
        WFX_RSI_LOG ("RSI ERR: %s", msg);
}
/*
 * Start an async Join command
 */
static void
wfx_rsi_do_join (void)
{
	int32_t status;

        if (wfx_rsi.dev_state & (WFX_RSI_ST_STA_CONNECTING|WFX_RSI_ST_STA_CONNECTED)) {
                WFX_RSI_LOG ("WLAN:Not Joining - Already in progress");
        } else {
                WFX_RSI_LOG ("WLAN: Connecting to %s==%s, sec=%d",
                             &wfx_rsi.sec.ssid [0],
                             &wfx_rsi.sec.passkey [0],
                             wfx_rsi.sec.security);
                /*
                 * Join the network
                 */
                /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
                 * Right now it's done by hand - we need something better
                 */
                wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTING;
                if ((status = rsi_wlan_connect_async ((int8_t *)&wfx_rsi.sec.ssid [0],
                                                      (rsi_security_mode_t)wfx_rsi.sec.security,
                                                      &wfx_rsi.sec.passkey [0],
                                                      wfx_rsi_join_cb )) != RSI_SUCCESS)
                {
                        wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTING;
                        WFX_RSI_LOG ("ERR: WLAN Conn");
                        /* TODO - Start a timer.. to retry */
                } else {
                        WFX_RSI_LOG ("WLAN:Starting JOIN to %s\n", (char *)&wfx_rsi.sec.ssid [0]);
                }
        }
}

/*
 * The main WLAN task - started by wfx_wifi_start () that interfaces with RSI.
 * The rest of RSI stuff come in call-backs.
 * The initialization has been already done.
 */
/* ARGSUSED */
void
wfx_rsi_task (void *arg)
{
	EventBits_t flags;
#ifndef RS911X_SOCKETS
        TickType_t last_dhcp_poll, now;
        void *sta_netif;
#endif
	(void)arg;
	if (wfx_rsi_init () != RSI_SUCCESS) {
		/* :-(
		 */
                WFX_RSI_LOG ("ERR: rsi init failed");
		return;
	}
#ifndef RS911X_SOCKETS
        wfx_lwip_start ();
        last_dhcp_poll = xTaskGetTickCount ();
        sta_netif = wfx_get_netif (SL_WFX_STA_INTERFACE);
#endif
	wfx_started_notify ();

        WFX_RSI_LOG ("WLAN:Starting Event Wait");
	for (;;) {
		/*
		 * This is the main job of this task.
		 * Wait for commands from the ConnectivityManager
		 * Make state changes (based on call backs)
		 */
		flags = xEventGroupWaitBits(wfx_rsi.events,
                                    WFX_EVT_STA_CONN
                                            | WFX_EVT_STA_DISCONN
					    | WFX_EVT_STA_START_JOIN
#ifdef RS911X_SOCKETS
					    | WFX_EVT_STA_DO_DHCP
					    | WFX_EVT_STA_DHCP_DONE
#endif
#ifdef SL_WFX_CONFIG_SOFTAP
					    | WFX_EVT_AP_START | WFX_EVT_AP_STOP
#endif /* SL_WFX_CONFIG_SOFTAP */
#ifdef SL_WFX_CONFIG_SCAN
					    | WFX_EVT_SCAN_COMPLETE
#endif /* SL_WFX_CONFIG_SCAN */
					    | 0,
					    pdTRUE, /* Clear the bits */
                                            pdFALSE, /* Wait for any bit */
                                            pdMS_TO_TICKS (250));

                if (flags) {
                        WFX_RSI_LOG ("WFX:RSI Wait EVT=%x", flags);
                }
#ifdef RS911X_SOCKETS
		if (flags & WFX_EVT_STA_DO_DHCP) {
			/*
			 * Do DHCP -
			 */
			if ((status = rsi_config_ipaddress (RSI_IP_VERSION_4, RSI_DHCP|RSI_DHCP_UNICAST_OFFER,
							    NULL, NULL, NULL,
							    &wfx_rsi.ip4_addr [0], 4, 0)) != RSI_SUCCESS)
			{
				/* We should try this again.. (perhaps sleep) */
				/* TODO - Figure out what to do here */
			}
		}
#else /* !RS911X_SOCKET - using LWIP */
                /*
                 * Let's handle DHCP polling here
                 */
                if (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) {
                        if ((now = xTaskGetTickCount ()) > (last_dhcp_poll + pdMS_TO_TICKS (250))) {
                                dhcpclient_poll (sta_netif);
                                last_dhcp_poll = now;
                        }
                }
#endif /* RS911X_SOCKETS */
		if (flags & WFX_EVT_STA_START_JOIN) {
                        wfx_rsi_do_join ();
		}
		if (flags & WFX_EVT_STA_CONN) {
			/*
			 * Initiate the Join command (assuming we have been provisioned)
			 */
                        WFX_RSI_LOG ("WLAN: Starting LWIP STA");
                        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTED;
                        wfx_lwip_set_sta_link_up();
                        /* We need to get AP Mac - TODO */
			wfx_connected_notify (0, &wfx_rsi.ap_mac);
		}
		if (flags & WFX_EVT_STA_DISCONN) {
			wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_READY|WFX_RSI_ST_STA_CONNECTING|WFX_RSI_ST_STA_CONNECTED|WFX_RSI_ST_STA_DHCP_DONE);
                        WFX_RSI_LOG ("WLAN: TODO-Discon Notify");
                        wfx_lwip_set_sta_link_down();

		}
#ifdef SL_WFX_CONFIG_SCAN
		if (flags & WFX_EVT_SCAN_COMPLETE) {
		}
#endif /* SL_WFX_CONFIG_SCAN */
#ifdef SL_WFX_CONFIG_SOFTAP
		/* TODO */
		if (flags & WFX_EVT_AP_START) {
		}
		if (flags & WFX_EVT_AP_STOP) {
		}
#endif /* SL_WFX_CONFIG_SOFTAP */
	}
}
void
wfx_dhcp_got_ipv4 (uint32_t ip)
{
        /* Acquire the new IP address
         */
        wfx_rsi.ip4_addr [0] = (ip) & 0xff;
        wfx_rsi.ip4_addr [1] = (ip >> 8) & 0xff;
        wfx_rsi.ip4_addr [2] = (ip >> 16) & 0xff;
        wfx_rsi.ip4_addr [3] = (ip >> 24) & 0xff;
        WFX_RSI_LOG ("WLAN: DHCP Ok: IP=%d.%d.%d.%d",
                     wfx_rsi.ip4_addr [0],
                     wfx_rsi.ip4_addr [1],
                     wfx_rsi.ip4_addr [2],
                     wfx_rsi.ip4_addr [3]);
        /* Notify the Connectivity Manager - via the app */
        wfx_ip_changed_notify (1);
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_READY;
}
/*
 * WARNING - Taken from RSI and broken up
 * This is my own RSI stuff for not copying code and allocating an extra
 * level of indirection - when using LWIP buffers
 */
void *
wfx_rsi_alloc_pkt ()
{
        rsi_pkt_t *pkt;
        // Allocate packet to send data
        if ((pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool)) == NULL) {
                return (void *)0;
        }

        return (void *)pkt;
}
void
wfx_rsi_pkt_add_data (void *p, uint8_t *buf, uint16_t len, uint16_t off)
{
        rsi_pkt_t *pkt;

        pkt = (rsi_pkt_t *)p;
        memcpy (((char *)pkt->data) + off, buf, len);
}
int32_t
wfx_rsi_send_data (void *p, uint16_t len)
{
        int32_t status;
        register uint8_t *host_desc;
        rsi_pkt_t *pkt;

        pkt = (rsi_pkt_t *)p;
        host_desc = pkt->desc;
        memset(host_desc, 0, RSI_HOST_DESC_LENGTH);
        rsi_uint16_to_2bytes(host_desc, (len & 0xFFF));

        // Fill packet type
        host_desc[1] |= (RSI_WLAN_DATA_Q << 4);
        host_desc[2] |= 0x01;

        rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

#ifndef RSI_SEND_SEM_BITMAP
        rsi_driver_cb_non_rom->send_wait_bitmap |= BIT(0);
#endif
        // Set TX packet pending event
        rsi_set_event(RSI_TX_EVENT);

        if (rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->send_data_sem, RSI_SEND_DATA_RESPONSE_WAIT_TIME)
            != RSI_ERROR_NONE) {
                return RSI_ERROR_RESPONSE_TIMEOUT;
        }
        status = rsi_wlan_get_status();

        return status;
}
#if 0
int32_t rsi_wlan_send_data_xx(uint8_t *buffer, uint32_t length)
{
  int32_t status = RSI_SUCCESS;
  uint8_t *host_desc;
  rsi_pkt_t *pkt;

  // If buffer is not valid
  if ((buffer == NULL) || (length == 0)) {
    // Return packet allocation failure error
    return RSI_ERROR_INVALID_PARAM;
  }

  // Allocate packet to send data
  pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool);

  if (pkt == NULL) {
    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  // Get host descriptor pointer
  host_desc = pkt->desc;

  // Memset host descriptor
  memset(host_desc, 0, RSI_HOST_DESC_LENGTH);

  // Fill host descriptor
  rsi_uint16_to_2bytes(host_desc, (length & 0xFFF));

  // Fill packet type
  host_desc[1] |= (RSI_WLAN_DATA_Q << 4);
  host_desc[2] |= 0x01;

  // Copy data to be sent
  memcpy(pkt->data, buffer, length);

  // Enqueue packet to WLAN TX queue
  rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

#ifndef RSI_SEND_SEM_BITMAP
  rsi_driver_cb_non_rom->send_wait_bitmap |= BIT(0);
#endif
  // Set TX packet pending event
  rsi_set_event(RSI_TX_EVENT);

  if (rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->send_data_sem, RSI_SEND_DATA_RESPONSE_WAIT_TIME)
      != RSI_ERROR_NONE) {
    return RSI_ERROR_RESPONSE_TIMEOUT;
  }
  status = rsi_wlan_get_status();
  // Return status
  return status;
}
#endif
struct wfx_rsi wfx_rsi;
