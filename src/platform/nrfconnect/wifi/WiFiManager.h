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

#include <net/net_if.h>
#include <net/wifi_mgmt.h>

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
        FAIL    = -1,
        SUCCESS = 0
    };

    using ScanResultCallback = void (*)(NetworkCommissioning::WiFiScanResponse *);
    using ScanDoneCallback   = void (*)(WiFiRequestStatus);

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

    using ConnectionCallback = void (*)();

    struct ConnectionHandling
    {
        ConnectionCallback mOnConnectionSuccess{};
        ConnectionCallback mOnConnectionFailed{};
        System::Clock::Timeout mConnectionTimeoutMs{};
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

    CHIP_ERROR Init();
    CHIP_ERROR Scan(const ByteSpan & ssid, ScanResultCallback resultCallback, ScanDoneCallback doneCallback);
    CHIP_ERROR Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling);
    StationStatus GetStationStatus() const;
    CHIP_ERROR ClearStationProvisioningData();
    CHIP_ERROR Disconnect();
    CHIP_ERROR GetWiFiInfo(WiFiInfo & info) const;
    CHIP_ERROR GetNetworkStatistics(NetworkStatistics & stats) const;

private:
    using ConnectionParams = wifi_connect_req_params;
    using NetEventCallback = net_mgmt_event_callback;
    using WiFiStatus       = wifi_status;
    using NetEventHandler  = void (*)(NetEventCallback *);

    struct NetworkEventData
    {
        NetEventCallback * mCallback;
        uint32_t mEvent;
    };

    constexpr static uint32_t kWifiManagementEvents = NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE |
        NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT | NET_EVENT_WIFI_IFACE_STATUS;

    CHIP_ERROR AddPsk(wifi_connect_req_params * params, const ByteSpan & credentials);
    CHIP_ERROR EnableStation(bool enable);
    CHIP_ERROR AddNetwork(const ByteSpan & ssid, const ByteSpan & credentials);
    uint8_t GetSecurityType() const;

    // Event handling
    static void WifiMgmtEventHandler(NetEventCallback * cb, uint32_t mgmtEvent, struct net_if * iface);
    static void ScanResultClbk(NetEventCallback * cb);
    static void ScanDoneClbk(NetEventCallback * cb);
    static void ConnectClbk(NetEventCallback * cb);
    static void DisconnectClbk(NetEventCallback * cb);
    static void PostConnectivityStatusChange(ConnectivityChange changeType);

    ConnectionParams mWiFiParams{};
    ConnectionHandling mHandling;
    wifi_iface_state mWiFiState;
    NetEventCallback mWiFiMgmtClbk{};
    ScanResultCallback mScanResultCallback{ nullptr };
    ScanDoneCallback mScanDoneCallback{ nullptr };
    static const Map<wifi_iface_state, StationStatus, 10> sStatusMap;
    static const Map<uint32_t, NetEventHandler, 4> sEventHandlerMap;
};

} // namespace DeviceLayer
} // namespace chip
