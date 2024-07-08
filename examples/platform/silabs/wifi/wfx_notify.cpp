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

#include "AppConfig.h"
#include "BaseApplication.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "silabs_utils.h"
#include "task.h"
#include "wfx_host_events.h"

#ifdef RS911X_WIFI
#include "wfx_rsi.h"
#endif

#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

static uint16_t retryInterval = WLAN_MIN_RETRY_TIMER_MS;
static osTimerId_t sRetryTimer;
/*
 * Notifications to the upper-layer
 * All done in the context of the RSI/WiFi task (rsi_if.c)
 */

static void RetryConnectionTimerHandler(void * arg)
{
#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
    // TODO: remove this once it is fixed in the wifi sdk SI91X-15845
    wfx_rsi_power_save(RSI_ACTIVE, HIGH_PERFORMANCE);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
    if (wfx_connect_to_ap() != SL_STATUS_OK) {
        ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
    }
}
/***********************************************************************************
 * @fn  wfx_started_notify()
 * @brief
 *       Wifi device started notification
 * @param[in]: None
 * @return None
 *************************************************************************************/
void wfx_started_notify()
{
    sl_wfx_startup_ind_t evt;
    sl_wfx_mac_address_t mac;

    // Creating a timer which will be used to retry connection with AP
    sRetryTimer = osTimerNew(RetryConnectionTimerHandler, osTimerOnce, NULL, NULL);
    if (sRetryTimer == NULL)
    {
        return;
    }

    ChipLogProgress(DeviceLayer,"wfx_started_notify: started.");

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

    ChipLogProgress(DeviceLayer,"wfx_connected_notify : started.");

    if (status != SUCCESS_STATUS)
    {
        ChipLogProgress(DeviceLayer,"wfx_connected_notify : error: failed status: %ld.", status);
        return;
    }

    ChipLogProgress(DeviceLayer,"wfx_connected_notify : connected.");

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

    ChipLogProgress(DeviceLayer,"wfx_disconnected_notify: started.");

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

    ChipLogProgress(DeviceLayer,"wfx_ipv6_notify: started.");

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

    ChipLogProgress(DeviceLayer,"wfx_ip_changed_notify: started.");

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
    if(BaseApplication::sAppDelegate.isCommissioningInProgress()) {
        if(retryAttempt < MAX_JOIN_RETRIES_COUNT) {
            ChipLogProgress(DeviceLayer,"wfx_retry_connection : Next attempt after %d Seconds", CONVERT_MS_TO_SEC(WLAN_RETRY_TIMER_MS));
            if (osTimerStart(sRetryTimer, pdMS_TO_TICKS(WLAN_RETRY_TIMER_MS)) != osOK) {
                ChipLogProgress(DeviceLayer,"Failed to start retry timer");
                return;
            }
        } else {
            ChipLogProgress(DeviceLayer,"Connect failed after max %d tries", retryAttempt);
        }
    } else {
        /* After disconnection or power cycle the DUT
         * At the telescopic time interval device try to reconnect with AP, upto WLAN_MAX_RETRY_TIMER_MS intervals
         * are telescopic. If interval exceed WLAN_MAX_RETRY_TIMER_MS then it will try to reconnect at
         * WLAN_MAX_RETRY_TIMER_MS intervals.
         */
        if (retryInterval > WLAN_MAX_RETRY_TIMER_MS)
        {
            retryInterval = WLAN_MAX_RETRY_TIMER_MS;
        }
        if (osTimerStart(sRetryTimer, pdMS_TO_TICKS(retryInterval)) != osOK) {
            ChipLogProgress(DeviceLayer,"Failed to start retry timer");
            return;
        }
#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
        wfx_rsi_power_save(RSI_SLEEP_MODE_8, STANDBY_POWER_SAVE_WITH_RAM_RETENTION);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
        ChipLogProgress(DeviceLayer,"wfx_retry_connection : Next attempt after %d Seconds", CONVERT_MS_TO_SEC(retryInterval));
        retryInterval += retryInterval;
        return ;
    }
}
