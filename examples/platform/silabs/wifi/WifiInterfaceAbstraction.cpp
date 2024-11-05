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

// SL MATTER WI-FI INTERFACE

#include "WifiInterfaceAbstraction.h"
#include "silabs_utils.h"
#include "wfx_host_events.h"
#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

constexpr uint8_t kWlanMinRetryIntervalsInSec = 1;
constexpr uint8_t kWlanMaxRetryIntervalsInSec = 60;
constexpr uint8_t kWlanRetryIntervalInSec     = 5;
uint8_t retryInterval                         = kWlanMinRetryIntervalsInSec;
osTimerId_t sRetryTimer;

// TODO: Remove this when the full abstraction is implemented
#ifdef RS911X_WIFI
// Thread for the WLAN RSI
osThreadId_t sWlanThread;
constexpr uint32_t kWlanTaskSize = 2048;
uint8_t wlanStack[kWlanTaskSize];
osThread_t sWlanTaskControlBlock;
constexpr osThreadAttr_t kWlanTaskAttr = { .name       = "wlan_rsi",
                                           .attr_bits  = osThreadDetached,
                                           .cb_mem     = &sWlanTaskControlBlock,
                                           .cb_size    = osThreadCbSize,
                                           .stack_mem  = wlanStack,
                                           .stack_size = kWlanTaskSize,
                                           .priority   = osPriorityAboveNormal7 };
#endif // RS911X_WIFI

/*
 * Notifications to the upper-layer
 * All done in the context of the RSI/WiFi task (rsi_if.c)
 */
void RetryConnectionTimerHandler(void * arg)
{
#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
    wfx_rsi_power_save(RSI_ACTIVE, HIGH_PERFORMANCE);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
    if (wfx_connect_to_ap() != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
    }
}

} // namespace

// TODO: Remove this when the full abstraction is implemented
#ifdef RS911X_WIFI
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
    VerifyOrReturnError(!(wfx_rsi.dev_state & WFX_RSI_ST_STARTED), SL_STATUS_OK);
    wfx_rsi.dev_state |= WFX_RSI_ST_STARTED;

    // Creating a Wi-Fi driver thread
    sWlanThread = osThreadNew(sl_matter_wifi_task, NULL, &kWlanTaskAttr);

    VerifyOrReturnError(sWlanThread != NULL, SL_STATUS_FAIL);

    ChipLogProgress(DeviceLayer, "sl_matter_wifi_task created successfully");
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
    return mode;
}

/*********************************************************************
 * @fn  void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t *addr)
 * @brief
 *      get the wifi mac address
 * @param[in]  Interface:
 * @param[in]  addr : address
 * @return
 *       None
 ***********************************************************************/
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t * addr)
{
    VerifyOrReturn(addr != nullptr);
#ifdef SL_WFX_CONFIG_SOFTAP
    *addr = (interface == SL_WFX_SOFTAP_INTERFACE) ? wfx_rsi.softap_mac : wfx_rsi.sta_mac;
#else
    *addr = wfx_rsi.sta_mac;
#endif
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
    VerifyOrReturn(cfg != nullptr);
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
    VerifyOrReturnError(wifiConfig != nullptr, false);
    VerifyOrReturnError(wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED, false);
    *wifiConfig = wfx_rsi.sec;
    return true;
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
    ChipLogProgress(DeviceLayer, "Clear WiFi Provision");
}

/*************************************************************************
 * @fn sl_status_t wfx_connect_to_ap(void)
 * @brief
 * Start a JOIN command to the AP - Done by the wfx_rsi task
 * @param[in]   None
 * @return  returns SL_STATUS_OK if successful
 ****************************************************************************/
sl_status_t wfx_connect_to_ap(void)
{
    VerifyOrReturnError(wfx_rsi.dev_state & WFX_RSI_ST_STA_PROVISIONED, SL_STATUS_INVALID_CONFIGURATION);
    VerifyOrReturnError(wfx_rsi.sec.ssid_length, SL_STATUS_INVALID_CREDENTIALS);
    VerifyOrReturnError(wfx_rsi.sec.ssid_length <= WFX_MAX_SSID_LENGTH, SL_STATUS_HAS_OVERFLOWED);
    ChipLogProgress(DeviceLayer, "connect to access point: %s", wfx_rsi.sec.ssid);
    WfxEvent_t event;
    event.eventType = WFX_EVT_STA_START_JOIN;
    sl_matter_wifi_post_event(&event);
    return SL_STATUS_OK;
}

#if SL_ICD_ENABLED
#if SLI_SI917
/*********************************************************************
 * @fn  sl_status_t wfx_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t
 sl_si91x_wifi_state)
 * @brief
 *      Implements the power save in sleepy application
 * @param[in]  sl_si91x_ble_state : State to set for the BLE
               sl_si91x_wifi_state : State to set for the WiFi
 * @return  SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t wfx_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state)
{
    return (wfx_rsi_power_save(sl_si91x_ble_state, sl_si91x_wifi_state) ? SL_STATUS_FAIL : SL_STATUS_OK);
}
#else  // For RS9116
/*********************************************************************
 * @fn  sl_status_t wfx_power_save(void)
 * @brief
 *      Implements the power save in sleepy application
 * @param[in]  None
 * @return  SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t wfx_power_save(void)
{
    return (wfx_rsi_power_save() ? SL_STATUS_FAIL : SL_STATUS_OK);
}
#endif /* SLI_SI917 */
#endif /* SL_ICD_ENABLED */

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
     * TODO: Implement IPV6 setup, currently in sl_matter_wifi_task()
     * This is hooked with MATTER code.
     */
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
    bool status = (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) > 0;
    return status;
}

/*********************************************************************
 * @fn  wifi_mode_t wfx_get_wifi_mode(void)
 * @brief
 *      get the wifi mode
 * @param[in]  None
 * @return  return WIFI_MODE_NULL if successful,
 *          WIFI_MODE_STA otherwise
 ***********************************************************************/
wifi_mode_t wfx_get_wifi_mode(void)
{
    if (wfx_rsi.dev_state & WFX_RSI_ST_DEV_READY)
        return WIFI_MODE_STA;
    return WIFI_MODE_NULL;
}

/*********************************************************************
 * @fn  sl_status_t sl_matter_wifi_disconnect(void)
 * @brief
 *      called fuction when STA disconnected
 * @param[in]  None
 * @return  return SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t sl_matter_wifi_disconnect(void)
{
    sl_status_t status;
    status = sl_wifi_platform_disconnect();
    wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTED;
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
    VerifyOrReturnError(which_if == SL_WFX_STA_INTERFACE, false);
    return ((wfx_rsi.dev_state & WFX_RSI_ST_STA_DHCP_DONE) > 0);
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
    VerifyOrReturnError(which_if == SL_WFX_STA_INTERFACE, false);
    // TODO: WFX_RSI_ST_STA_CONNECTED does not guarantee SLAAC IPv6 LLA, maybe use a different FLAG
    return ((wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED) > 0);
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
 * @fn   int32_t wfx_reset_counts(void)
 * @brief
 *      get the driver reset count
 * @param[in]  None
 * @return
 *      reset count
 *****************************************************************************/
int32_t wfx_reset_counts(void)
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
    // check if already in progress
    VerifyOrReturnError(wfx_rsi.scan_cb != nullptr, false);
    wfx_rsi.scan_cb = callback;

    VerifyOrReturnError(ssid != nullptr, false);
    wfx_rsi.scan_ssid_length = strnlen(ssid, std::min<size_t>(sizeof(ssid), WFX_MAX_SSID_LENGTH));
    wfx_rsi.scan_ssid        = reinterpret_cast<char *>(chip::Platform::MemoryAlloc(wfx_rsi.scan_ssid_length));
    VerifyOrReturnError(wfx_rsi.scan_ssid != nullptr, false);
    chip::Platform::CopyString(wfx_rsi.scan_ssid, wfx_rsi.scan_ssid_length, ssid);

    WfxEvent_t event;
    event.eventType = WFX_EVT_SCAN;
    sl_matter_wifi_post_event(&event);

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
    ChipLogError(DeviceLayer, "cannot cancel scan");
}
#endif /* SL_WFX_CONFIG_SCAN */
#endif // RS911X_WIFI

/***********************************************************************************
 * @fn  sl_matter_wifi_task_started(void)
 * @brief
 *       Wifi device started notification
 * @param[in]: None
 * @return None
 *************************************************************************************/
void sl_matter_wifi_task_started(void)
{
    sl_wfx_startup_ind_t evt;
    sl_wfx_mac_address_t mac;

    // Creating a timer which will be used to retry connection with AP
    sRetryTimer = osTimerNew(RetryConnectionTimerHandler, osTimerOnce, NULL, NULL);
    VerifyOrReturn(sRetryTimer != NULL);

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_STARTUP_IND_ID;
    evt.header.length = sizeof evt;
    evt.body.status   = 0;
    wfx_get_wifi_mac_addr(SL_WFX_STA_INTERFACE, &mac);
    memcpy(&evt.body.mac_addr[0], &mac.octet[0], MAC_ADDRESS_FIRST_OCTET);

    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/***********************************************************************************
 * @fn  void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t *ap)
 * @brief
 * For now we are not notifying anything other than AP Mac -
 * Other stuff such as DTIM etc. may be required for later
 * @param[in] status:
 * @param[in] ap: access point
 * @return None
 *************************************************************************************/
void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t * ap)
{
    sl_wfx_connect_ind_t evt;

    VerifyOrReturn(status == SUCCESS_STATUS);

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_CONNECT_IND_ID;
    evt.header.length = sizeof evt;

#ifdef RS911X_WIFI
    evt.body.channel = wfx_rsi.ap_chan;
#endif
    memcpy(&evt.body.mac[0], &ap->octet[0], MAC_ADDRESS_FIRST_OCTET);

    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/**************************************************************************************
 * @fn  void wfx_disconnected_notify(int32_t status)
 * @brief
 *    notification of disconnection
 * @param[in] status:
 * @return None
 ********************************************************************************************/
void wfx_disconnected_notify(int32_t status)
{
    sl_wfx_disconnect_ind_t evt;

    memset(&evt, 0, sizeof(evt));
    evt.header.id     = SL_WFX_DISCONNECT_IND_ID;
    evt.header.length = sizeof evt;
    evt.body.reason   = status;
    PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, (sl_wfx_generic_message_t *) &evt);
}

/**************************************************************************************
 * @fn  void wfx_ipv6_notify(int got_ip)
 * @brief
 *      notification of ipv6
 * @param[in]  got_ip:
 * @return None
 ********************************************************************************************/
void wfx_ipv6_notify(int got_ip)
{
    sl_wfx_generic_message_t eventData;

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = got_ip ? IP_EVENT_GOT_IP6 : IP_EVENT_STA_LOST_IP;
    eventData.header.length = sizeof(eventData.header);
    PlatformMgrImpl().HandleWFXSystemEvent(IP_EVENT, &eventData);
}

/**************************************************************************************
 * @fn   void wfx_ip_changed_notify(int got_ip)
 * @brief
 *      notification of ip change
 * @param[in]  got_ip:
 * @return None
 ********************************************************************************************/
void wfx_ip_changed_notify(int got_ip)
{
    sl_wfx_generic_message_t eventData;

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = got_ip ? IP_EVENT_STA_GOT_IP : IP_EVENT_STA_LOST_IP;
    eventData.header.length = sizeof(eventData.header);
    PlatformMgrImpl().HandleWFXSystemEvent(IP_EVENT, &eventData);
}

/**************************************************************************************
 * @fn  void wfx_retry_connection(uint16_t retryAttempt)
 * @brief
 *      During commissioning, we retry to join the network MAX_JOIN_RETRIES_COUNT times.
 *      If DUT is disconnected from the AP or device is power cycled, then retry connection
 *      with AP continously after a certain time interval.
 * @param[in]  retryAttempt
 * @return None
 ********************************************************************************************/
void wfx_retry_connection(uint16_t retryAttempt)
{
    // During commissioning, we retry to join the network MAX_JOIN_RETRIES_COUNT
    if (/*BaseApplication::sAppDelegate.isCommissioningInProgress()*/ true)
    {
        if (retryAttempt < MAX_JOIN_RETRIES_COUNT)
        {
            ChipLogProgress(DeviceLayer, "wfx_retry_connection : Next attempt after %d Seconds", kWlanRetryIntervalInSec);
            if (osTimerStart(sRetryTimer, pdMS_TO_TICKS(CONVERT_SEC_TO_MS(kWlanRetryIntervalInSec))) != osOK)
            {
                ChipLogProgress(DeviceLayer, "Failed to start retry timer");
                // Sending the join command if retry timer failed to start
                if (wfx_connect_to_ap() != SL_STATUS_OK)
                {
                    ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
                }
                return;
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "Connect failed after max %d tries", retryAttempt);
        }
    }
    else
    {
        /* After disconnection or power cycle the DUT
         * At the telescopic time interval device try to reconnect with AP, upto WLAN_MAX_RETRY_TIMER_MS intervals
         * are telescopic. If interval exceed WLAN_MAX_RETRY_TIMER_MS then it will try to reconnect at
         * WLAN_MAX_RETRY_TIMER_MS intervals.
         */
        if (retryInterval > kWlanMaxRetryIntervalsInSec)
        {
            retryInterval = kWlanMaxRetryIntervalsInSec;
        }
        if (osTimerStart(sRetryTimer, pdMS_TO_TICKS(CONVERT_SEC_TO_MS(retryInterval))) != osOK)
        {
            ChipLogProgress(DeviceLayer, "Failed to start retry timer");
            // Sending the join command if retry timer failed to start
            if (wfx_connect_to_ap() != SL_STATUS_OK)
            {
                ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
            }
            return;
        }
#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
        wfx_rsi_power_save(RSI_SLEEP_MODE_8, STANDBY_POWER_SAVE_WITH_RAM_RETENTION);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
        ChipLogProgress(DeviceLayer, "wfx_retry_connection : Next attempt after %d Seconds", retryInterval);
        retryInterval += retryInterval;
        return;
    }
}
