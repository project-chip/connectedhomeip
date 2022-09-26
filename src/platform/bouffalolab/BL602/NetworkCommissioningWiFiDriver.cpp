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
#include <wifi_mgmr_ext.h>

#include <limits>
#include <string>

using namespace ::chip;
// #if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";

constexpr char blWiFiSSIDKeyName[]        = "bl-wifi-ssid";
constexpr char blWiFiCredentialsKeyName[] = "bl-wifi-pass";

static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
} // namespace

CHIP_ERROR BLWiFiDriver::Init()
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

    mStagingNetwork   = mSavedNetwork;
    mpScanCallback    = nullptr;
    mpConnectCallback = nullptr;

    return err;
}

void BLWiFiDriver::Shutdown() {}

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

Status BLWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials)
{
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    return Status::kSuccess;
}

Status BLWiFiDriver::RemoveNetwork(ByteSpan networkId)
{
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status BLWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index)
{
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
    // Set the wifi configuration
    memcpy(wifi_ssid, ssid, ssidLen);
    memcpy(passwd, key, keyLen);
    wifi_interface_t wifi_interface;
    wifi_interface = wifi_mgmr_sta_enable();
    wifi_mgmr_sta_connect(wifi_interface, ssid, passwd, NULL, NULL, 0, 0);

    // Configure the WiFi interface.
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

    // VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    // VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
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

#if 0
CHIP_ERROR ESPWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    esp_err_t err = ESP_OK;
    if (!ssid.empty())
    {
        wifi_scan_config_t scan_config = { 0 };
        memset(WiFiSSIDStr, 0, sizeof(WiFiSSIDStr));
        memcpy(WiFiSSIDStr, ssid.data(), ssid.size());
        scan_config.ssid = WiFiSSIDStr;
        err              = esp_wifi_scan_start(&scan_config, false);
    }
    else
    {
        err = esp_wifi_scan_start(NULL, false);
    }
    if (err != ESP_OK)
    {
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }
    return CHIP_NO_ERROR;
}
#endif

#if 0
void ESPWiFiDriver::OnScanWiFiNetworkDone()
{
    uint16_t ap_number;
    esp_wifi_scan_get_ap_num(&ap_number);
    if (!ap_number)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }
    std::unique_ptr<wifi_ap_record_t[]> ap_buffer_ptr(new wifi_ap_record_t[ap_number]);
    if (ap_buffer_ptr == NULL)
    {
        ChipLogError(DeviceLayer, "can't malloc memory for ap_list_buffer");
        if (mpScanCallback)
        {
            mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }
    wifi_ap_record_t * ap_list_buffer = ap_buffer_ptr.get();
    if (esp_wifi_scan_get_ap_records(&ap_number, ap_list_buffer) == ESP_OK)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_number, ap_list_buffer]() {
                std::unique_ptr<wifi_ap_record_t[]> auto_free(ap_list_buffer);
                ESPScanResponseIterator iter(ap_number, ap_list_buffer);
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
            ap_buffer_ptr.release();
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
#endif

void BLWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
#if 0
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (StartScanWiFiNetworks(ssid) != CHIP_NO_ERROR)
        {
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
#endif
}

#if 0
CHIP_ERROR GetConnectedNetwork(Network & network)
{
    wifi_ap_record_t ap_info;
    esp_err_t err;
    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err != ESP_OK)
    {
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }
    uint8_t length = strnlen(reinterpret_cast<const char *>(ap_info.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, ap_info.ssid, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}
#endif

size_t BLWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool BLWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
#if 0
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = false;
    mExhausted        = true;

    Network connectedNetwork;
    CHIP_ERROR err = GetConnectedNetwork(connectedNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (connectedNetwork.networkIDLen == item.networkIDLen &&
            memcmp(connectedNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }
#endif
    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
// #endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
