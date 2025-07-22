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
#include <platform/senscomm/scm1612s/SCM1612SConfig.h>
#include <platform/senscomm/scm1612s/NetworkCommissioningWiFiDriver.h>

#include <limits>

#include "wise_event_loop.h"
#include "wise_wifi_types.h"
#include "wise_err.h"
#include "scm_wifi.h"

#define SECURITY_OPEN 0
#define SECURITY_TKIP 2
#define SECURITY_CCMP 3
#define SECURITY_CCMP_256 4
#define SECURITY_SAE  6

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

CHIP_ERROR WiseWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;
    size_t outLen         = 0;
    mpScanCallback        = nullptr;
    mpConnectCallback     = nullptr;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::Init");


#if 0
    err = SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, "ax3");
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);
    err = SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, "12345678");
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);
    uint8_t auth = 2;
    err = SCM1612SConfig::WriteConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &auth, sizeof(auth));
#endif

    // If reading fails, wifi is not provisioned, no need to go further.
    err = SCM1612SConfig::ReadConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                           ssidLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = SCM1612SConfig::ReadConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, mSavedNetwork.credentials,
                                           sizeof(mSavedNetwork.credentials), credentialsLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = SCM1612SConfig::ReadConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &mSavedNetwork.auth_mode,
                                           sizeof(mSavedNetwork.auth_mode), outLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;
    mStagingNetwork              = mSavedNetwork;

    ConnectWiFiNetwork(mSavedNetwork.ssid, ssidLen, mSavedNetwork.credentials, credentialsLen);
    return err;
}

CHIP_ERROR WiseWiFiDriver::CommitConfiguration()
{
    CHIP_ERROR err;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::CommitConfiguration");

    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, mStagingNetwork.ssid));

    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, mStagingNetwork.credentials));

    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &mStagingNetwork.auth_mode,
                                                           sizeof(mStagingNetwork.auth_mode)));

    mSavedNetwork = mStagingNetwork;

    return CHIP_NO_ERROR;
}

void WiseWiFiDriver::UpdateWiFiAuthmode()
{
    wifi_config_t config;
    wifi_auth_mode_t authmode = WIFI_AUTH_OPEN;
    scm_wifi_get_config(WIFI_IF_STA, &config);

    switch (config.sta.proto)
    {
        case WIFI_PROTO_WPA:
            authmode = WIFI_AUTH_WPA_PSK;
            break;
        case WIFI_PROTO_WPA2:
            switch (config.sta.alg)
            {
                case SECURITY_CCMP:
                    authmode = WIFI_AUTH_WPA2_PSK;
                    break;
                case SECURITY_SAE:
                    authmode = WIFI_AUTH_WPA3_SAE;
                    break;
                default:
                    authmode = WIFI_AUTH_OPEN;
                    break;
            }
            break;
        default:
            authmode = WIFI_AUTH_OPEN;
            break;
    }

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::UpdateWiFiAuthmode, authmode %d %d", 
                    mStagingNetwork.auth_mode, authmode);

    if (mStagingNetwork.auth_mode != authmode)
    {
        mStagingNetwork.auth_mode = authmode;
        CommitConfiguration();
    }
}

CHIP_ERROR WiseWiFiDriver::RevertConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::RevertConfiguration");

    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool WiseWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status WiseWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::AddOrUpdateNetwork");

    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memset(mStagingNetwork.credentials, 0, sizeof(mStagingNetwork.credentials));
    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memset(mStagingNetwork.ssid, 0, sizeof(mStagingNetwork.ssid));
    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    mStagingNetwork.auth_mode = credentials.size() != 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

    return Status::kSuccess;
}

Status WiseWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::RemoveNetwork");

    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status WiseWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ReorderNetwork");

    outDebugText.reduce_size(0);
    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR WiseWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    scm_wifi_assoc_request req = {0};
    int ret;
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ConnectWiFiNetwork");

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    memcpy(req.ssid, ssid, ssidLen);
    memcpy(req.key, key, keyLen);
    switch (mStagingNetwork.auth_mode)
    {
        case WIFI_AUTH_WPA3_SAE:
            req.auth = SCM_WIFI_SECURITY_SAE;
            break;
        case WIFI_AUTH_WPA2_PSK:
            req.auth = SCM_WIFI_SECURITY_WPA2PSK;
            break;
        case WIFI_AUTH_OPEN:
            req.auth = SCM_WIFI_SECURITY_OPEN;
            break;
        case SCM_WIFI_SECURITY_UNKNOWN:
        default:
            req.auth = SCM_WIFI_SECURITY_WPA2PSK;
            break;
    }
    req.pairwise = SCM_WIFI_PAIRWISE_AES;

    scm_wifi_sta_set_config(&req, NULL);
#if 0
    ChipLogProgress(NetworkProvisioning, "ssid: %s %d", ssid, ssidLen);
    ChipLogProgress(NetworkProvisioning, "key: %s %d", key, keyLen);
    ChipLogProgress(NetworkProvisioning, "auth: %d", mStagingNetwork.auth_mode);
#endif

    ChipLogProgress(NetworkProvisioning, "Setting up connection for WiFi SSID: %.*s", static_cast<int>(ssidLen), ssid);

    //ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    //ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
    ret = scm_wifi_sta_connect();
    if (ret != WISE_OK)
    {
        ChipLogError(DeviceLayer, "scm_wifi_connect() failed");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void WiseWiFiDriver::OnConnectWiFiNetwork()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::OnConnectWiFiNetwork");

    if (mpConnectCallback)
    {
        CommitConfiguration();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

CHIP_ERROR WiseWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    return CHIP_NO_ERROR;
}

void WiseWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kUnknownError;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ConnectNetwork");

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

chip::BitFlags<WiFiSecurityBitmap> ConvertSecurityType(scm_wifi_auth_mode auth_mode)
{
    chip::BitFlags<WiFiSecurityBitmap> securityType;
    if (auth_mode == SCM_WIFI_SECURITY_OPEN)
    {
        securityType = WiFiSecurity::kUnencrypted;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_WPAPSK)
    {
        securityType = WiFiSecurity::kWpaPersonal;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_WPA2PSK)
    {
        securityType = WiFiSecurity::kWpa2Personal;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_SAE)
    {
        securityType = WiFiSecurity::kWpa3Personal;
    }
    else
    {
        securityType = WiFiSecurity::kUnencrypted;
    }

    return securityType;
}

bool WiseWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::StartScanWiFiNetworks");

    ChipLogProgress(DeviceLayer, "Start Scan WiFi Networks");

    if (!ssid.empty()) // ssid is given, only scan this network
    {
        scm_wifi_scan_params sp = {0};
        sp.scan_type = SCM_WIFI_SSID_SCAN;
        sp.ssid_len = ssid.size();
        memcpy(sp.ssid, ssid.data(), ssid.size());
        scm_wifi_sta_advance_scan(&sp);
    }
    else // scan all networks
    {
        scm_wifi_sta_scan();
    }

    return true;
}

void WiseWiFiDriver::OnScanWiFiNetworkDone()
{
    if (!GetInstance().mpScanCallback)
    {
        ChipLogProgress(DeviceLayer, "No scan callback");
        return;
    }
    uint16_t ap_number = 32, num;

    // Since this is the dynamic memory allocation, restrict it to a configured limit
    ap_number = std::min(static_cast<uint16_t>(CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS), ap_number);

    std::unique_ptr<scm_wifi_ap_info[]> ap_buffer_ptr(new scm_wifi_ap_info[ap_number]);
    if (ap_buffer_ptr == NULL)
    {
        ChipLogError(DeviceLayer, "can't malloc memory for ap_list_buffer");
        GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        GetInstance().mpScanCallback = nullptr;
        return;
    }
    scm_wifi_ap_info * ap_list_buffer = ap_buffer_ptr.get();
    if (scm_wifi_sta_scan_results(ap_list_buffer, &num, ap_number) == WISE_OK)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_number, ap_list_buffer]() {
                std::unique_ptr<scm_wifi_ap_info[]> auto_free(ap_list_buffer);
                WiseScanResponseIterator iter(ap_number, ap_list_buffer);
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
        else
        {
            ChipLogError(DeviceLayer, "can't schedule the scan result processing");
            GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            GetInstance().mpScanCallback = nullptr;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "can't get ap_records ");
        GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        GetInstance().mpScanCallback = nullptr;
    }
}

void WiseWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ScanNetworks");

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

    wifi_config_t stationConfig;
    scm_wifi_get_config(WIFI_IF_STA, &stationConfig);
    uint8_t length = strnlen(reinterpret_cast<const char*>(stationConfig.sta.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, stationConfig.sta.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

size_t WiseWiFiDriver::WiFiNetworkIterator::Count()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::WiFiNetworkIterator::Count");

    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool WiseWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::WiFiNetworkIterator::Next");

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
