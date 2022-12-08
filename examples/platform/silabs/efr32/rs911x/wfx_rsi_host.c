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

/* wfxRsi Task will use as its stack */
StackType_t wfxRsiTaskStack[WFX_RSI_TASK_SZ] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
StaticTask_t wfxRsiTaskBuffer;

/*********************************************************************
 * @fn  sl_status_t wfx_wifi_start(void)
 * @brief
 * Called from ConnectivityManagerImpl.cpp - to enable the device
 * Create the RSI task and let it deal with life.
 * @param[in]  None
 * @return  Returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t wfx_wifi_start(void)
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_STARTED)
    {
        WFX_RSI_LOG("%s: already started.", __func__);
        return SL_STATUS_OK;
    }
    wfx_rsi.dev_state |= WFX_RSI_ST_STARTED;
    WFX_RSI_LOG("%s: starting..", __func__);
    /*
     * Create the Wifi driver task
     */
    wfx_rsi.wlan_task = xTaskCreateStatic(wfx_rsi_task, "wfx_rsi", WFX_RSI_TASK_SZ, NULL, WLAN_DRIVER_TASK_PRIORITY,
                                          wfxRsiTaskStack, &wfxRsiTaskBuffer);

    if (NULL == wfx_rsi.wlan_task)
    {
        WFX_RSI_LOG("%s: error: failed to create task.", __func__);
        return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
}

/*********************************************************************
 * @fn  void wfx_enable_sta_mode(void)
 * @brief
 *      driver enable the STA mode
 * @param[in]  None
 * @return   None
 ***********************************************************************/
void wfx_enable_sta_mode(void)
{
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_MODE;
}

/*********************************************************************
 * @fn  bool wfx_is_sta_mode_enabled(void)
 * @brief
 *      driver enabled the STA mode
 * @param[in]  None
 * @return   mode
 ***********************************************************************/
bool wfx_is_sta_mode_enabled(void)
{
    bool mode;
    mode = !!(wfx_rsi.dev_state & WFX_RSI_ST_STA_MODE);
    // WFX_RSI_LOG("%s: %d", __func__, (mode ? "yes" : "no"));
    return mode;
}

/*********************************************************************
 * @fn  sl_wfx_state_t wfx_get_wifi_state(void)
 * @brief
 *      get the wifi state
 * @param[in]  None
 * @return   return SL_WFX_NOT_INIT if successful,
 *         SL_WFX_started otherwise
 ***********************************************************************/
sl_wfx_state_t wfx_get_wifi_state(void)
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_STA_DHCP_DONE)
    {
        return SL_WFX_STA_INTERFACE_CONNECTED;
    }
    if (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY)
    {
        return SL_WFX_STARTED;
    }
    return SL_WFX_NOT_INIT;
}

/*********************************************************************
 * @fn  sl_wfx_state_t wfx_get_wifi_state(void)
 * @brief
 *      get the wifi mac address
 * @param[in]  Interface:
 * @param[in]  addr : address
 * @return
 *       None
 ***********************************************************************/
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t * addr)
{
    sl_wfx_mac_address_t * mac;

#ifdef SL_WFX_CONFIG_SOFTAP
    mac = (interface == SL_WFX_SOFTAP_INTERFACE) ? &wfx_rsi.softap_mac : &wfx_rsi.sta_mac;
#else
    mac = &wfx_rsi.sta_mac;
#endif
    *addr = *mac;
    WFX_RSI_LOG("%s: %02x:%02x:%02x:%02x:%02x:%02x", __func__, mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3],
                mac->octet[4], mac->octet[5]);
}

/*********************************************************************
 * @fn  void wfx_set_wifi_provision(wfx_wifi_provision_t *cfg)
 * @brief
 *      Driver set the wifi provision
 * @param[in]  cfg:  wifi configuration
 * @return
 *       None
 ***********************************************************************/
void wfx_set_wifi_provision(wfx_wifi_provision_t * cfg)
{
    WFX_RSI_LOG("%s: SSID: %s", __func__, &wfx_rsi.sec.ssid[0]);

    wfx_rsi.sec = *cfg;
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_PROVISIONED;
}

/*********************************************************************
 * @fn  bool wfx_get_wifi_provision(wfx_wifi_provision_t *wifiConfig)
 * @brief
 *      Driver get the wifi provision
 * @param[in]  wifiConfig:  wifi configuration
 * @return  return false if successful,
 *        true otherwise
 ***********************************************************************/
bool wfx_get_wifi_provision(wfx_wifi_provision_t * wifiConfig)
{
    if (wifiConfig != NULL)
    {
        if (wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED)
        {
            *wifiConfig = wfx_rsi.sec;
            return true;
        }
    }
    return false;
}

/*********************************************************************
 * @fn  bool wfx_is_sta_provisioned(void)
 * @brief
 *      Driver is STA provisioned
 * @param[in]  None
 * @return  None
 ***********************************************************************/
bool wfx_is_sta_provisioned(void)
{
    bool status = (wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED) ? true : false;
    WFX_RSI_LOG("%s: status: SSID -> %s: %s", __func__, &wfx_rsi.sec.ssid[0], (status ? "provisioned" : "not provisioned"));
    return status;
}

/*********************************************************************
 * @fn  void wfx_clear_wifi_provision(void)
 * @brief
 *      Driver is clear the wifi provision
 * @param[in]  None
 * @return  None
 ***********************************************************************/
void wfx_clear_wifi_provision(void)
{
    memset(&wfx_rsi.sec, 0, sizeof(wfx_rsi.sec));
    wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_PROVISIONED;
    WFX_RSI_LOG("%s: completed.", __func__);
}

/*************************************************************************
 * @fn sl_status_t wfx_connect_to_ap(void)
 * @brief
 * Start a JOIN command to the AP - Done by the wfx_rsi task
 * @param[in]   None
 * @return  returns SL_STATUS_OK if successful,
 *         SL_STATUS_INVALID_CONFIGURATION otherwise
 ****************************************************************************/
sl_status_t wfx_connect_to_ap(void)
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED)
    {
        WFX_RSI_LOG("%s: connecting to access point -> SSID: %s, PSK:%s", __func__, &wfx_rsi.sec.ssid[0], &wfx_rsi.sec.passkey[0]);
        xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_START_JOIN);
    }
    else
    {
        WFX_RSI_LOG("%s: error: access point not provisioned", __func__);
        return SL_STATUS_INVALID_CONFIGURATION;
    }
    return SL_STATUS_OK;
}

/*********************************************************************
 * @fn  void wfx_setup_ip6_link_local(sl_wfx_interface_t whichif)
 * @brief
 *      Implement the ipv6 setup
 * @param[in]  whichif:
 * @return  None
 ***********************************************************************/
void wfx_setup_ip6_link_local(sl_wfx_interface_t whichif)
{
    /*
     * TODO: Implement IPV6 setup, currently in wfx_rsi_task()
     * This is hooked with MATTER code.
     */
    WFX_RSI_LOG("%s: warning: not implemented.", __func__);
}

/*********************************************************************
 * @fn  bool wfx_is_sta_connected(void)
 * @brief
 *      called fuction when driver is connected to STA
 * @param[in]  None
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_is_sta_connected(void)
{
    bool status;
    status = (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) ? true : false;
    WFX_RSI_LOG("%s: status: %s", __func__, (status ? "connected" : "not connected"));
    return status;
}

/*********************************************************************
 * @fn  wifi_mode_t wfx_get_wifi_mode()
 * @brief
 *      get the wifi mode
 * @param[in]  None
 * @return  return WIFI_MODE_NULL if successful,
 *          WIFI_MODE_STA otherwise
 ***********************************************************************/
wifi_mode_t wfx_get_wifi_mode()
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY)
        return WIFI_MODE_STA;
    return WIFI_MODE_NULL;
}

/*********************************************************************
 * @fn  sl_status_t wfx_sta_discon(void)
 * @brief
 *      called fuction when STA disconnected
 * @param[in]  None
 * @return  return SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t wfx_sta_discon(void)
{
    WFX_RSI_LOG("%s: started.", __func__);
    int32_t status;
    status = wfx_rsi_disconnect();
    wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTED;
    WFX_RSI_LOG("%s: completed.", __func__);
    return status;
}
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/*********************************************************************
 * @fn  bool wfx_have_ipv4_addr(sl_wfx_interface_t which_if)
 * @brief
 *      called fuction when driver have ipv4 address
 * @param[in]  which_if:
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_have_ipv4_addr(sl_wfx_interface_t which_if)
{
    bool status = false;
    if (which_if == SL_WFX_STA_INTERFACE)
    {
        status = (wfx_rsi.dev_state & WFX_RSI_ST_STA_DHCP_DONE) ? true : false;
    }
    else
    {
        status = false; /* TODO */
    }
    WFX_RSI_LOG("%s: status: %d", __func__, status);
    return status;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*********************************************************************
 * @fn  bool wfx_have_ipv6_addr(sl_wfx_interface_t which_if)
 * @brief
 *      called fuction when driver have ipv6 address
 * @param[in]  which_if:
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_have_ipv6_addr(sl_wfx_interface_t which_if)
{
    bool status = false;
    if (which_if == SL_WFX_STA_INTERFACE)
    {
        status = (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) ? true : false;
    }
    else
    {
        status = false; /* TODO */
    }
    WFX_RSI_LOG("%s: status: %d", __func__, status);
    return status;
}

/*********************************************************************
 * @fn  bool wfx_hw_ready(void)
 * @brief
 *      called fuction when driver ready
 * @param[in]  None
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_hw_ready(void)
{
    return (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY) ? true : false;
}

/*********************************************************************
 * @fn  int32_t wfx_get_ap_info(wfx_wifi_scan_result_t *ap)
 * @brief
 *      get the access point information
 * @param[in]  ap: access point
 * @return
 *      access point information
 ***********************************************************************/
int32_t wfx_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    return wfx_rsi_get_ap_info(ap);
}

/*********************************************************************
 * @fn   int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t *extra_info)
 * @brief
 *      get the access point extra information
 * @param[in]  extra_info:access point extra information
 * @return
 *      access point extra information
 ***********************************************************************/
int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    return wfx_rsi_get_ap_ext(extra_info);
}

/***************************************************************************
 * @fn   int32_t wfx_reset_counts(){
 * @brief
 *      get the driver reset count
 * @param[in]  None
 * @return
 *      reset count
 *****************************************************************************/
int32_t wfx_reset_counts()
{
    return wfx_rsi_reset_count();
}

#ifdef SL_WFX_CONFIG_SCAN
/*******************************************************************************
 * @fn   bool wfx_start_scan(char *ssid, void (*callback)(wfx_wifi_scan_result_t *))
 * @brief
 *       called fuction when driver start scaning
 * @param[in]  ssid:
 * @return returns ture if successful,
 *          false otherwise
 *******************************************************************************/
bool wfx_start_scan(char * ssid, void (*callback)(wfx_wifi_scan_result_t *))
{
    int sz;

    if (wfx_rsi.scan_cb)
        return false; /* Already in progress */
    if (ssid)
    {
        sz = strlen(ssid);
        if ((wfx_rsi.scan_ssid = (char *) pvPortMalloc(sz + 1)) == (char *) 0)
        {
            return false;
        }
        strcpy(wfx_rsi.scan_ssid, ssid);
    }
    wfx_rsi.scan_cb = callback;
    xEventGroupSetBits(wfx_rsi.events, WFX_EVT_SCAN);

    return true;
}

/***************************************************************************
 * @fn   void wfx_cancel_scan(void)
 * @brief
 *      called function when driver cancel scaning
 * @param[in]  None
 * @return
 *      None
 *****************************************************************************/
void wfx_cancel_scan(void)
{
    /* Not possible */
    WFX_RSI_LOG("%s: cannot cancel scan", __func__);
}
#endif /* SL_WFX_CONFIG_SCAN */
