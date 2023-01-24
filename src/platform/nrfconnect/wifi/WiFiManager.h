/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides the wrapper for nRF wpa_supplicant API
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/NetworkCommissioning.h>
#include <system/SystemLayer.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

extern "C" {
#include <src/utils/common.h>
#include <wpa_supplicant/wpa_supplicant_i.h>
}

struct net_if;
struct wpa_ssid;
using WpaNetwork = struct wpa_ssid;

namespace chip {
namespace DeviceLayer {

// emulation of dictionary - might be moved to utils
template <typename T1, typename T2, std::size_t N>
class Map
{
    struct Pair
    {
        T1 key;
        T2 value;
    };

public:
    Map(const Pair (&list)[N])
    {
        int idx{ 0 };
        for (const auto & pair : list)
        {
            mMap[idx++] = pair;
        }
    }

    T2 operator[](const T1 & key) const
    {
        for (const auto & it : mMap)
        {
            if (key == it.key)
                return it.value;
        }

        return T2{};
    }

    Map()            = delete;
    Map(const Map &) = delete;
    Map(Map &&)      = delete;
    Map & operator=(const Map &) = delete;
    Map & operator=(Map &&) = delete;
    ~Map()                  = default;

private:
    Pair mMap[N];
};

class WiFiManager
{
public:
    enum WiFiRequestStatus : int
    {
        SUCCESS    = 0,
        FAILURE    = 1,
        TERMINATED = 2
    };

    using ScanResultCallback = void (*)(const NetworkCommissioning::WiFiScanResponse &);
    using ScanDoneCallback   = void (*)(WiFiRequestStatus);
    using ConnectionCallback = void (*)();

    enum class StationStatus : uint8_t
    {
        NONE,
        DISCONNECTED,
        DISABLED,
        SCANNING,
        CONNECTING,
        CONNECTED,
        PROVISIONING,
        FULLY_PROVISIONED,
        UNKNOWN
    };

    static WiFiManager & Instance()
    {
        static WiFiManager sInstance;
        return sInstance;
    }

    struct ConnectionHandling
    {
        ConnectionCallback mOnConnectionSuccess{};
        ConnectionCallback mOnConnectionFailed{};
        System::Clock::Seconds32 mConnectionTimeout{};
    };

    struct WiFiInfo
    {
        ByteSpan mBssId{};
        uint8_t mSecurityType{};
        uint8_t mWiFiVersion{};
        uint16_t mChannel{};
        int8_t mRssi{};
        uint8_t mSsid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        size_t mSsidLen{ 0 };
    };

    struct NetworkStatistics
    {
        uint32_t mPacketMulticastRxCount{};
        uint32_t mPacketMulticastTxCount{};
        uint32_t mPacketUnicastRxCount{};
        uint32_t mPacketUnicastTxCount{};
        uint32_t mOverruns{};
    };

    struct WiFiNetwork
    {
        uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        size_t ssidLen = 0;
        uint8_t pass[DeviceLayer::Internal::kMaxWiFiKeyLength];
        size_t passLen = 0;

        bool IsConfigured() const { return ssidLen > 0; }
        ByteSpan GetSsidSpan() const { return ByteSpan(ssid, ssidLen); }
        ByteSpan GetPassSpan() const { return ByteSpan(pass, passLen); }
        void Clear() { ssidLen = 0; }
        void Erase()
        {
            memset(ssid, 0, DeviceLayer::Internal::kMaxWiFiSSIDLength);
            memset(pass, 0, DeviceLayer::Internal::kMaxWiFiKeyLength);
            ssidLen = 0;
            passLen = 0;
        }
    };

    static constexpr uint16_t kRouterSolicitationIntervalMs        = 4000;
    static constexpr uint16_t kMaxInitialRouterSolicitationDelayMs = 1000;
    static constexpr uint8_t kRouterSolicitationMaxCount           = 3;

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    static constexpr uint8_t kDefaultDTIMInterval = 3;
    static constexpr uint8_t kBeaconIntervalMs    = 100;
#endif

    CHIP_ERROR Init();
    CHIP_ERROR Scan(const ByteSpan & ssid, ScanResultCallback resultCallback, ScanDoneCallback doneCallback,
                    bool internalScan = false);
    CHIP_ERROR Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling);
    StationStatus GetStationStatus() const;
    CHIP_ERROR ClearStationProvisioningData();
    CHIP_ERROR Disconnect();
    CHIP_ERROR GetWiFiInfo(WiFiInfo & info) const;
    CHIP_ERROR GetNetworkStatistics(NetworkStatistics & stats) const;

private:
    using NetEventHandler = void (*)(uint8_t *);

    struct ConnectionParams
    {
        wifi_connect_req_params mParams;
        int8_t mRssi{ std::numeric_limits<int8_t>::min() };
    };

    constexpr static uint32_t kWifiManagementEvents = NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE |
        NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT | NET_EVENT_WIFI_IFACE_STATUS;

    // Event handling
    static void WifiMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
    static void ScanResultHandler(uint8_t * data);
    static void ScanDoneHandler(uint8_t * data);
    static void ConnectHandler(uint8_t * data);
    static void DisconnectHandler(uint8_t * data);
    static void PostConnectivityStatusChange(ConnectivityChange changeType);
    static void SendRouterSolicitation(System::Layer * layer, void * param);

    ConnectionParams mWiFiParams{};
    ConnectionHandling mHandling;
    wifi_iface_state mWiFiState;
    net_mgmt_event_callback mWiFiMgmtClbk{};
    ScanResultCallback mScanResultCallback{ nullptr };
    ScanDoneCallback mScanDoneCallback{ nullptr };
    WiFiNetwork mWantedNetwork{};
    bool mInternalScan{ false };
    uint8_t mRouterSolicitationCounter = 0;
    static const Map<wifi_iface_state, StationStatus, 10> sStatusMap;
    static const Map<uint32_t, NetEventHandler, 4> sEventHandlerMap;
};

} // namespace DeviceLayer
} // namespace chip
