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

#include <aos/yloop.h>
#include <hal_wifi.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/bouffalolab/BL602/NetworkCommissioningDriver.h>
#include <tcpip.h>
#include <wifi_mgmr.h>
#include <wifi_mgmr_api.h>
#include <wifi_mgmr_ext.h>
#include <wifi_mgmr_portable.h>

#include <limits>
#include <stdint.h>
#include <string>
#include <utils_log.h>

#define WIFI_STA_DISCONNECT_DELAY (pdMS_TO_TICKS(200))

using namespace ::chip;
//#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";

constexpr char blWiFiSSIDKeyName[]        = "bl-wifi-ssid";
constexpr char blWiFiCredentialsKeyName[] = "bl-wifi-pass";

static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
static uint8_t scan_type = 0;
} // namespace

CHIP_ERROR BLWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), &ssidLen);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork        = mSavedNetwork;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    return err;
}

void BLWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR BLWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLWiFiDriver::SaveConfiguration()
{
    if (NULL == mStagingNetwork.ssid || 0 == mStagingNetwork.ssidLen || NULL == mStagingNetwork.credentials ||
        0 == mStagingNetwork.credentialsLen)
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    ReturnErrorOnFailure(
        PersistedStorage::KeyValueStoreMgr().Put(blWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(blWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
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
    // ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    char wifi_ssid[64] = { 0 };
    char passwd[64]    = { 0 };
    int state          = 0;

    wifi_mgmr_sta_disconnect();
    vTaskDelay(WIFI_STA_DISCONNECT_DELAY);

    wifi_mgmr_sta_disable(NULL);
    wifi_mgmr_state_get(&state);
    while (state != WIFI_STATE_IDLE)
    {
        wifi_mgmr_state_get(&state);
        vTaskDelay(100);
    }

    memcpy(wifi_ssid, ssid, ssidLen);
    memcpy(passwd, key, keyLen);
    wifi_interface_t wifi_interface;
    wifi_interface = wifi_mgmr_sta_enable();
    wifi_mgmr_sta_connect(wifi_interface, wifi_ssid, passwd, NULL, NULL, 0, 0);

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

CHIP_ERROR BLWiFiDriver::ReConnectWiFiNetwork(void)
{
    char ssid[64]  = { 0 };
    char psk[64]   = { 0 };
    size_t ssidLen = 0;
    size_t pskLen  = 0;

    ReturnErrorOnFailure(
        PersistedStorage::KeyValueStoreMgr().Get((const char *) blWiFiSSIDKeyName, (void *) ssid, 64, &ssidLen, 0));
    ReturnErrorOnFailure(
        PersistedStorage::KeyValueStoreMgr().Get((const char *) blWiFiCredentialsKeyName, (void *) psk, 64, &pskLen, 0));

    ConnectWiFiNetwork(ssid, ssidLen, psk, pskLen);

    return CHIP_NO_ERROR;
}

void BLWiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void BLWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;
    static int save         = 0;

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

CHIP_ERROR BLWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!ssid.empty())
    {
        memset(WiFiSSIDStr, 0, sizeof(WiFiSSIDStr));
        memcpy(WiFiSSIDStr, ssid.data(), ssid.size());
        err       = (CHIP_ERROR) wifi_mgmr_scan_adv(NULL, NULL, NULL, 0, NULL, WiFiSSIDStr, 1, 0);
        scan_type = 1;
    }
    else
    {
        err       = (CHIP_ERROR) wifi_mgmr_scan(NULL, NULL);
        scan_type = 0;
    }
    if (err != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

void BLWiFiDriver::OnScanWiFiNetworkDone()
{
    int ap_num = 0;

    ap_num = wifi_mgmr_get_scan_ap_num();
    if (!ap_num)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }

    wifi_mgmr_ap_item_t * ScanResult = (wifi_mgmr_ap_item_t *) pvPortMalloc(ap_num * sizeof(wifi_mgmr_ap_item_t));
    wifi_mgmr_get_scan_result(ScanResult, &ap_num, scan_type, WiFiSSIDStr);

    if (ScanResult)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_num, ScanResult]() {
                BLScanResponseIterator iter(ap_num, ScanResult);
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
            vPortFree(ScanResult);
            ChipLogProgress(DeviceLayer, "ScheduleLambda OK");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "can't get ap_records ");
        if (mpScanCallback)
        {
            mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
    }
}

void BLWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (StartScanWiFiNetworks(ssid) != CHIP_NO_ERROR)
        {
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
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

void BLWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staConnected = false;
    // VerifyOrReturn(ESP32Utils::IsStationEnabled(staEnabled) == CHIP_NO_ERROR);
    VerifyOrReturn(mpStatusChangeCallback != nullptr);
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    if (ConnectivityManagerImpl::mWiFiStationState == ConnectivityManager::kWiFiStationState_Connected)
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
        Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
        MakeOptional(GetLastDisconnectReason()));
}

CHIP_ERROR BLWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    // VerifyOrReturnError(event->Type == DeviceEventType::kRtkWiFiStationDisconnectedEvent, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t status_code, reason_code;

    wifi_mgmr_conn_result_get(&status_code, &reason_code);
    mLastDisconnectedReason = reason_code;

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

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
//#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
