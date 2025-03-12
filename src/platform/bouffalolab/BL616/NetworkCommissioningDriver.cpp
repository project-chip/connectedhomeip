/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/BL616/NetworkCommissioningDriver.h>
extern "C" {
#include <wifi_mgmr_ext.h>
}
#include <wifi_mgmr_portable.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::Platform;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

CHIP_ERROR BLWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiPassword, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    SuccessOrExit(err);
    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
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

void BLWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR BLWiFiDriver::CommitConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "BLWiFiDriver::CommitConfiguration");
    ReturnErrorOnFailure(
        PersistedStorage::KeyValueStoreMgr().Put(BLConfig::kConfigKey_WiFiSSID, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(BLConfig::kConfigKey_WiFiPassword, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool BLWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status BLWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
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

Status BLWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status BLWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR BLWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    wifi_mgmr_sta_connect_params_t conn_param = { 0 };

    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting);

    strncpy((char *) conn_param.ssid, ssid, ssidLen);
    conn_param.ssid_len = ssidLen;

    if (keyLen)
    {
        strncpy((char *) conn_param.key, key, keyLen);
        conn_param.key_len = keyLen;
    }
    conn_param.freq1         = 0;
    conn_param.freq2         = 0;
    conn_param.use_dhcp      = 1;
    conn_param.pmf_cfg       = 1;
    conn_param.quick_connect = 1;
    conn_param.timeout_ms    = -1;

    wifi_mgmr_sta_connect(&conn_param);

    return CHIP_NO_ERROR;
}

void BLWiFiDriver::OnConnectWiFiNetwork(bool isConnected)
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

void BLWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "BL NetworkCommissioningDelegate: SSID: %.*s", static_cast<int>(networkId.size()),
                    networkId.data());

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
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network:%s", chip::ErrorStr(err));
        mpConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

void BLWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback = nullptr;
        if (0 == wifi_start_scan(ssid.data(), ssid.size()))
        {
            mpScanCallback = callback;
        }
        else
        {
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

void BLWiFiDriver::OnScanWiFiNetworkDone()
{
    uint32_t nums = wifi_mgmr_sta_scanlist_nums_get();
    if (nums)
    {
        wifi_mgmr_scan_item_t * pScanList = (wifi_mgmr_scan_item_t *) MemoryAlloc(nums * sizeof(wifi_mgmr_scan_item_t));

        if (NULL == pScanList || 0 == wifi_mgmr_sta_scanlist_dump(pScanList, nums))
        {
            mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        else
        {

            if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([nums, pScanList]() {
                    BLScanResponseIterator iter(nums, pScanList);
                    if (GetInstance().mpScanCallback)
                    {
                        GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
                        GetInstance().mpScanCallback = nullptr;
                    }
                    else
                    {
                        ChipLogError(DeviceLayer, "can't find the ScanCallback function");
                    }
                }))
            {
                MemoryFree(pScanList);
            }
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kNetworkNotFound, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
    }
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    wifi_mgmr_connect_ind_stat_info_t statInfo;

    memset(&statInfo, 0, sizeof(wifi_mgmr_connect_ind_stat_info_t));
    if (0 == wifi_mgmr_sta_connect_ind_stat_get(&statInfo))
    {
        memcpy(network.networkID, statInfo.ssid, strlen(statInfo.ssid));
        network.networkIDLen = strlen(statInfo.ssid);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INTERNAL;
}

void BLWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;

    VerifyOrReturn(mpStatusChangeCallback != nullptr);
    if (GetConfiguredNetwork(configuredNetwork) != CHIP_NO_ERROR)
    {
        return;
    }

    if (ConnectivityManagerImpl().GetWiFiStationState() == ConnectivityManager::kWiFiStationState_Connected)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
    }
    else
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
            MakeOptional(GetLastDisconnectReason()));
    }
}

CHIP_ERROR BLWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    mLastDisconnectedReason = wifi_mgmr_sta_info_status_code_get();

    return CHIP_NO_ERROR;
}

int32_t BLWiFiDriver::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t BLWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool BLWiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
        BLWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
        break;
    case kWiFiOnConnecting:
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting);
        ConnectivityMgrImpl().OnConnectivityChanged(deviceInterface_getNetif());
        break;
    case kWiFiOnConnected:
        BLWiFiDriver::GetInstance().OnNetworkStatusChange();
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
        wifi_mgmr_init(&conf);
        break;
    case CODE_WIFI_ON_MGMR_DONE:
        wifi_mgmr_sta_autoconnect_enable();
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
    default: {
        ChipLogProgress(DeviceLayer, "[APP] [EVT] Unknown code %lu \r\n", code);
    }
    }
}

extern "C" void network_netif_ext_callback(struct netif * nif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args)
{
    ChipDeviceEvent event;

    memset(&event, 0, sizeof(ChipDeviceEvent));

    if (((LWIP_NSC_IPV6_ADDR_STATE_CHANGED | LWIP_NSC_IPV6_SET) & reason) && args)
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

    if ((LWIP_NSC_IPV4_SETTINGS_CHANGED & reason) && args)
    {
        if (!ip4_addr_isany(netif_ip4_addr(nif)) && !ip4_addr_isany(netif_ip4_gw(nif)))
        {
            event.Type = kGotIpAddress;
            PlatformMgr().PostEventOrDie(&event);
        }
    }
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
// #endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
