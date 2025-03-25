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

#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/BL702/NetworkCommissioningDriver.h>
#include <platform/bouffalolab/BL702/wifi_mgmr_portable.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

CHIP_ERROR BLWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiSSID, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    SuccessOrExit(err);
    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiPassword, mSavedNetwork.ssid,
                                                   sizeof(mSavedNetwork.ssid), &ssidLen);
    SuccessOrExit(err);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork = mSavedNetwork;
    mScanSpecific   = false;

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
    mScanSpecific          = false;
    memset(mScanSSID, 0, sizeof(mScanSSID));
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
    ChipLogProgress(NetworkProvisioning, "ConnectWiFiNetwork");
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
    if (keyLen == BLWiFiDriver::WiFiCredentialLength::kOpen || keyLen == BLWiFiDriver::WiFiCredentialLength::kWEP64 ||
        (keyLen >= BLWiFiDriver::WiFiCredentialLength::kMinWPAPSK && keyLen <= BLWiFiDriver::WiFiCredentialLength::kMaxWPAPSK))
    {
        wifiInterface_disconnect();
        vTaskDelay(500);
        if (keyLen == BLWiFiDriver::WiFiCredentialLength::kOpen)
        {
            wifiInterface_connect((char *) ssid, NULL);
        }
        else
        {
            wifiInterface_connect((char *) ssid, (char *) key);
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting);
    return CHIP_NO_ERROR;
}

void BLWiFiDriver::OnConnectWiFiNetwork(bool isConnected)
{
    ChipLogProgress(NetworkProvisioning, "BLWiFiDriver::OnConnectWiFiNetwork, isConnected=%d\r\n", isConnected);
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
        if (ssid.data())
        {
            memset(mScanSSID, 0, sizeof(mScanSSID));
            memcpy(mScanSSID, ssid.data(), ssid.size());
            mScanSpecific = true;
        }

        mScanResponseNum = 0;
        mpScanCallback   = callback;
        wifiInterface_startScan();
    }
}

void BLWiFiDriver::OnScanWiFiNetworkDone(void * opaque)
{
    netbus_wifi_mgmr_msg_cmd_t * pkg_data = (netbus_wifi_mgmr_msg_cmd_t *) ((struct pkg_protocol *) opaque)->payload;
    netbus_fs_scan_ind_cmd_msg_t * pmsg   = (netbus_fs_scan_ind_cmd_msg_t *) ((netbus_fs_scan_ind_cmd_msg_t *) pkg_data);

    size_t i = 0, ap_num = 0, ap_cnt = 0;
    WiFiScanResponse *pScanResponse, *p;

    ChipLogProgress(DeviceLayer, "expected ssid %s. get %d in total, %d", mScanSSID, pmsg->num, mScanSpecific);

    if (mScanSpecific)
    {
        for (i = 0; i < pmsg->num; i++)
        {
            if (mScanSpecific && !strcmp(mScanSSID, (char *) (pmsg->records[i].ssid)))
            {
                ap_num = 1;
                break;
            }
        }
    }
    else
    {
        ap_num = pmsg->num;
    }

    if (ap_num)
    {
        p = mScanResponse = (WiFiScanResponse *) malloc(sizeof(WiFiScanResponse) * ap_num);
        if (mScanResponse == nullptr)
        {
            return;
        }

        for (i = 0; i < pmsg->num; i++)
        {
            if (mScanSpecific && strcmp(mScanSSID, (char *) (pmsg->records[i].ssid)))
            {
                continue;
            }

            p->security.SetRaw(pmsg->records[i].auth_mode);
            strncpy((char *) p->ssid, (const char *) pmsg->records[i].ssid, kMaxWiFiSSIDLength);
            p->ssidLen  = strlen((char *) pmsg->records[i].ssid);
            p->channel  = pmsg->records[i].channel;
            p->wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
            p->rssi     = pmsg->records[i].rssi;
            memcpy(p->bssid, pmsg->records[i].bssid, 6);

            p++;
            ap_cnt++;

            if (ap_cnt >= ap_num)
            {
                break;
            }
        }
    }

    mScanResponseNum = ap_cnt;
}

void BLWiFiDriver::OnScanWiFiNetworkDone(void)
{
    size_t ap_cnt                    = mScanResponseNum;
    WiFiScanResponse * pScanResponse = mScanResponse;

    if (mScanResponse)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_cnt, pScanResponse]() {
                BLScanResponseIterator iter(ap_cnt, pScanResponse);
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
            ChipLogProgress(DeviceLayer, "ScheduleLambda OK");
        }

        free(mScanResponse);
        mScanResponse = nullptr;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
    }

    mScanResponseNum = 0;
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    struct bflbwifi_ap_record * pApInfo = wifiInterface_getApInfo();

    if (NULL == pApInfo)
    {
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "GetConfiguredNetwork [%s]", pApInfo->ssid);

    uint8_t length = strnlen(reinterpret_cast<const char *>(pApInfo->ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, pApInfo->ssid, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}

void BLWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staConnected = false;

    ChipLogProgress(DeviceLayer, "OnNetworkStatusChange");

    VerifyOrReturn(mpStatusChangeCallback != nullptr);

    if (CHIP_NO_ERROR != GetConfiguredNetwork(configuredNetwork))
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status.");
        return;
    }

    if (ConnectivityMgrImpl().GetWiFiStationState() == ConnectivityManager::kWiFiStationState_Connected)
    {
        ChipLogProgress(DeviceLayer, "OnNetworkStatusChange kWiFiStationState_Connected, %s", configuredNetwork.networkID);
        staConnected = true;
    }

    if (staConnected)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
        return;
    }
    mpStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
}

CHIP_ERROR BLWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    // TODO: to be added
    mLastDisconnectedReason = 0;

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

    Network configuredNetwork;
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (ConnectivityMgrImpl()._IsWiFiStationConnected() && configuredNetwork.networkIDLen == item.networkIDLen &&
            memcmp(configuredNetwork.networkID, item.networkID, item.networkIDLen) == 0)
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

extern "C" void wifi_event_handler(uint32_t code)
{
    ChipDeviceEvent event;

    memset(&event, 0, sizeof(ChipDeviceEvent));
    switch (code)
    {
    case VIRT_NET_EV_ON_CONNECTED:
        event.Type = kWiFiOnConnected;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case VIRT_NET_EV_ON_GOT_IP:
        event.Type = kGotIpAddress;
        PlatformMgr().PostEventOrDie(&event);
        break;
    case VIRT_NET_EV_ON_DISCONNECT:
        event.Type = kWiFiOnDisconnected;
        PlatformMgr().PostEventOrDie(&event);
        break;
    default:
        ChipLogProgress(DeviceLayer, "[APP] [EVT] Unknown code %lu \r\n", code);
    }
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
