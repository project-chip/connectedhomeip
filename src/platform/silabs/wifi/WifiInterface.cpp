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

// TODO: We shouldn't need to have access to a global variable in the interface here
extern WfxRsi_t wfx_rsi;

// TODO: This is a workaround because we depend on the platform lib which depends on the platform implementation.
//       As such we can't depend on the platform here as well
extern void HandleWFXSystemEvent(sl_wfx_generic_message_t * eventData);

namespace {

constexpr uint8_t kWlanMinRetryIntervalsInSec = 1;
constexpr uint8_t kWlanMaxRetryIntervalsInSec = 60;
uint8_t retryInterval                         = kWlanMinRetryIntervalsInSec;

/**
 * @brief Retry timer callback that triggers a reconnection attempt
 *
 * TODO: The structure of the retry needs to be redone
 *
 * @param arg
 */
void RetryConnectionTimerHandler(void * arg)
{
    if (chip::DeviceLayer::Silabs::WifiInterface::GetInstance().ConnectToAccessPoint() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ConnectToAccessPoint() failed.");
    }
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {

void WifiInterface::NotifyIPv6Change(bool gotIPv6Addr)
{
    mHasNotifiedIPv6 = gotIPv6Addr;

    sl_wfx_generic_message_t eventData = {};
    eventData.header.id                = gotIPv6Addr ? to_underlying(WifiEvent::kGotIPv6) : to_underlying(WifiEvent::kLostIP);
    eventData.header.length            = sizeof(eventData.header);

    HandleWFXSystemEvent(&eventData);
}

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
void WifiInterface::NotifyIPv4Change(bool gotIPv4Addr)
{
    mHasNotifiedIPv4 = gotIPv4Addr;

    sl_wfx_generic_message_t eventData;

    memset(&eventData, 0, sizeof(eventData));
    eventData.header.id     = gotIPv4Addr ? to_underlying(WifiEvent::kGotIPv4) : to_underlying(WifiEvent::kLostIP);
    eventData.header.length = sizeof(eventData.header);
    HandleWFXSystemEvent(&eventData);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4

void WifiInterface::NotifyDisconnection(WifiDisconnectionReasons reason)
{
    sl_wfx_disconnect_ind_t evt = {};
    evt.header.id               = to_underlying(WifiEvent::kDisconnect);
    evt.header.length           = sizeof evt;
    evt.body.reason             = to_underlying(reason);

    HandleWFXSystemEvent((sl_wfx_generic_message_t *) &evt);
}

void WifiInterface::NotifyConnection(const MacAddress & ap)
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

void WifiInterface::ResetIPNotificationStates()
{
    mHasNotifiedIPv6 = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    mHasNotifiedIPv4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
}

void WifiInterface::NotifyWifiTaskInitialized(void)
{
    sl_wfx_startup_ind_t evt = { 0 };

    // TODO: We should move this to the init function and not the notification function
    // Creating a timer which will be used to retry connection with AP
    mRetryTimer = osTimerNew(RetryConnectionTimerHandler, osTimerOnce, NULL, NULL);
    VerifyOrReturn(mRetryTimer != NULL);

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

// TODO: The retry stategy needs to be re-worked
void WifiInterface::ScheduleConnectionAttempt()
{
    if (retryInterval > kWlanMaxRetryIntervalsInSec)
    {
        retryInterval = kWlanMaxRetryIntervalsInSec;
    }

    if (osTimerStart(mRetryTimer, pdMS_TO_TICKS(retryInterval * 1000)) != osOK)
    {
        ChipLogProgress(DeviceLayer, "Failed to start retry timer");
        // Sending the join command if retry timer failed to start
        if (ConnectToAccessPoint() != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ConnectToAccessPoint() failed.");
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

    ChipLogProgress(DeviceLayer, "ScheduleConnectionAttempt : Next attempt after %d Seconds", retryInterval);
    retryInterval += retryInterval;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
