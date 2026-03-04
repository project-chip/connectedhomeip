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

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/BL702/wifi_mgmr_portable.h>
#include <platform/bouffalolab/common/NetworkCommissioningDriver.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

auto converter = [](const WiFiScanResponse & raw) -> WiFiScanResponse { return raw; };

CHIP_ERROR BflbWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    err = PersistedStorage::KeyValueStoreMgr().Get(BflbConfig::kConfigKey_WiFiSSID, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    SuccessOrExit(err);
    err = PersistedStorage::KeyValueStoreMgr().Get(BflbConfig::kConfigKey_WiFiPassword, mSavedNetwork.ssid,
                                                   sizeof(mSavedNetwork.ssid), &ssidLen);
    SuccessOrExit(err);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork = mSavedNetwork;
    mScanSSIDlength = 0;

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
    mScanSSIDlength        = 0;
    memset(mScanSSID, 0, sizeof(mScanSSID));
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
    if (keyLen == BflbWiFiDriver::WiFiCredentialLength::kOpen || keyLen == BflbWiFiDriver::WiFiCredentialLength::kWEP64 ||
        (keyLen >= BflbWiFiDriver::WiFiCredentialLength::kMinWPAPSK && keyLen <= BflbWiFiDriver::WiFiCredentialLength::kMaxWPAPSK))
    {
        wifiInterface_disconnect();
        vTaskDelay(500);
        if (keyLen == BflbWiFiDriver::WiFiCredentialLength::kOpen)
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

void BflbWiFiDriver::OnConnectWiFiNetwork(bool isConnected)
{
    ChipLogProgress(NetworkProvisioning, "BflbWiFiDriver::OnConnectWiFiNetwork, isConnected=%d\r\n", isConnected);
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
        if (ssid.data())
        {
            memset(mScanSSID, 0, sizeof(mScanSSID));
            memcpy(mScanSSID, ssid.data(), ssid.size());
            mScanSSIDlength = ssid.size();
        }

        mScanResponseNum = 0;
        mpScanCallback   = callback;
        wifiInterface_startScan();
    }
}

void BflbWiFiDriver::OnScanWiFiNetworkDone(void * opaque)
{
    netbus_wifi_mgmr_msg_cmd_t * pkg_data = (netbus_wifi_mgmr_msg_cmd_t *) ((struct pkg_protocol *) opaque)->payload;
    netbus_fs_scan_ind_cmd_msg_t * pmsg   = (netbus_fs_scan_ind_cmd_msg_t *) ((netbus_fs_scan_ind_cmd_msg_t *) pkg_data);

    size_t i = 0, ap_num = 0, ap_cnt = 0;
    WiFiScanResponse *pScanResponse, *p;

    if (mScanSSIDlength)
    {
        for (i = 0; i < pmsg->num; i++)
        {
            if (0 == strncmp(mScanSSID, (char *) (pmsg->records[i].ssid), mScanSSIDlength))
            {
                ap_num = 1;
                break;
            }
        }

        ChipLogProgress(DeviceLayer, "Wi-Fi SSID scanned %d in total, %s matched SSID %s", pmsg->num, ap_num ? "with" : "without",
                        mScanSSID);
    }
    else
    {
        ap_num = pmsg->num;
        ChipLogProgress(DeviceLayer, "Wi-Fi SSID scanned %d in total", pmsg->num);
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
            if (mScanSSIDlength && strncmp(mScanSSID, (char *) (pmsg->records[i].ssid), mScanSSIDlength))
            {
                continue;
            }

            p->security.SetRaw(pmsg->records[i].auth_mode);
            strncpy((char *) p->ssid, (const char *) pmsg->records[i].ssid, kMaxWiFiSSIDLength);
            p->ssidLen         = strlen((char *) pmsg->records[i].ssid);
            p->channel         = pmsg->records[i].channel;
            p->wiFiBand        = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
            p->signal.type     = NetworkCommissioning::WirelessSignalType::kdBm;
            p->signal.strength = pmsg->records[i].rssi;
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

    OnScanWiFiNetworkDone();
}

void BflbWiFiDriver::OnScanWiFiNetworkDone(void)
{
    size_t ap_cnt                    = mScanResponseNum;
    WiFiScanResponse * pScanResponse = mScanResponse;

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([ap_cnt, pScanResponse]() {
        if (GetInstance().mpScanCallback)
        {
            BflbScanResponseIterator iter(ap_cnt, pScanResponse, converter);
            GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
        }
        else
        {
            ChipLogError(DeviceLayer, "can't find the ScanCallback function");
        }
    });

    if (mScanResponse)
    {
        free(mScanResponse);
    }
    mScanResponse    = nullptr;
    mScanResponseNum = 0;
    mScanSSIDlength  = 0;
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

void BflbWiFiDriver::OnNetworkStatusChange()
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

CHIP_ERROR BflbWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    // TODO: to be added
    mLastDisconnectedReason = 0;

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
