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
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>

#include <limits>
#include <string>

extern uint32_t apNum;
using namespace ::chip;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

CHIP_ERROR AmebaWiFiDriver::Init()
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

CHIP_ERROR AmebaWiFiDriver::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool AmebaWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status AmebaWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials)
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

Status AmebaWiFiDriver::RemoveNetwork(ByteSpan networkId)
{
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status AmebaWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index)
{
    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR AmebaWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    rtw_wifi_setting_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.ssid, ssid, ssidLen + 1);
    memcpy(wifiConfig.password, key, keyLen + 1);
    wifiConfig.mode = RTW_MODE_STA;

    // Configure the WiFi interface.
    int err = CHIP_SetWiFiConfig(&wifiConfig);
    if (err != 0)
    {
        ChipLogError(DeviceLayer, "_SetWiFiConfig() failed: %d", err);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

void AmebaWiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void AmebaWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "Ameba NetworkCommissioningDelegate: SSID: %s", networkId.data());

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

CHIP_ERROR AmebaWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    if (ssid.data()) // ssid is given, only scan this network
    {
        matter_scan_networks_with_ssid(ssid.data(), ssid.size());
    }
    else // scan all networks
    {
        matter_scan_networks();
    }
    return CHIP_NO_ERROR;
}

void AmebaWiFiDriver::OnScanWiFiNetworkDone()
{
    uint16_t NumAP = apNum;
    apNum          = 0;
    if (!NumAP)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mpScanCallback != nullptr)
        {
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
        return;
    }

    rtw_scan_result_t * ScanResult = (rtw_scan_result *) pvPortMalloc(NumAP * sizeof(rtw_scan_result));
    matter_get_scan_results(ScanResult, NumAP);

    if (ScanResult)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([NumAP, ScanResult]() {
                AmebaScanResponseIterator iter(NumAP, ScanResult);
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
    vPortFree(ScanResult);
}

void AmebaWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
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

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    rtw_wifi_setting_t wifi_setting;
    CHIP_GetWiFiConfig(&wifi_setting);

    uint8_t length = strnlen(reinterpret_cast<const char *>(wifi_setting.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, wifi_setting.ssid, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}

size_t AmebaWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool AmebaWiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
    CHIP_ERROR err = GetConnectedNetwork(connectedNetwork);
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
