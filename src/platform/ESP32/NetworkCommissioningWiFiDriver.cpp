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

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/ESP32/ESP32Utils.h>

#include "esp_wifi.h"

#include <string>
#include <limits>

using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
}

CHIP_ERROR ESPWiFiDriver::Init()
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

CHIP_ERROR ESPWiFiDriver::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool ESPWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status ESPWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials)
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

Status ESPWiFiDriver::RemoveNetwork(ByteSpan networkId)
{
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status ESPWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index)
{
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    wifi_config_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, ssid, std::min(ssidLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.ssid))));
    memcpy(wifiConfig.sta.password, key, std::min(keyLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.password))));
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

    // Configure the ESP WiFi interface.
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

void ESPWiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
   {
       mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
       mpConnectCallback = nullptr;
   }
}

void ESPWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "ESP NetworkCommissioningDelegate: SSID: %s", networkId.data());

    err = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
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

CHIP_ERROR ESPWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    esp_err_t err = ESP_OK;
    if (ssid.data())
    {
        wifi_scan_config_t scan_config = { 0 };
        memset(WiFiSSIDStr, 0, sizeof(WiFiSSIDStr));
        memcpy(WiFiSSIDStr, ssid.data(), ssid.size());
        scan_config.ssid = WiFiSSIDStr;
        err = esp_wifi_scan_start(&scan_config, false);
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

bool GetWiFiScanRespFromAPRecord(wifi_ap_record_t * ap_record, WiFiScanResponse & scanResponse)
{
    scanResponse.security = ap_record->authmode;
    scanResponse.ssidLen = strnlen((const char *)ap_record->ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
    memcpy((char *)scanResponse.ssid, ap_record->ssid, scanResponse.ssidLen);
    memcpy(scanResponse.bssid, ap_record->bssid, 6);
    scanResponse.channel = ap_record->primary;
    scanResponse.wiFiBand = DeviceLayer::NetworkCommissioning::WiFiBand::k2g4; // ESP32 platform only support 2.4G Hz WiFi Band
    scanResponse.rssi = ap_record->rssi;
    return true;
}

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
    if (ap_number > kMaxScanResultsNum)
    {
        ap_number = kMaxScanResultsNum;
    }
    wifi_ap_record_t * ap_list_buffer;
    ap_list_buffer = (wifi_ap_record_t *)malloc(ap_number * sizeof(wifi_ap_record_t));
    if (ap_list_buffer == NULL)
    {
        ChipLogError(DeviceLayer, "can't malloc memory for ap_list_buffer");
        if (mpScanCallback)
        {
            mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }
    if (esp_wifi_scan_get_ap_records(&ap_number, (wifi_ap_record_t *)ap_list_buffer) == ESP_OK)
    {
        ESPScanResponseIterator<WiFiScanResponse> * iter = new ESPScanResponseIterator<WiFiScanResponse>();
        WiFiScanResponse scanResponse;
        for (uint8_t idx = 0; idx < ap_number; idx++)
        {
            GetWiFiScanRespFromAPRecord(&ap_list_buffer[idx], scanResponse);
            iter->Add(scanResponse);
        }
        DeviceLayer::SystemLayer().ScheduleLambda([iter]() {
            if (GetInstance().mpScanCallback)
            {
                GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(),
                                                  const_cast<ESPScanResponseIterator<WiFiScanResponse> *>(iter));
                GetInstance().mpScanCallback = nullptr;
                delete const_cast<ESPScanResponseIterator<WiFiScanResponse> *>(iter);
            }
        });
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
    free(ap_list_buffer);
}

void ESPWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    CHIP_ERROR err = StartScanWiFiNetworks(ssid);
    mpScanCallback = callback;
    if (err != CHIP_NO_ERROR)
    {
        mpScanCallback = nullptr;
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    wifi_ap_record_t ap_info;
    esp_err_t err;
    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err != ESP_OK)
    {
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }
    uint8_t length = strlen((const char *)(ap_info.ssid));
    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, ap_info.ssid, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}

size_t ESPWiFiDriver::WiFiNetworkIterator::Count()
{
    return driver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool ESPWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (exhausted || driver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, driver->mStagingNetwork.ssid, driver->mStagingNetwork.ssidLen);
    item.networkIDLen = driver->mStagingNetwork.ssidLen;
    item.connected    = false;
    exhausted         = true;

    Network connectedNetwork;
    CHIP_ERROR err = GetConnectedNetwork(connectedNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (connectedNetwork.networkIDLen == item.networkIDLen &&
            memcmp(connectedNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected    = true;
        }
    }
    return true;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
