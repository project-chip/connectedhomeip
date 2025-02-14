/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *          Provides the wrapper for Telink wpa_supplicant API
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/NetworkCommissioning.h>
#include <system/SystemLayer.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

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

    Map()                        = delete;
    Map(const Map &)             = delete;
    Map(Map &&)                  = delete;
    Map & operator=(const Map &) = delete;
    Map & operator=(Map &&)      = delete;
    ~Map()                       = default;

private:
    Pair mMap[N];
};

class WiFiManager
{
public:
    /* No copy, nor move. */
    WiFiManager(const WiFiManager &)             = delete;
    WiFiManager & operator=(const WiFiManager &) = delete;
    WiFiManager(WiFiManager &&)                  = delete;
    WiFiManager & operator=(WiFiManager &&)      = delete;

    using ScanDoneStatus     = decltype(wifi_status::status);
    using ScanResultCallback = void (*)(const NetworkCommissioning::WiFiScanResponse &);
    using ScanDoneCallback   = void (*)(const ScanDoneStatus &);
    using ConnectionCallback = void (*)(const wifi_conn_status &);

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

    enum class WlanReason : uint8_t
    {
        UNSPECIFIED = 1,
        PREV_AUTH_NOT_VALID,
        DEAUTH_LEAVING,
        DISASSOC_DUE_TO_INACTIVITY,
        DISASSOC_AP_BUSY
    };

    static WiFiManager & Instance()
    {
        static WiFiManager sInstance;
        return sInstance;
    }

    struct ConnectionHandling
    {
        ConnectionCallback mOnConnectionDone{};
        System::Clock::Seconds32 mConnectionTimeout{};
    };

    struct WiFiInfo
    {
        uint8_t mBssId[DeviceLayer::Internal::kWiFiBSSIDLength];
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

    CHIP_ERROR Init();
    CHIP_ERROR Scan(const ByteSpan & ssid, ScanResultCallback resultCallback, ScanDoneCallback doneCallback,
                    bool internalScan = false);
    CHIP_ERROR Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling);
    StationStatus GetStationStatus() const;
    CHIP_ERROR ClearStationProvisioningData();
    CHIP_ERROR Disconnect();
    CHIP_ERROR GetWiFiInfo(WiFiInfo & info) const;
    const WiFiNetwork & GetWantedNetwork() const { return mWantedNetwork; }
    CHIP_ERROR GetNetworkStatistics(NetworkStatistics & stats) const;
    void AbortConnectionRecovery();
    CHIP_ERROR SetLowPowerMode(bool onoff);
    void SetLastDisconnectReason(uint16_t reason);
    uint16_t GetLastDisconnectReason();

private:
    using NetEventHandler = void (*)(Platform::UniquePtr<uint8_t>, size_t);

    WiFiManager()  = default;
    ~WiFiManager() = default;

    struct ConnectionParams
    {
        wifi_connect_req_params mParams;
        int8_t mRssi{ std::numeric_limits<int8_t>::min() };
    };

    constexpr static uint32_t kWifiManagementEvents = NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE |
        NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT | NET_EVENT_WIFI_IFACE_STATUS;

    constexpr static uint32_t kIPv6ManagementEvents = NET_EVENT_IPV6_ADDR_ADD | NET_EVENT_IPV6_ADDR_DEL;

    // Event handling
    static void WifiMgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
    static void IPv6MgmtEventHandler(net_mgmt_event_callback * cb, uint32_t mgmtEvent, net_if * iface);
    static void ScanResultHandler(Platform::UniquePtr<uint8_t> data, size_t length);
    static void ScanDoneHandler(Platform::UniquePtr<uint8_t> data, size_t length);
    static void ConnectHandler(Platform::UniquePtr<uint8_t> data, size_t length);
    static void DisconnectHandler(Platform::UniquePtr<uint8_t> data, size_t length);
    static void PostConnectivityStatusChange(ConnectivityChange changeType);
    static void SendRouterSolicitation(System::Layer * layer, void * param);
    static void IPv6AddressChangeHandler(const void * data);

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

    net_if * mNetIf{ nullptr };
    ConnectionParams mWiFiParams{};
    ConnectionHandling mHandling{};
    wifi_scan_params mScanParams{};
    char mScanSsidBuffer[DeviceLayer::Internal::kMaxWiFiSSIDLength + 1] = { 0 };
    wifi_iface_state mWiFiState;
    wifi_iface_state mCachedWiFiState;
    net_mgmt_event_callback mWiFiMgmtClbk{};
    net_mgmt_event_callback mIPv6MgmtClbk{};
    ScanResultCallback mScanResultCallback{ nullptr };
    ScanDoneCallback mScanDoneCallback{ nullptr };
    WiFiNetwork mWantedNetwork{};
    bool mInternalScan{ false };
    uint8_t mRouterSolicitationCounter = 0;
    bool mSsidFound{ false };
    uint32_t mConnectionRecoveryCounter{ 0 };
    uint32_t mConnectionRecoveryTimeMs{ kConnectionRecoveryMinIntervalMs };
    bool mApplicationDisconnectRequested{ false };
    uint16_t mLastDisconnectedReason = static_cast<uint16_t>(WlanReason::UNSPECIFIED);

    static const Map<wifi_iface_state, StationStatus, 10> sStatusMap;
    static const Map<uint32_t, NetEventHandler, 5> sEventHandlerMap;
};

} // namespace DeviceLayer
} // namespace chip
