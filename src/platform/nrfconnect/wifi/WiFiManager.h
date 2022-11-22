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
#include <platform/NetworkCommissioning.h>
#include <system/SystemLayer.h>

#include <net/net_if.h>

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
    using ConnectionCallback = void (*)();

public:
    enum class StationStatus : uint8_t
    {
        NONE,
        DISCONNECTED,
        DISABLED,
        SCANNING,
        CONNECTING,
        CONNECTED,
        PROVISIONING,
        FULLY_PROVISIONED
    };

    static WiFiManager & Instance()
    {
        static WiFiManager sInstance;
        return sInstance;
    }

    using ScanCallback = void (*)(int /* status */, NetworkCommissioning::WiFiScanResponse *);

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
    CHIP_ERROR Scan(const ByteSpan & ssid, ScanCallback callback);
    CHIP_ERROR Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling);
    StationStatus GetStationStatus() const;
    CHIP_ERROR ClearStationProvisioningData();
    CHIP_ERROR DisconnectStation();
    CHIP_ERROR GetWiFiInfo(WiFiInfo & info) const;
    CHIP_ERROR GetNetworkStatistics(NetworkStatistics & stats) const;

private:
    CHIP_ERROR AddPsk(const ByteSpan & credentials);
    CHIP_ERROR EnableStation(bool enable);
    CHIP_ERROR AddNetwork(const ByteSpan & ssid, const ByteSpan & credentials);
    void PollTimerCallback();
    void WaitForConnectionAsync();
    void OnConnectionSuccess();
    void OnConnectionFailed();
    uint8_t GetSecurityType() const;

    WpaNetwork * mpWpaNetwork{ nullptr };
    ConnectionCallback mConnectionSuccessClbk;
    ConnectionCallback mConnectionFailedClbk;
    System::Clock::Timeout mConnectionTimeoutMs;
    ScanCallback mScanCallback{ nullptr };

    static uint8_t FrequencyToChannel(uint16_t freq);
    static StationStatus StatusFromWpaStatus(const wpa_states & status);

    static const Map<wpa_states, StationStatus, 10> sStatusMap;
    static const Map<uint16_t, uint8_t, 42> sFreqChannelMap;
};

} // namespace DeviceLayer
} // namespace chip
