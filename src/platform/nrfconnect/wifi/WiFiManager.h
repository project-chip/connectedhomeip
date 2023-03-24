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
        app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum mSecurityType{};
        app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum mWiFiVersion{};
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
        uint32_t mBeaconsSuccessCount{};
        uint32_t mBeaconsLostCount{};
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
    static constexpr uint32_t kConnectionRecoveryMinIntervalMs     = CONFIG_CHIP_WIFI_CONNECTION_RECOVERY_MINIMUM_INTERVAL;
    static constexpr uint32_t kConnectionRecoveryMaxIntervalMs     = CONFIG_CHIP_WIFI_CONNECTION_RECOVERY_MAXIMUM_INTERVAL;
    static constexpr uint32_t kConnectionRecoveryJitterMs          = CONFIG_CHIP_WIFI_CONNECTION_RECOVERY_JITTER;
    static constexpr uint32_t kConnectionRecoveryMaxRetries        = CONFIG_CHIP_WIFI_CONNECTION_RECOVERY_MAX_RETRIES_NUMBER;

    static_assert(kConnectionRecoveryMinIntervalMs < kConnectionRecoveryMaxIntervalMs);
    static_assert(kConnectionRecoveryJitterMs <= kConnectionRecoveryMaxIntervalMs);

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
    void AbortConnectionRecovery();

private:
    using NetEventHandler = void (*)(Platform::UniquePtr<uint8_t>);

    struct ConnectionParams
    {
        wifi_connect_req_params mParams;
        int8_t mRssi{ std::numeric_limits<int8_t>::min() };
    };

    constexpr static uint32_t kWifiManagementEvents = NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE |
        NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT | NET_EVENT_WIFI_IFACE_STATUS;

    // Event handling
    static void WifiMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
    static void ScanResultHandler(Platform::UniquePtr<uint8_t> data);
    static void ScanDoneHandler(Platform::UniquePtr<uint8_t> data);
    static void ConnectHandler(Platform::UniquePtr<uint8_t> data);
    static void DisconnectHandler(Platform::UniquePtr<uint8_t> data);
    static void PostConnectivityStatusChange(ConnectivityChange changeType);
    static void SendRouterSolicitation(System::Layer * layer, void * param);

    // Connection Recovery feature
    // This feature allows re-scanning and re-connecting the connection to the known network after
    // a reboot or when a connection is lost. The following attempts will occur with increasing interval.
    // The connection recovery interval starts from kConnectionRecoveryMinIntervalMs and is doubled
    // with each occurrence until reaching kConnectionRecoveryMaxIntervalMs.
    // When the connection recovery interval reaches the maximum value the randomized kConnectionRecoveryJitterMs
    // from the range [-jitter, +jitter] is added to the value to avoid the periodicity.
    // To avoid frequent recovery attempts when the signal to an access point is poor quality
    // The connection recovery interval will be cleared after the defined delay in kConnectionRecoveryDelayToReset.
    static void Recover(System::Layer * layer, void * param);
    void ResetRecoveryTime();
    System::Clock::Milliseconds32 CalculateNextRecoveryTime();

    ConnectionParams mWiFiParams{};
    ConnectionHandling mHandling;
    wifi_iface_state mWiFiState;
    wifi_iface_state mCachedWiFiState;
    net_mgmt_event_callback mWiFiMgmtClbk{};
    ScanResultCallback mScanResultCallback{ nullptr };
    ScanDoneCallback mScanDoneCallback{ nullptr };
    WiFiNetwork mWantedNetwork{};
    bool mInternalScan{ false };
    uint8_t mRouterSolicitationCounter = 0;
    bool mSsidFound{ false };
    uint32_t mConnectionRecoveryCounter{ 0 };
    uint32_t mConnectionRecoveryTimeMs{ kConnectionRecoveryMinIntervalMs };
    bool mRecoveryTimerAborted{ false };

    static const Map<wifi_iface_state, StationStatus, 10> sStatusMap;
    static const Map<uint32_t, NetEventHandler, 4> sEventHandlerMap;
};

} // namespace DeviceLayer
} // namespace chip
