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

#include "silabs_utils.h"
#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <platform/silabs/wifi/icd/WifiSleepManager.h>
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

using namespace chip;
using namespace chip::DeviceLayer;

#define CONVERT_SEC_TO_MS(TimeInS) (TimeInS * 1000)

// TODO: This is a workaround because we depend on the platform lib which depends on the platform implementation.
//       As such we can't depend on the platform here as well
extern void HandleWFXSystemEvent(sl_wfx_generic_message_t * eventData);

namespace {

constexpr uint8_t kWlanMinRetryIntervalsInSec = 1;
constexpr uint8_t kWlanMaxRetryIntervalsInSec = 60;
uint8_t retryInterval                         = kWlanMinRetryIntervalsInSec;
osTimerId_t sRetryTimer;

bool hasNotifiedIPV6 = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
bool hasNotifiedIPV4 = false;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*
 * Notifications to the upper-layer
 * All done in the context of the RSI/WiFi task (rsi_if.c)
 */
void RetryConnectionTimerHandler(void * arg)
{
    if (wfx_connect_to_ap() != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
    }
}

} // namespace

/* Updated functions */

void NotifyIPv6Change(bool gotIPv6Addr)
{
    hasNotifiedIPV6 = gotIPv6Addr;

    sl_wfx_generic_message_t eventData = {};
    eventData.header.id                = gotIPv6Addr ? to_underlying(WifiEvent::kGotIPv6) : to_underlying(WifiEvent::kLostIP);
    eventData.header.length            = sizeof(eventData.header);

    HandleWFXSystemEvent(&eventData);
}
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
void NotifyIPv4Change(bool gotIPv4Addr)
{
    hasNotifiedIPV4 = gotIPv4Addr;

    sl_wfx_generic_message_t eventData;

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = gotIPv4Addr ? to_underlying(WifiEvent::kGotIPv4) : to_underlying(WifiEvent::kLostIP);
    eventData.header.length = sizeof(eventData.header);
    HandleWFXSystemEvent(&eventData);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4

void NotifyDisconnection(WifiDisconnectionReasons reason)
{
    sl_wfx_disconnect_ind_t evt = {};
    evt.header.id               = to_underlying(WifiEvent::kDisconnect);
    evt.header.length           = sizeof evt;
    evt.body.reason             = to_underlying(reason);

    HandleWFXSystemEvent((sl_wfx_generic_message_t *) &evt);
}

void NotifyConnection(const MacAddress & ap)
{
    sl_wfx_connect_ind_t evt = {};
    evt.header.id            = to_underlying(WifiEvent::kConnect);
    evt.header.length        = sizeof evt;
#ifdef RS911X_WIFI
    evt.body.channel = wfx_rsi.ap_chan;
#endif
    std::copy(ap.begin(), ap.end(), evt.body.mac);

    HandleWFXSystemEvent((sl_wfx_generic_message_t *) &evt);
}

bool HasNotifiedIPv6Change()
{
    return hasNotifiedIPV6;
}

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
bool HasNotifiedIPv4Change()
{
    return hasNotifiedIPV4;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4

void ResetIPNotificationStates()
{
    hasNotifiedIPV6 = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
}
/* Function to update */

/***********************************************************************************
 * @fn  sl_matter_wifi_task_started(void)
 * @brief
 *       Wifi device started notification
 * @param[in]: None
 * @return None
 *************************************************************************************/
void sl_matter_wifi_task_started(void)
{
    sl_wfx_startup_ind_t evt = {};

    // Creating a timer which will be used to retry connection with AP
    sRetryTimer = osTimerNew(RetryConnectionTimerHandler, osTimerOnce, NULL, NULL);
    VerifyOrReturn(sRetryTimer != NULL);

    evt.header.id     = to_underlying(WifiEvent::kStartUp);
    evt.header.length = sizeof evt;
    evt.body.status   = 0;

    // TODO : Remove workwound when sl_wfx_startup_ind_t is unified
    //        Issue is same structure name but different contents
#if WF200_WIFI
    MutableByteSpan macSpan(evt.body.mac_addr[SL_WFX_STA_INTERFACE], kWifiMacAddressLength);
#else
    MutableByteSpan macSpan(evt.body.mac_addr, kWifiMacAddressLength);
#endif // WF200_WIFI

    GetMacAddress(SL_WFX_STA_INTERFACE, macSpan);

    HandleWFXSystemEvent((sl_wfx_generic_message_t *) &evt);
}

/**************************************************************************************
 * @fn  void wfx_retry_connection(uint16_t retryAttempt)
 * @brief
 *      During commissioning, we retry to join the network MAX_JOIN_RETRIES_COUNT times.
 *      If DUT is disconnected from the AP or device is power cycled, then retry connection
 *      with AP continously after a certain time interval.
 * @param[in]  retryAttempt
 * @return None
 *************************************************************************************/
void wfx_retry_connection(uint16_t retryAttempt)
{
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

#if CHIP_CONFIG_ENABLE_ICD_SERVER
        //  Remove High performance request before giving up due to a timer start error to save battery life
        Silabs::WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
        return;
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    Silabs::WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    ChipLogProgress(DeviceLayer, "wfx_retry_connection : Next attempt after %d Seconds", retryInterval);
    retryInterval += retryInterval;
}
