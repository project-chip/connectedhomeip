/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <system/SystemPacketBuffer.h>
#include <wifipaf/WiFiPAFLayer.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/version.h>

namespace chip {
namespace DeviceLayer {

class ConnectivityManagerImplWiFiPAF
{
    friend class ConnectivityManager;

protected:
    CHIP_ERROR InitWiFiPAF();
    void OnWiFiPAFPlatformEvent(const ChipDeviceEvent * event);

private:
    // Configuration / advertising control (required by ConnectivityManager interface)
    void _WiFiPAFSetParam(const ConnectivityManager::WiFiPAFAdvertiseParam & pafAdvParam);
    CHIP_ERROR _SetWiFiPAFAdvertisingEnabled(bool enabled, uint32_t & publishId);

    // Publisher methods
    CHIP_ERROR _WiFiPAFPublish(ConnectivityManager::WiFiPAFAdvertiseParam & args);
    CHIP_ERROR _WiFiPAFCancelPublish(uint32_t PublishId);

    // Subscriber methods
    CHIP_ERROR _WiFiPAFSubscribe(const uint16_t & connDiscriminator, void * appState,
                                 ConnectivityManager::OnConnectionCompleteFunct onSuccess,
                                 ConnectivityManager::OnConnectionErrorFunct onError);
    CHIP_ERROR _WiFiPAFCancelSubscribe(uint32_t SubscribeId);
    CHIP_ERROR _WiFiPAFCancelIncompleteSubscribe();

    // Communication methods
    CHIP_ERROR _WiFiPAFSend(const WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msgBuf);
    CHIP_ERROR _WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole role);

    // Configuration
    void _WiFiPafSetApFreq(const uint16_t freq) { mApFreq = freq; }
    bool _WiFiPAFResourceAvailable() { return mPublishActive && mPafChannelAvailable; }

    // Helper to get the instance
    static ConnectivityManagerImplWiFiPAF & GetInstance();

    // NAN event callbacks
    void OnNanReplied(const wifi_nan_replied_event * event);
    void OnNanReceive(const wifi_nan_receive_event * event);
    void OnNanPublishTerminated(const wifi_nan_terminated_event * event);

    // Static event handler for Zephyr net_mgmt (NAN events)
#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(4, 2, 0)
    static void NanMgmtEventHandler(net_mgmt_event_callback * cb, uint64_t mgmtEvent, net_if * iface);
#else
    static void NanMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
#endif

    // Static event handler for WiFi STA connect/disconnect events
#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(4, 2, 0)
    static void WifiStaMgmtEventHandler(net_mgmt_event_callback * cb, uint64_t mgmtEvent, net_if * iface);
#else
    static void WifiStaMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
#endif

    // NAN event mask (Publisher-only events)
#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(4, 2, 0)
    static constexpr uint64_t kNanManagementEvents =
        NET_EVENT_WIFI_NAN_REPLIED | NET_EVENT_WIFI_NAN_RECEIVE | NET_EVENT_WIFI_NAN_PUBLISH_TERMINATED;
    // WiFi STA event mask (track AP connection state for PAF channel availability)
    static constexpr uint64_t kWifiStaManagementEvents = NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT;
#else
    static constexpr uint32_t kNanManagementEvents =
        NET_EVENT_WIFI_NAN_REPLIED | NET_EVENT_WIFI_NAN_RECEIVE | NET_EVENT_WIFI_NAN_PUBLISH_TERMINATED;
    // WiFi STA event mask (track AP connection state for PAF channel availability)
    static constexpr uint32_t kWifiStaManagementEvents = NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT;
#endif

    // Member variables
    ConnectivityManager::WiFiPAFAdvertiseParam mPafAdvParam;
    net_mgmt_event_callback mNanMgmtClbk{};
    net_mgmt_event_callback mWifiStaMgmtClbk{};
    struct net_if * mWiFiIface = nullptr;
    uint16_t mApFreq           = 0;
    uint32_t mCurrentPublishId = 0;
    bool mPublishActive        = false;
    bool mPeerRegistered       = false;
    bool mPafChannelAvailable  = true; ///< false while station is connected to an AP
    struct k_mutex mNanMutex
    {
    };
};

} // namespace DeviceLayer
} // namespace chip
