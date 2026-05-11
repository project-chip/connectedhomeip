/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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
#include <stdint.h>
#include <string.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/common/NetworkCommissioningDriver.h>

#include <lwip/ethip6.h>
#include <lwip/tcpip.h>

#include <wifi_mgmr_ext.h>
#include <wifi_mgmr_portable.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

auto converter = [](const wifi_mgmr_ap_item_t & raw) -> WiFiScanResponse {
    WiFiScanResponse item;

    item.security.SetRaw(raw.auth);
    item.ssidLen         = raw.ssid_len < chip::DeviceLayer::Internal::kMaxWiFiSSIDLength ? raw.ssid_len
                                                                                          : chip::DeviceLayer::Internal::kMaxWiFiSSIDLength;
    item.channel         = raw.channel;
    item.wiFiBand        = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
    item.signal.type     = WirelessSignalType::kdBm;
    item.signal.strength = raw.rssi;
    memcpy(item.ssid, raw.ssid, item.ssidLen);
    memcpy(item.bssid, raw.bssid, 6);

    return item;
};

CHIP_ERROR BflbWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(BflbConfig::kConfigKey_WiFiPassword, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    SuccessOrExit(err);
    err = PersistedStorage::KeyValueStoreMgr().Get(BflbConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                                   &ssidLen);
    SuccessOrExit(err);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork        = mSavedNetwork;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

exit:
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        /** Not commissioned, SSID and Network Password not found. */
        return CHIP_NO_ERROR;
    }

    return err;
}

void BflbWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR BflbWiFiDriver::CommitConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "BflbWiFiDriver::CommitConfiguration");
    ReturnErrorOnFailure(
        PersistedStorage::KeyValueStoreMgr().Put(BflbConfig::kConfigKey_WiFiSSID, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(BflbConfig::kConfigKey_WiFiPassword, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BflbWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool BflbWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status BflbWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                          uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    return Status::kSuccess;
}

Status BflbWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status BflbWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR BflbWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    char wifi_ssid[64] = { 0 };
    char passwd[64]    = { 0 };
    wifi_interface_t wifi_interface;

    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting);

    memcpy(wifi_ssid, ssid, ssidLen);
    memcpy(passwd, key, keyLen);

    wifi_interface = wifi_mgmr_sta_enable();
    // Valid Credentials length are:
    // - 0 bytes: Unsecured (open) connection
    // - 5 bytes: WEP-64 passphrase
    // - 10 hexadecimal ASCII characters: WEP-64 40-bit hex raw PSK
    // - 13 bytes: WEP-128 passphrase
    // - 26 hexadecimal ASCII characters: WEP-128 104-bit hex raw PSK
    // - 8..63 bytes: WPA/WPA2/WPA3 passphrase
    // - 64 bytes: WPA/WPA2/WPA3 raw hex PSK
    // Note 10 hex WEP64 and 13 bytes / 26 hex WEP128 passphrase are covered by 8~63 bytes WPA passphrase, so we don't check WEP64
    // hex and WEP128 passphrase.
    if (keyLen == BflbWiFiDriver::WiFiCredentialLength::kOpen || keyLen == BflbWiFiDriver::WiFiCredentialLength::kWEP64 ||
        (keyLen >= BflbWiFiDriver::WiFiCredentialLength::kMinWPAPSK && keyLen <= BflbWiFiDriver::WiFiCredentialLength::kMaxWPAPSK))
    {

        if (keyLen == BflbWiFiDriver::WiFiCredentialLength::kOpen)
        {
            wifi_mgmr_sta_connect(&wifi_interface, wifi_ssid, NULL, NULL, NULL, 0, 0);
        }
        else
        {
            wifi_mgmr_sta_connect(&wifi_interface, wifi_ssid, passwd, NULL, NULL, 0, 0);
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    return CHIP_NO_ERROR;
}

void BflbWiFiDriver::OnConnectWiFiNetwork(bool isConnected)
{
    if (mpConnectCallback)
    {
        if (isConnected)
        {
            mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        }
        else
        {
            mpConnectCallback->OnResult(Status::kUnknownError, CharSpan(), 0);
        }
        mpConnectCallback = nullptr;
    }
}

void BflbWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "BL NetworkCommissioningDelegate: SSID: %s", NullTerminated(networkId).c_str());

    err               = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                                           reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);
    mpConnectCallback = callback;

exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
        mpConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

void BflbWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback  = nullptr;
        mScanSSIDlength = 0;

        if (ssid.size() <= sizeof(mScanSSID) && 0 == wifi_mgmr_scan(nullptr, nullptr))
        {
            memcpy(mScanSSID, ssid.data(), ssid.size());
            mScanSSIDlength = ssid.size();
            mpScanCallback  = callback;
        }
        else
        {
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

void BflbWiFiDriver::OnScanWiFiNetworkDone()
{
    int ap_num                        = 0;
    wifi_mgmr_ap_item_t * pScanResult = nullptr;

    ap_num = wifi_mgmr_get_scan_ap_num();
    if (ap_num)
    {
        if (mScanSSIDlength)
        {
            ap_num = 1;
        }
        pScanResult = (wifi_mgmr_ap_item_t *) pvPortMalloc(ap_num * sizeof(wifi_mgmr_ap_item_t));
    }
    if (pScanResult)
    {
        wifi_mgmr_get_scan_result(pScanResult, &ap_num, mScanSSIDlength, mScanSSID);
    }
    else
    {
        ap_num = 0;
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([ap_num, pScanResult]() {
        if (GetInstance().mpScanCallback)
        {
            BflbScanResponseIterator iter(ap_num, pScanResult, converter);
            GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
        }
        else
        {
            ChipLogError(DeviceLayer, "can't find the ScanCallback function");
        }
    });

    if (pScanResult)
    {
        vPortFree(pScanResult);
    }
    mScanSSIDlength = 0;
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    uint8_t ssid[64];
    uint16_t ssid_len;

    ssid_len = wifi_mgmr_profile_ssid_get(ssid);
    if (!ssid_len || ssid_len > DeviceLayer::Internal::kMaxWiFiSSIDLength)
    {
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, ssid, ssid_len);
    network.networkIDLen = ssid_len;

    return CHIP_NO_ERROR;
}

void BflbWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staConnected = false;

    VerifyOrReturn(mpStatusChangeCallback != nullptr);
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    if (ConnectivityManagerImpl().GetWiFiStationState() == ConnectivityManager::kWiFiStationState_Connected)
    {
        staConnected = true;
    }

    if (staConnected)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
        return;
    }
    mpStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
        MakeOptional(GetLastDisconnectReason()));
}

CHIP_ERROR BflbWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    uint16_t status_code, reason_code;

    wifi_mgmr_conn_result_get(&status_code, &reason_code);
    mLastDisconnectedReason = reason_code;

    return CHIP_NO_ERROR;
}

int32_t BflbWiFiDriver::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t BflbWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool BflbWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = false;
    mExhausted        = true;

    Network connectedNetwork;
    CHIP_ERROR err = GetConfiguredNetwork(connectedNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (connectedNetwork.networkIDLen == item.networkIDLen &&
            memcmp(connectedNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }

    return true;
}

void NetworkEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    if (!(DeviceEventType::IsPlatformSpecific(event->Type) && DeviceEventType::IsPublic(event->Type)))
    {
        return;
    }

    switch (event->Type)
    {
    case kWiFiOnInitDone:
        break;
    case kWiFiOnScanDone:
        BflbWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
        break;
    case kWiFiOnConnecting:
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting);
        ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        break;
    case kWiFiOnConnected:
        BflbWiFiDriver::GetInstance().OnNetworkStatusChange();
        break;
    case kGotIpAddress:
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManagerImpl::kWiFiStationState_Connected);
        ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        break;
    case kGotIpv6Address:
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManagerImpl::kWiFiStationState_Connected);
        ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        break;
    case kWiFiOnDisconnected:
        if (ConnectivityManager::kWiFiStationState_Connecting == ConnectivityMgrImpl().GetWiFiStationState())
        {
            ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting_Failed);
        }
        break;
    default:
        ChipLogProgress(DeviceLayer, "Undefined network commission event type %x.\r\n", event->Type);
        break;
    }
}

static wifi_conf_t conf = {
    .country_code = "CN",
};

extern "C" void wifi_event_handler(uint32_t code)
{
    ChipDeviceEvent event;

    memset(&event, 0, sizeof(ChipDeviceEvent));

    switch (code)
    {
    case CODE_WIFI_ON_INIT_DONE:
        wifi_mgmr_start_background(&conf);
        break;
    case CODE_WIFI_ON_MGMR_DONE:
        break;
    case CODE_WIFI_ON_SCAN_DONE:
        event.Type = kWiFiOnScanDone;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case CODE_WIFI_ON_CONNECTING:
        event.Type = kWiFiOnConnecting;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case CODE_WIFI_ON_CONNECTED:
        event.Type = kWiFiOnConnected;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case CODE_WIFI_ON_GOT_IP:
        event.Type = kGotIpAddress;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case CODE_WIFI_ON_DISCONNECT:
        event.Type = kWiFiOnDisconnected;
        PlatformMgr().PostEventOrDie(&event);
        break;
    default:
        ChipLogProgress(DeviceLayer, "[APP] [EVT] Unknown code %lu \r\n", code);
    }
}

extern "C" void network_netif_ext_callback(struct netif * nif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args)
{
    ChipDeviceEvent event;

    memset(&event, 0, sizeof(ChipDeviceEvent));

    if ((LWIP_NSC_STATUS_CHANGED & reason) && args && !nif->ip6_autoconfig_enabled)
    {
        nif->flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_MLD6;
        nif->output_ip6 = ethip6_output;
        netif_create_ip6_linklocal_address(nif, 1);
        nif->ip6_autoconfig_enabled = 1;
    }

    if ((LWIP_NSC_IPV6_ADDR_STATE_CHANGED & reason) && args)
    {
        if (args->ipv6_addr_state_changed.addr_index >= LWIP_IPV6_NUM_ADDRESSES ||
            ip6_addr_islinklocal(netif_ip6_addr(nif, args->ipv6_addr_state_changed.addr_index)))
        {
            return;
        }

        if (netif_ip6_addr_state(nif, args->ipv6_addr_state_changed.addr_index) != args->ipv6_addr_state_changed.old_state &&
            ip6_addr_ispreferred(netif_ip6_addr_state(nif, args->ipv6_addr_state_changed.addr_index)))
        {
            event.Type = kGotIpv6Address;
            PlatformMgr().PostEventOrDie(&event);
        }
    }
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
