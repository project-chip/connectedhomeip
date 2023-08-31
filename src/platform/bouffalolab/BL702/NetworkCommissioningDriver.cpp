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
#include <platform/bouffalolab/BL702/WiFiInterface.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

//#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

CHIP_ERROR BLWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                                   &ssidLen);
    SuccessOrExit(err);
    err = PersistedStorage::KeyValueStoreMgr().Get(BLConfig::kConfigKey_WiFiPassword, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    SuccessOrExit(err);

    mSavedNetwork.credentialsLen                            = credentialsLen;
    mSavedNetwork.ssidLen                                   = ssidLen;
    mSavedNetwork.credentials[mSavedNetwork.credentialsLen] = '\0';
    mSavedNetwork.ssid[mSavedNetwork.ssidLen]               = '\0';

    mStagingNetwork        = mSavedNetwork;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;
    mScanSpecific          = false;

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
    wifiInterface_connect((char *) ssid, (char *) key);
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
    ChipLogProgress(NetworkProvisioning, "BL702 Network Commissioning ConnectNetwork: SSID: %.*s",
                    static_cast<int>(networkId.size()), networkId.data());

    mStagingNetwork.ssid[mStagingNetwork.ssidLen]               = '\0';
    mStagingNetwork.credentials[mStagingNetwork.credentialsLen] = '\0';
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

void BLWiFiDriver::OnScanWiFiNetworkDone(void * arg)
{
    netbus_fs_scan_ind_cmd_msg_t * pmsg = (netbus_fs_scan_ind_cmd_msg_t *) arg;
    size_t i = 0, ap_num = pmsg->num;
    WiFiScanResponse *pScanResponse, *p;

    for (i = 0; i < pmsg->num; i++)
    {
        if (mScanSpecific && !strcmp(mScanSSID, (char *) (pmsg->records[i].ssid)))
        {
            ap_num = 1;
            break;
        }
    }

    if (0 == ap_num || (mScanSpecific && ap_num == pmsg->num))
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }

    p = pScanResponse = (WiFiScanResponse *) malloc(sizeof(WiFiScanResponse) * ap_num);
    for (i = 0; i < pmsg->num; i++)
    {
        if (mScanSpecific && strcmp(mScanSSID, (char *) (pmsg->records[i].ssid)))
        {
            continue;
        }

        p->security.SetRaw(pmsg->records[i].auth_mode);
        p->ssidLen = strlen((char *) pmsg->records[i].ssid) < chip::DeviceLayer::Internal::kMaxWiFiSSIDLength
            ? strlen((char *) pmsg->records[i].ssid)
            : chip::DeviceLayer::Internal::kMaxWiFiSSIDLength;
        p->channel  = pmsg->records[i].channel;
        p->wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
        p->rssi     = pmsg->records[i].rssi;
        memcpy(p->ssid, pmsg->records[i].ssid, p->ssidLen);
        memcpy(p->bssid, pmsg->records[i].bssid, 6);

        if (mScanSpecific)
        {
            break;
        }

        p++;
    }

    if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_num, pScanResponse]() {
            BLScanResponseIterator iter(ap_num, pScanResponse);
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

    free(pScanResponse);
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
        mpScanCallback = callback;
        wifiInterface_startScan();
    }
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    struct bflbwifi_ap_record ap_info;

    ChipLogProgress(DeviceLayer, "GetConfiguredNetwork");

    if (!wifiInterface_getApInfo(&ap_info))
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint8_t length = strnlen(reinterpret_cast<const char *>(ap_info.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, ap_info.ssid, length);
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

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
//#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
