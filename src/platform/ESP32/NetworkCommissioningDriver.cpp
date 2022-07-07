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
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

#include "esp_wifi.h"

#include <limits>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
} // namespace

CHIP_ERROR ESPWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
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

void ESPWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
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

Status ESPWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
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

Status ESPWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status ESPWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    // If device is already connected to WiFi, then disconnect the WiFi,
    // clear the WiFi configurations and add the newly provided WiFi configurations.
    if (chip::DeviceLayer::Internal::ESP32Utils::IsStationProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
        esp_err_t err = esp_wifi_disconnect();
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_disconnect() failed: %s", esp_err_to_name(err));
            return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
        }
        CHIP_ERROR error = chip::DeviceLayer::Internal::ESP32Utils::ClearWiFiStationProvision();
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %s", chip::ErrorStr(error));
            return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
        }
    }

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    wifi_config_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, ssid, std::min(ssidLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.ssid))));
    memcpy(wifiConfig.sta.password, key, std::min(keyLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.password))));

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
        DeviceLayer::SystemLayer().CancelTimer(OnConnectWiFiNetworkFailed, NULL);
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void ESPWiFiDriver::OnConnectWiFiNetworkFailed()
{
    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(Status::kNetworkNotFound, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void ESPWiFiDriver::OnConnectWiFiNetworkFailed(chip::System::Layer * aLayer, void * aAppState)
{
    CHIP_ERROR error = chip::DeviceLayer::Internal::ESP32Utils::ClearWiFiStationProvision();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %s", chip::ErrorStr(error));
    }
    ESPWiFiDriver::GetInstance().OnConnectWiFiNetworkFailed();
}

void ESPWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    Status networkingStatus     = Status::kSuccess;
    const uint32_t secToMiliSec = 1000;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "ESP NetworkCommissioningDelegate: SSID: %.*s", static_cast<int>(networkId.size()),
                    networkId.data());

    err = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                             reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);

    err = DeviceLayer::SystemLayer().StartTimer(
        static_cast<System::Clock::Timeout>(kWiFiConnectNetworkTimeoutSeconds * secToMiliSec), OnConnectWiFiNetworkFailed, NULL);
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

CHIP_ERROR GetConfiguredNetwork(Network & network)
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

void ESPWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staEnabled = false, staConnected = false;
    VerifyOrReturn(ESP32Utils::IsStationEnabled(staEnabled) == CHIP_NO_ERROR);
    VerifyOrReturn(staEnabled && mpStatusChangeCallback != nullptr);
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }
    VerifyOrReturn(ESP32Utils::IsStationConnected(staConnected) == CHIP_NO_ERROR);
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

void ESPWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
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

CHIP_ERROR ESPWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    VerifyOrReturnError(event->Type == DeviceEventType::kESPSystemEvent && event->Platform.ESPSystemEvent.Base == WIFI_EVENT &&
                            event->Platform.ESPSystemEvent.Id == WIFI_EVENT_STA_DISCONNECTED,
                        CHIP_ERROR_INVALID_ARGUMENT);
    mLastDisconnectedReason = event->Platform.ESPSystemEvent.Data.WiFiStaDisconnected.reason;
    return CHIP_NO_ERROR;
}

int32_t ESPWiFiDriver::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t ESPWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool ESPWiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
        bool isConnected = false;
        err              = ESP32Utils::IsStationConnected(isConnected);
        if (err == CHIP_NO_ERROR && isConnected && configuredNetwork.networkIDLen == item.networkIDLen &&
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
