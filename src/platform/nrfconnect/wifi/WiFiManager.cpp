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
 *          Provides the wrapper for nRF WiFi API
 */

#include "WiFiManager.h"

#include <inet/InetInterface.h>
#include <inet/UDPEndPointImplSockets.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Zephyr/InetUtils.h>

#include <net/net_stats.h>
#include <zephyr.h>

extern "C" {
#include <common/defs.h>
#include <wpa_supplicant/config.h>
#include <wpa_supplicant/driver_i.h>
#include <wpa_supplicant/scan.h>
#include <zephyr/net/wifi_mgmt.h>
}

extern struct wpa_global * global;

static struct wpa_supplicant * wpa_s;

namespace chip {
namespace DeviceLayer {

namespace {

NetworkCommissioning::WiFiScanResponse ToScanResponse(wifi_scan_result * result)
{
    NetworkCommissioning::WiFiScanResponse response = {};

    if (result != nullptr)
    {
        static_assert(sizeof(response.ssid) == sizeof(result->ssid), "SSID length mismatch");
        static_assert(sizeof(response.bssid) == sizeof(result->mac), "BSSID length mismatch");

        // TODO: Distinguish WPA versions
        response.security.Set(result->security == WIFI_SECURITY_TYPE_PSK ? NetworkCommissioning::WiFiSecurity::kWpaPersonal
                                                                         : NetworkCommissioning::WiFiSecurity::kUnencrypted);
        response.channel = result->channel;
        response.rssi    = result->rssi;
        response.ssidLen = result->ssid_length;
        memcpy(response.ssid, result->ssid, result->ssid_length);
        // TODO: MAC/BSSID is not filled by the Wi-Fi driver
        memcpy(response.bssid, result->mac, result->mac_length);
    }

    return response;
}

} // namespace

// These enums shall reflect the overall ordered disconnected->connected flow
const Map<wpa_states, WiFiManager::StationStatus, 10>
    WiFiManager::sStatusMap({ { WPA_DISCONNECTED, WiFiManager::StationStatus::DISCONNECTED },
                              { WPA_INTERFACE_DISABLED, WiFiManager::StationStatus::DISABLED },
                              { WPA_INACTIVE, WiFiManager::StationStatus::DISABLED },
                              { WPA_SCANNING, WiFiManager::StationStatus::SCANNING },
                              { WPA_AUTHENTICATING, WiFiManager::StationStatus::CONNECTING },
                              { WPA_ASSOCIATING, WiFiManager::StationStatus::CONNECTING },
                              { WPA_ASSOCIATED, WiFiManager::StationStatus::CONNECTED },
                              { WPA_4WAY_HANDSHAKE, WiFiManager::StationStatus::PROVISIONING },
                              { WPA_GROUP_HANDSHAKE, WiFiManager::StationStatus::PROVISIONING },
                              { WPA_COMPLETED, WiFiManager::StationStatus::FULLY_PROVISIONED } });

// Map WiFi center frequency to the corresponding channel number
const Map<uint16_t, uint8_t, 42> WiFiManager::sFreqChannelMap(
    { { 4915, 183 }, { 4920, 184 }, { 4925, 185 }, { 4935, 187 }, { 4940, 188 }, { 4945, 189 }, { 4960, 192 },
      { 4980, 196 }, { 5035, 7 },   { 5040, 8 },   { 5045, 9 },   { 5055, 11 },  { 5060, 12 },  { 5080, 16 },
      { 5170, 34 },  { 5180, 36 },  { 5190, 38 },  { 5200, 40 },  { 5210, 42 },  { 5220, 44 },  { 5230, 46 },
      { 5240, 48 },  { 5260, 52 },  { 5280, 56 },  { 5300, 60 },  { 5320, 64 },  { 5500, 100 }, { 5520, 104 },
      { 5540, 108 }, { 5560, 112 }, { 5580, 116 }, { 5600, 120 }, { 5620, 124 }, { 5640, 128 }, { 5660, 132 },
      { 5680, 136 }, { 5700, 140 }, { 5745, 149 }, { 5765, 153 }, { 5785, 157 }, { 5805, 161 }, { 5825, 165 } });

CHIP_ERROR WiFiManager::Init()
{
    // wpa_supplicant instance is initialized in dedicated supplicant thread, so wait until
    // the initialization is completed.
    // TODO: fix thread-safety of the solution.
    constexpr size_t kInitTimeoutMs = 5000;
    const int64_t initStartTime     = k_uptime_get();
    // TODO: Handle multiple VIFs
    const char * ifname = "wlan0";

    while (!global || !(wpa_s = wpa_supplicant_get_iface(global, ifname)))
    {
        if (k_uptime_get() > initStartTime + kInitTimeoutMs)
        {
            ChipLogError(DeviceLayer, "wpa_supplicant is not initialized!");
            return CHIP_ERROR_INTERNAL;
        }

        k_msleep(200);
    }

    // TODO: consider moving these to ConnectivityManagerImpl to be prepared for handling multiple interfaces on a single device.
    Inet::UDPEndPointImplSockets::SetJoinMulticastGroupHandler([](Inet::InterfaceId interfaceId, const Inet::IPAddress & address) {
        const in6_addr addr = InetUtils::ToZephyrAddr(address);
        net_if * iface      = InetUtils::GetInterface(interfaceId);
        VerifyOrReturnError(iface != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

        net_if_mcast_addr * maddr = net_if_ipv6_maddr_add(iface, &addr);

        if (maddr && !net_if_ipv6_maddr_is_joined(maddr) && !net_ipv6_is_addr_mcast_link_all_nodes(&addr))
        {
            net_if_ipv6_maddr_join(maddr);
        }

        return CHIP_NO_ERROR;
    });

    Inet::UDPEndPointImplSockets::SetLeaveMulticastGroupHandler([](Inet::InterfaceId interfaceId, const Inet::IPAddress & address) {
        const in6_addr addr = InetUtils::ToZephyrAddr(address);
        net_if * iface      = InetUtils::GetInterface(interfaceId);
        VerifyOrReturnError(iface != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

        if (!net_ipv6_is_addr_mcast_link_all_nodes(&addr) && !net_if_ipv6_maddr_rm(iface, &addr))
        {
            return CHIP_ERROR_INVALID_ADDRESS;
        }

        return CHIP_NO_ERROR;
    });

    ChipLogDetail(DeviceLayer, "wpa_supplicant has been initialized");

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::AddNetwork(const ByteSpan & ssid, const ByteSpan & credentials)
{
    ChipLogDetail(DeviceLayer, "Adding WiFi network");
    mpWpaNetwork = wpa_supplicant_add_network(wpa_s);
    if (mpWpaNetwork)
    {
        static constexpr size_t kMaxSsidLen{ 32 };
        mpWpaNetwork->ssid = (u8 *) k_malloc(kMaxSsidLen);

        if (mpWpaNetwork->ssid)
        {
            memcpy(mpWpaNetwork->ssid, ssid.data(), ssid.size());
            mpWpaNetwork->ssid_len    = ssid.size();
            mpWpaNetwork->key_mgmt    = WPA_KEY_MGMT_NONE;
            mpWpaNetwork->disabled    = 1;
            wpa_s->conf->filter_ssids = 1;

            return AddPsk(credentials);
        }
    }

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR WiFiManager::Scan(const ByteSpan & ssid, ScanCallback callback)
{
    const StationStatus stationStatus = GetStationStatus();
    VerifyOrReturnError(stationStatus != StationStatus::DISABLED && stationStatus != StationStatus::SCANNING &&
                            stationStatus != StationStatus::CONNECTING,
                        CHIP_ERROR_INCORRECT_STATE);

    net_if * const iface = InetUtils::GetInterface();
    VerifyOrReturnError(iface != nullptr, CHIP_ERROR_INTERNAL);

    const device * dev = net_if_get_device(iface);
    VerifyOrReturnError(dev != nullptr, CHIP_ERROR_INTERNAL);

    const net_wifi_mgmt_offload * ops = static_cast<const net_wifi_mgmt_offload *>(dev->api);
    VerifyOrReturnError(ops != nullptr, CHIP_ERROR_INTERNAL);

    mScanCallback = callback;

    // TODO: Use saner API once such exists.
    // TODO: Take 'ssid' into account.
    VerifyOrReturnError(ops->scan(dev,
                                  [](net_if *, int status, wifi_scan_result * result) {
                                      VerifyOrReturn(Instance().mScanCallback != nullptr);
                                      NetworkCommissioning::WiFiScanResponse response = ToScanResponse(result);
                                      Instance().mScanCallback(status, result != nullptr ? &response : nullptr);
                                  }) == 0,
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling)
{
    ChipLogDetail(DeviceLayer, "Connecting to WiFi network");

    mConnectionSuccessClbk = handling.mOnConnectionSuccess;
    mConnectionFailedClbk  = handling.mOnConnectionFailed;
    mConnectionTimeoutMs   = handling.mConnectionTimeoutMs;

    CHIP_ERROR err = AddNetwork(ssid, credentials);
    if (CHIP_NO_ERROR == err)
    {
        EnableStation(true);
        wpa_supplicant_select_network(wpa_s, mpWpaNetwork);
        WaitForConnectionAsync();
    }
    else
    {
        OnConnectionFailed();
    }
    return err;
}

void WiFiManager::OnConnectionSuccess()
{
    if (mConnectionSuccessClbk)
        mConnectionSuccessClbk();
}

void WiFiManager::OnConnectionFailed()
{
    if (mConnectionFailedClbk)
        mConnectionFailedClbk();
}

CHIP_ERROR WiFiManager::AddPsk(const ByteSpan & credentials)
{
    mpWpaNetwork->key_mgmt = WPA_KEY_MGMT_PSK;
    str_clear_free(mpWpaNetwork->passphrase);
    mpWpaNetwork->passphrase = dup_binstr(credentials.data(), credentials.size());

    if (mpWpaNetwork->passphrase)
    {
        wpa_config_update_psk(mpWpaNetwork);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INTERNAL;
}

WiFiManager::StationStatus WiFiManager::GetStationStatus() const
{
    if (wpa_s)
    {
        return StatusFromWpaStatus(wpa_s->wpa_state);
    }
    else
    {
        ChipLogError(DeviceLayer, "wpa_supplicant is not initialized!");
        return StationStatus::NONE;
    }
}

WiFiManager::StationStatus WiFiManager::StatusFromWpaStatus(const wpa_states & status)
{
    ChipLogDetail(DeviceLayer, "WPA internal status: %d", static_cast<int>(status));
    return WiFiManager::sStatusMap[status];
}

CHIP_ERROR WiFiManager::EnableStation(bool enable)
{
    VerifyOrReturnError(nullptr != wpa_s && nullptr != mpWpaNetwork, CHIP_ERROR_INTERNAL);
    if (enable)
    {
        wpa_supplicant_enable_network(wpa_s, mpWpaNetwork);
    }
    else
    {
        wpa_supplicant_disable_network(wpa_s, mpWpaNetwork);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::ClearStationProvisioningData()
{
    VerifyOrReturnError(nullptr != wpa_s && nullptr != mpWpaNetwork, CHIP_ERROR_INTERNAL);
    wpa_supplicant_cancel_scan(wpa_s);
    wpa_clear_keys(wpa_s, mpWpaNetwork->bssid);
    str_clear_free(mpWpaNetwork->passphrase);
    wpa_config_update_psk(mpWpaNetwork);
    wpa_supplicant_set_state(wpa_s, WPA_INACTIVE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::DisconnectStation()
{
    VerifyOrReturnError(nullptr != wpa_s, CHIP_ERROR_INTERNAL);
    wpa_supplicant_cancel_scan(wpa_s);
    wpas_request_disconnection(wpa_s);

    return CHIP_NO_ERROR;
}

void WiFiManager::WaitForConnectionAsync()
{
    chip::DeviceLayer::SystemLayer().StartTimer(
        static_cast<System::Clock::Timeout>(1000), [](System::Layer *, void *) { Instance().PollTimerCallback(); }, nullptr);
}

void WiFiManager::PollTimerCallback()
{
    const uint32_t kMaxRetriesNumber{ mConnectionTimeoutMs.count() / 1000 };
    static uint32_t retriesNumber{ 0 };

    if (WiFiManager::StationStatus::FULLY_PROVISIONED == GetStationStatus())
    {
        retriesNumber = 0;
        OnConnectionSuccess();
    }
    else
    {
        if (retriesNumber++ < kMaxRetriesNumber)
        {
            // wait more time
            WaitForConnectionAsync();
        }
        else
        {
            // connection timeout
            retriesNumber = 0;
            OnConnectionFailed();
        }
    }
}

CHIP_ERROR WiFiManager::GetWiFiInfo(WiFiInfo & info) const
{
    VerifyOrReturnError(nullptr != wpa_s, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(nullptr != mpWpaNetwork, CHIP_ERROR_INTERNAL);

    static uint8_t sBssid[ETH_ALEN];
    if (WiFiManager::StationStatus::CONNECTED <= GetStationStatus())
    {
        memcpy(sBssid, wpa_s->bssid, ETH_ALEN);
        info.mBssId        = ByteSpan(sBssid, ETH_ALEN);
        info.mSecurityType = GetSecurityType();
        // TODO: this should reflect the real connection compliance
        // i.e. the AP might support WiFi 5 only even though the station
        // is WiFi 6 ready (so the connection is WiFi 5 effectively).
        // For now just return what the station supports.
        info.mWiFiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_AX;

        wpa_signal_info signalInfo{};
        if (0 == wpa_drv_signal_poll(wpa_s, &signalInfo))
        {
            info.mRssi    = signalInfo.current_signal; // dBm
            info.mChannel = FrequencyToChannel(signalInfo.frequency);
        }
        else
        {
            // this values should be nullable according to the Matter spec
            info.mRssi    = std::numeric_limits<decltype(info.mRssi)>::min();
            info.mChannel = std::numeric_limits<decltype(info.mChannel)>::min();
        }

        memcpy(info.mSsid, mpWpaNetwork->ssid, mpWpaNetwork->ssid_len);
        info.mSsidLen = mpWpaNetwork->ssid_len;

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INTERNAL;
}

uint8_t WiFiManager::GetSecurityType() const
{
    VerifyOrReturnValue(nullptr != mpWpaNetwork, EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED);

    if ((mpWpaNetwork->key_mgmt & WPA_KEY_MGMT_NONE) || !wpa_key_mgmt_wpa_any(mpWpaNetwork->key_mgmt))
    {
        return EMBER_ZCL_SECURITY_TYPE_NONE;
    }
    else if (wpa_key_mgmt_wpa_psk_no_sae(mpWpaNetwork->key_mgmt))
    {
        return (mpWpaNetwork->pairwise_cipher & (WPA_CIPHER_TKIP | WPA_CIPHER_CCMP)) ? EMBER_ZCL_SECURITY_TYPE_WPA2
                                                                                     : EMBER_ZCL_SECURITY_TYPE_WPA3;
    }
    else if (wpa_key_mgmt_sae(mpWpaNetwork->key_mgmt))
    {
        return EMBER_ZCL_SECURITY_TYPE_WPA3;
    }
    else
    {
        return EMBER_ZCL_SECURITY_TYPE_WEP;
    }

    return EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
}

uint8_t WiFiManager::FrequencyToChannel(uint16_t freq)
{
    static constexpr uint16_t k24MinFreq{ 2401 };
    static constexpr uint16_t k24MaxFreq{ 2484 };
    static constexpr uint8_t k24FreqConstDiff{ 5 };

    if (freq >= k24MinFreq && freq < k24MaxFreq)
    {
        return static_cast<uint8_t>((freq - k24MinFreq) / k24FreqConstDiff + 1);
    }
    else if (freq == k24MaxFreq)
    {
        return 14;
    }
    else if (freq > k24MaxFreq)
    {
        // assume we are in 5GH band
        return sFreqChannelMap[freq];
    }
    return 0;
}

CHIP_ERROR WiFiManager::GetNetworkStatistics(NetworkStatistics & stats) const
{
    // TODO: below will not work (result will be all zeros) until
    // the get_stats handler is implemented in WiFi driver
    net_stats_eth data{};
    net_mgmt(NET_REQUEST_STATS_GET_ETHERNET, InetUtils::GetInterface(), &data, sizeof(data));

    stats.mPacketMulticastRxCount = data.multicast.rx;
    stats.mPacketMulticastTxCount = data.multicast.tx;
    stats.mPacketUnicastRxCount   = data.pkts.rx - data.multicast.rx - data.broadcast.rx;
    stats.mPacketUnicastTxCount   = data.pkts.tx - data.multicast.tx - data.broadcast.tx;
    stats.mOverruns               = 0; // TODO: clarify if this can be queried from mgmt API (e.g. data.tx_dropped)

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
