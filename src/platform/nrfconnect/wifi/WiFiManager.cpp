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
#include <zephyr/net/net_event.h>

extern "C" {
#include <common/defs.h>
#include <wpa_supplicant/config.h>
#include <wpa_supplicant/driver_i.h>
#include <wpa_supplicant/scan.h>

// extern function to obtain bssid from status buffer
// It is defined in zephyr/subsys/net/ip/utils.c
extern char * net_sprint_ll_addr_buf(const uint8_t * ll, uint8_t ll_len, char * buf, int buflen);
}

namespace chip {
namespace DeviceLayer {

namespace {

NetworkCommissioning::WiFiScanResponse ToScanResponse(const wifi_scan_result * result)
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

const Map<wifi_iface_state, WiFiManager::StationStatus, 10>
    WiFiManager::sStatusMap({ { WIFI_STATE_DISCONNECTED, WiFiManager::StationStatus::DISCONNECTED },
                              { WIFI_STATE_INTERFACE_DISABLED, WiFiManager::StationStatus::DISABLED },
                              { WIFI_STATE_INACTIVE, WiFiManager::StationStatus::DISABLED },
                              { WIFI_STATE_SCANNING, WiFiManager::StationStatus::SCANNING },
                              { WIFI_STATE_AUTHENTICATING, WiFiManager::StationStatus::CONNECTING },
                              { WIFI_STATE_ASSOCIATING, WiFiManager::StationStatus::CONNECTING },
                              { WIFI_STATE_ASSOCIATED, WiFiManager::StationStatus::CONNECTED },
                              { WIFI_STATE_4WAY_HANDSHAKE, WiFiManager::StationStatus::PROVISIONING },
                              { WIFI_STATE_GROUP_HANDSHAKE, WiFiManager::StationStatus::PROVISIONING },
                              { WIFI_STATE_COMPLETED, WiFiManager::StationStatus::FULLY_PROVISIONED } });

const Map<uint32_t, WiFiManager::NetEventHandler, 4>
    WiFiManager::sEventHandlerMap({ { NET_EVENT_WIFI_SCAN_RESULT, WiFiManager::ScanResultClbk },
                                    { NET_EVENT_WIFI_SCAN_DONE, WiFiManager::ScanDoneClbk },
                                    { NET_EVENT_WIFI_CONNECT_RESULT, WiFiManager::ConnectClbk },
                                    { NET_EVENT_WIFI_DISCONNECT_RESULT, WiFiManager::DisconnectClbk } });

void WiFiManager::WifiMgmtEventHandler(NetEventCallback * cb, uint32_t mgmtEvent, struct net_if * iface)
{
    if (0 == strcmp(iface->if_dev->dev->name, "wlan0"))
    {
        DeviceLayer::SystemLayer().ScheduleLambda([cb, mgmtEvent] { sEventHandlerMap[mgmtEvent](cb); });
    }
}

CHIP_ERROR WiFiManager::Init()
{
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

    net_mgmt_init_event_callback(&mWiFiMgmtClbk, WifiMgmtEventHandler, kWifiManagementEvents);
    net_mgmt_add_event_callback(&mWiFiMgmtClbk);

    ChipLogDetail(DeviceLayer, "WiFiManager has been initialized");

    return CHIP_NO_ERROR;
}
CHIP_ERROR WiFiManager::Scan(const ByteSpan & ssid, ScanResultCallback resultCallback, ScanDoneCallback doneCallback)
{
    struct net_if * iface = InetUtils::GetInterface();
    VerifyOrReturnError(nullptr != iface, CHIP_ERROR_INTERNAL);

    mScanResultCallback = resultCallback;
    mScanDoneCallback   = doneCallback;
    mWiFiState          = WIFI_STATE_SCANNING;

    if (net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0))
    {
        ChipLogError(DeviceLayer, "Scan request failed");
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogDetail(DeviceLayer, "WiFi scanning started...");

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::ClearStationProvisioningData()
{
    memset(&mWiFiParams, 0, sizeof(mWiFiParams));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::Connect(const ByteSpan & ssid, const ByteSpan & credentials, const ConnectionHandling & handling)
{
    ChipLogDetail(DeviceLayer, "Connecting to WiFi network: %*s", ssid.size(), ssid.data());

    mHandling.mOnConnectionSuccess = handling.mOnConnectionSuccess;
    mHandling.mOnConnectionFailed  = handling.mOnConnectionFailed;

    mWiFiState = WIFI_STATE_ASSOCIATING;

    struct net_if * iface = InetUtils::GetInterface();

    VerifyOrReturnError(nullptr != iface, CHIP_ERROR_INTERNAL);

    // We can use pointers for ssid and credentials without clone them because setting new ones is blocked during associating.
    mWiFiParams.ssid        = ssid.data();
    mWiFiParams.ssid_length = ssid.size();
    mWiFiParams.psk         = const_cast<unsigned char *>(credentials.data());
    mWiFiParams.psk_length  = credentials.size();
    mWiFiParams.security    = WIFI_SECURITY_TYPE_PSK;
    mWiFiParams.timeout     = handling.mConnectionTimeoutMs.count();
    mWiFiParams.channel     = WIFI_CHANNEL_ANY;

    if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &mWiFiParams, sizeof(ConnectionParams)))
    {
        ChipLogError(DeviceLayer, "Connection request failed");
        if (Instance().mHandling.mOnConnectionFailed)
            Instance().mHandling.mOnConnectionFailed();
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogError(DeviceLayer, "Connection to %s requested", ssid.data());

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::Disconnect()
{
    struct net_if * iface = InetUtils::GetInterface();
    VerifyOrReturnError(nullptr != iface, CHIP_ERROR_INTERNAL);

    int status = net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0);

    if (status)
    {
        if (status == -EALREADY)
        {
            ChipLogDetail(DeviceLayer, "Already disconnected");
        }
        else
        {
            ChipLogDetail(DeviceLayer, "Disconnect request failed");
            return CHIP_ERROR_INTERNAL;
        }
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Disconnect requested");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiManager::GetWiFiInfo(WiFiInfo & info) const
{
    struct net_if * iface = InetUtils::GetInterface();
    VerifyOrReturnError(nullptr != iface, CHIP_ERROR_INTERNAL);
    struct wifi_iface_status status = { 0 };

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)))
    {
        ChipLogError(DeviceLayer, "Status request failed");
        return CHIP_ERROR_INTERNAL;
    }

    if (status.state >= WIFI_STATE_ASSOCIATED)
    {
        uint8_t mac_string_buf[sizeof("xx:xx:xx:xx:xx:xx")];
        net_sprint_ll_addr_buf(reinterpret_cast<const uint8_t *>(status.bssid), WIFI_MAC_ADDR_LEN,
                               reinterpret_cast<char *>(mac_string_buf), sizeof(mac_string_buf));
        info.mBssId        = ByteSpan(mac_string_buf, sizeof(mac_string_buf));
        info.mSecurityType = static_cast<uint8_t>(status.security);
        info.mWiFiVersion  = static_cast<uint8_t>(status.link_mode);
        info.mRssi         = status.rssi;
        info.mChannel      = status.channel;
        info.mSsidLen      = status.ssid_len;
        memcpy(info.mSsid, status.ssid, status.ssid_len);

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INTERNAL;
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

void WiFiManager::ScanResultClbk(NetEventCallback * cb)
{
    VerifyOrReturn(Instance().mScanResultCallback != nullptr);
    const struct wifi_scan_result * scanResult      = (const struct wifi_scan_result *) cb->info;
    NetworkCommissioning::WiFiScanResponse response = ToScanResponse(scanResult);
    Instance().mScanResultCallback(scanResult != nullptr ? &response : nullptr);
}

void WiFiManager::ScanDoneClbk(NetEventCallback * cb)
{
    VerifyOrReturn(Instance().mScanDoneCallback != nullptr);
    const WiFiStatus * status       = static_cast<const WiFiStatus *>(cb->info);
    WiFiRequestStatus requestStatus = static_cast<WiFiRequestStatus>(status->status);

    if (requestStatus == WiFiRequestStatus::FAIL)
    {
        ChipLogDetail(DeviceLayer, "Scan request failed (%d)", status->status);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Scan request done (%d)", status->status);
    }

    Instance().mScanDoneCallback(requestStatus);
}

void WiFiManager::ConnectClbk(NetEventCallback * cb)
{
    const WiFiStatus * status       = static_cast<const WiFiStatus *>(cb->info);
    WiFiRequestStatus requestStatus = static_cast<WiFiRequestStatus>(status->status);

    if (requestStatus == WiFiRequestStatus::FAIL)
    {
        ChipLogDetail(DeviceLayer, "Connection to WiFi network failed");
        Instance().mWiFiState = WIFI_STATE_DISCONNECTED;
        if (Instance().mHandling.mOnConnectionFailed)
        {
            Instance().mHandling.mOnConnectionFailed();
        }
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Connected to WiFi network");
        Instance().mWiFiState = WIFI_STATE_COMPLETED;
        if (Instance().mHandling.mOnConnectionSuccess)
        {
            Instance().mHandling.mOnConnectionSuccess();
        }
        Instance().PostConnectivityStatusChange(kConnectivity_Established);
    }
}

void WiFiManager::DisconnectClbk(NetEventCallback * cb)
{
    ChipLogDetail(DeviceLayer, "WiFi station disconnected");
    Instance().mWiFiState = WIFI_STATE_DISCONNECTED;
    Instance().PostConnectivityStatusChange(kConnectivity_Lost);
}

WiFiManager::StationStatus WiFiManager::GetStationStatus() const
{
    return WiFiManager::sStatusMap[mWiFiState];
}

void WiFiManager::PostConnectivityStatusChange(ConnectivityChange changeType)
{
    ChipDeviceEvent networkEvent{};
    networkEvent.Type                          = DeviceEventType::kWiFiConnectivityChange;
    networkEvent.WiFiConnectivityChange.Result = changeType;
    PlatformMgr().PostEventOrDie(&networkEvent);
}

} // namespace DeviceLayer
} // namespace chip
