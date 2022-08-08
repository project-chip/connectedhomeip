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
#include <platform/EFR32/EFR32Config.h>
#include <platform/EFR32/NetworkCommissioningWiFiDriver.h>

#include <limits>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
NetworkCommissioning::WiFiScanResponse * sScanResult;
SlScanResponseIterator<NetworkCommissioning::WiFiScanResponse> mScanResponseIter(sScanResult);
} // namespace

CHIP_ERROR SlWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;
    mpScanCallback        = nullptr;
    mpConnectCallback     = nullptr;

    // If reading fails, wifi is not provisioned, no need to go further.
    err =
        EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), ssidLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = EFR32Config::ReadConfigValueStr(EFR32Config::kConfigKey_WiFiPSK, mSavedNetwork.credentials,
                                          sizeof(mSavedNetwork.credentials), credentialsLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;
    mStagingNetwork              = mSavedNetwork;

    ConnectWiFiNetwork(mSavedNetwork.ssid, ssidLen, mSavedNetwork.credentials, credentialsLen);
    return err;
}

CHIP_ERROR SlWiFiDriver::CommitConfiguration()
{
    uint8_t securityType = WFX_SEC_WPA2;

    ReturnErrorOnFailure(EFR32Config::WriteConfigValueStr(EFR32Config::kConfigKey_WiFiSSID, mStagingNetwork.ssid));
    ReturnErrorOnFailure(EFR32Config::WriteConfigValueStr(EFR32Config::kConfigKey_WiFiPSK, mStagingNetwork.credentials));
    ReturnErrorOnFailure(EFR32Config::WriteConfigValueBin(EFR32Config::kConfigKey_WiFiSEC, &securityType, sizeof(securityType)));

    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SlWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool SlWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status SlWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                        uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memset(mStagingNetwork.credentials, 0, sizeof(mStagingNetwork.credentials));
    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memset(mStagingNetwork.ssid, 0, sizeof(mStagingNetwork.ssid));
    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    return Status::kSuccess;
}

Status SlWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status SlWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR SlWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Disconecting for current wifi");
        int32_t status = wfx_sta_discon();
        if (status != 0)
        {
            return CHIP_ERROR_INTERNAL;
        }
    }
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    // Set the wifi configuration
    wfx_wifi_provision_t wifiConfig = {};
    memcpy(wifiConfig.ssid, ssid, ssidLen);
    memcpy(wifiConfig.passkey, key, keyLen);
    wifiConfig.security = WFX_SEC_WPA_WPA2_MIXED;

    ChipLogProgress(NetworkProvisioning, "Setting up connection for WiFi SSID: %.*s", static_cast<int>(ssidLen), ssid);
    // Configure the WFX WiFi interface.
    wfx_set_wifi_provision(&wifiConfig);
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));
    return CHIP_NO_ERROR;
}

void SlWiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
    {
        CommitConfiguration();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void SlWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kUnknownError;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);

    err = ConnectWiFiNetwork(mStagingNetwork.ssid, mStagingNetwork.ssidLen, mStagingNetwork.credentials,
                             mStagingNetwork.credentialsLen);
    if (err == CHIP_NO_ERROR)
    {
        mpConnectCallback = callback;
        networkingStatus  = Status::kSuccess;
    }

exit:
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network:%s", chip::ErrorStr(err));
        mpConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

chip::BitFlags<WiFiSecurity> SlWiFiDriver::ConvertSecuritytype(uint8_t security)
{
    chip::BitFlags<WiFiSecurity> securityType;
    if (security == WFX_SEC_NONE)
    {
        securityType = WiFiSecurity::kUnencrypted;
    }
    else if (security & WFX_SEC_WEP)
    {
        securityType = WiFiSecurity::kWepPersonal;
    }
    else if (security & WFX_SEC_WPA)
    {
        securityType = WiFiSecurity::kWpaPersonal;
    }
    else if (security & WFX_SEC_WPA2)
    {
        securityType = WiFiSecurity::kWpa2Personal;
    }
    else if (security == WFX_SEC_WPA3)
    {
        securityType = WiFiSecurity::kWpa3Personal;
    }
    else
    {
        // wfx_sec_t support more type
        securityType = WiFiSecurity::kUnencrypted;
    }

    return securityType;
}

bool SlWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    bool scanStarted = false;
    ChipLogProgress(DeviceLayer, "Start Scan WiFi Networks");
    if (!ssid.empty()) // ssid is given, only scan this network
    {
        char cSsid[DeviceLayer::Internal::kMaxWiFiSSIDLength] = {};
        memcpy(cSsid, ssid.data(), ssid.size());
        scanStarted = wfx_start_scan(cSsid, OnScanWiFiNetworkDone);
    }
    else // scan all networks
    {
        scanStarted = wfx_start_scan(nullptr, OnScanWiFiNetworkDone);
    }
    return scanStarted;
}

void SlWiFiDriver::OnScanWiFiNetworkDone(wfx_wifi_scan_result_t * aScanResult)
{
    ChipLogProgress(DeviceLayer, "OnScanWiFiNetworkDone");
    if (!aScanResult)
    {
        if (GetInstance().mpScanCallback != nullptr)
        {
            DeviceLayer::SystemLayer().ScheduleLambda([]() {
                GetInstance().mpScanCallback->OnFinished(NetworkCommissioning::Status::kSuccess, CharSpan(), &mScanResponseIter);
                GetInstance().mpScanCallback = nullptr;
            });
        }
    }
    else
    {
        NetworkCommissioning::WiFiScanResponse scanResponse = {};

        scanResponse.security.Set(GetInstance().ConvertSecuritytype(aScanResult->security));
        scanResponse.channel = aScanResult->chan;
        scanResponse.rssi    = aScanResult->rssi;
        scanResponse.ssidLen = strnlen(aScanResult->ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
        memcpy(scanResponse.ssid, aScanResult->ssid, scanResponse.ssidLen);
        memcpy(scanResponse.bssid, aScanResult->bssid, sizeof(scanResponse.bssid));

        mScanResponseIter.Add(&scanResponse);
    }
}

void SlWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (!StartScanWiFiNetworks(ssid))
        {
            ChipLogError(DeviceLayer, "ScanWiFiNetworks failed to start");
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    wfx_wifi_provision_t wifiConfig;

    if (!wfx_is_sta_connected() || !wfx_get_wifi_provision(&wifiConfig))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    uint8_t length = strnlen(wifiConfig.ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, wifiConfig.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

size_t SlWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool SlWiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
