/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <platform/mt793x/MT793XConfig.h>
#include <platform/mt793x/NetworkCommissioningWiFiDriver.h>

#include <limits>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
NetworkCommissioning::WiFiScanResponse * sScanResult;
GenioScanResponseIterator<NetworkCommissioning::WiFiScanResponse> mScanResponseIter(sScanResult);

using chip::app::Clusters::NetworkCommissioning::WiFiSecurityBitmap;
} // namespace

CHIP_ERROR GenioWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;
    size_t outLen         = 0;
    mpScanCallback        = nullptr;
    mpConnectCallback     = nullptr;

    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::Init");

    // If reading fails, wifi is not provisioned, no need to go further.
    err = MT793XConfig::ReadConfigValueStr(MT793XConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                           ssidLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = MT793XConfig::ReadConfigValueStr(MT793XConfig::kConfigKey_WiFiPSK, mSavedNetwork.credentials,
                                           sizeof(mSavedNetwork.credentials), credentialsLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = MT793XConfig::ReadConfigValueBin(MT793XConfig::kConfigKey_WiFiSEC, &mSavedNetwork.auth_mode,
                                           sizeof(mSavedNetwork.auth_mode), outLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;
    mStagingNetwork              = mSavedNetwork;

    ConnectWiFiNetwork(mSavedNetwork.ssid, ssidLen, mSavedNetwork.credentials, credentialsLen);
    return err;
}

CHIP_ERROR GenioWiFiDriver::CommitConfiguration()
{
    CHIP_ERROR err;

    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::CommitConfiguration");

    ReturnErrorOnFailure(MT793XConfig::WriteConfigValueStr(MT793XConfig::kConfigKey_WiFiSSID, mStagingNetwork.ssid));

    ReturnErrorOnFailure(MT793XConfig::WriteConfigValueStr(MT793XConfig::kConfigKey_WiFiPSK, mStagingNetwork.credentials));

    ReturnErrorOnFailure(MT793XConfig::WriteConfigValueBin(MT793XConfig::kConfigKey_WiFiSEC, &mStagingNetwork.auth_mode,
                                                           sizeof(mStagingNetwork.auth_mode)));

    mSavedNetwork = mStagingNetwork;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenioWiFiDriver::RevertConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::RevertConfiguration");

    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool GenioWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status GenioWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::AddOrUpdateNetwork");

    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memset(mStagingNetwork.credentials, 0, sizeof(mStagingNetwork.credentials));
    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memset(mStagingNetwork.ssid, 0, sizeof(mStagingNetwork.ssid));
    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    mStagingNetwork.auth_mode = WIFI_AUTH_MODE_WPA2_PSK;

    return Status::kSuccess;
}

Status GenioWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::RemoveNetwork");

    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status GenioWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::ReorderNetwork");

    outDebugText.reduce_size(0);
    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR GenioWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::ConnectWiFiNetwork");

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    // Set the wifi configuration
    filogic_wifi_sta_prov_t wifi_prov = {};
    memcpy(wifi_prov.ssid, ssid, ssidLen);
    memcpy(wifi_prov.psk, key, keyLen);
    wifi_prov.ssid_len  = ssidLen;
    wifi_prov.psk_len   = keyLen;
    wifi_prov.auth_mode = WIFI_AUTH_MODE_WPA2_PSK;

    ChipLogProgress(NetworkProvisioning, "Setting up connection for WiFi SSID: %.*s", static_cast<int>(ssidLen), ssid);

    void * filogicCtx = PlatformMgrImpl().mFilogicCtx;

    // Configure the FILOGIC WiFi interface.
    filogic_wifi_sta_prov_set_sync(filogicCtx, &wifi_prov);
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    return CHIP_NO_ERROR;
}

void GenioWiFiDriver::OnConnectWiFiNetwork()
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::OnConnectWiFiNetwork");

    if (mpConnectCallback)
    {
        CommitConfiguration();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void GenioWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kUnknownError;

    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::ConnectNetwork");

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

chip::BitFlags<WiFiSecurityBitmap> GenioWiFiDriver::ConvertSecuritytype(wifi_auth_mode_t auth_mode)
{
    chip::BitFlags<WiFiSecurityBitmap> securityType;
    if (auth_mode == WIFI_AUTH_MODE_OPEN)
    {
        securityType = WiFiSecurity::kUnencrypted;
    }
    else if (auth_mode == WIFI_AUTH_MODE_WPA_PSK)
    {
        securityType = WiFiSecurity::kWpaPersonal;
    }
    else if (auth_mode == WIFI_AUTH_MODE_WPA2_PSK)
    {
        securityType = WiFiSecurity::kWpa2Personal;
    }
    else if (auth_mode == WIFI_AUTH_MODE_WPA3_PSK)
    {
        securityType = WiFiSecurity::kWpa3Personal;
    }
    else
    {
        securityType = WiFiSecurity::kUnencrypted;
    }

    return securityType;
}

bool GenioWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::StartScanWiFiNetworks");

    ChipLogProgress(DeviceLayer, "Start Scan WiFi Networks");

    void * filogicCtx = PlatformMgrImpl().mFilogicCtx;

    if (!ssid.empty()) // ssid is given, only scan this network
    {
        char cSsid[DeviceLayer::Internal::kMaxWiFiSSIDLength] = {};
        memcpy(cSsid, ssid.data(), ssid.size());
        filogic_wifi_scan(filogicCtx, (uint8_t *) cSsid, ssid.size(), kMaxWiFiScanAPs, OnScanWiFiNetworkDone);
    }
    else // scan all networks
    {
        filogic_wifi_scan(filogicCtx, nullptr, 0, kMaxWiFiScanAPs, OnScanWiFiNetworkDone);
    }
    return true;
}

void GenioWiFiDriver::OnScanWiFiNetworkDone(wifi_scan_list_item_t * aScanResult)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::OnScanWiFiNetworkDone");

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
        while (aScanResult->is_valid)
        {
            NetworkCommissioning::WiFiScanResponse scanResponse = {};
            chip::BitFlags<WiFiSecurityBitmap> security;

            security = GetInstance().ConvertSecuritytype(aScanResult->auth_mode);

            scanResponse.security.Set(security);
            scanResponse.channel = aScanResult->channel;
            scanResponse.rssi    = aScanResult->rssi;
            scanResponse.ssidLen = strnlen((char *) aScanResult->ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
            memcpy(scanResponse.ssid, aScanResult->ssid, scanResponse.ssidLen);
            memcpy(scanResponse.bssid, aScanResult->bssid, sizeof(scanResponse.bssid));

            mScanResponseIter.Add(&scanResponse);

            aScanResult++; // process next result
        }
    }
}

void GenioWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::ScanNetworks");

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
    ChipLogProgress(NetworkProvisioning, "GetConnectedNetwork");

    void * filogicCtx = PlatformMgrImpl().mFilogicCtx;

    filogic_wifi_sta_prov_t wifi_prov;

    if (!filogic_wifi_sta_get_link_status_sync(filogicCtx) || !filogic_wifi_sta_prov_get_sync(filogicCtx, &wifi_prov))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    uint8_t length = strnlen(wifi_prov.ssid, DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, wifi_prov.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

size_t GenioWiFiDriver::WiFiNetworkIterator::Count()
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::WiFiNetworkIterator::Count");

    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool GenioWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    ChipLogProgress(NetworkProvisioning, "GenioWiFiDriver::WiFiNetworkIterator::Next");

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
