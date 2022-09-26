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
#include <platform/Infineon/PSOC6/NetworkCommissioningDriver.h>
#include <platform/Infineon/PSOC6/P6Config.h>
#include <platform/Infineon/PSOC6/P6Utils.h>

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
} // namespace

cy_wcm_scan_result_t scan_result_list[kWiFiMaxNetworks];
uint8_t NumAP; // no of network scanned

CHIP_ERROR P6WiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen         = 0;
    size_t credentialsLen  = 0;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), &ssidLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;
    mStagingNetwork              = mSavedNetwork;

    return err;
}

void P6WiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR P6WiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR P6WiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool P6WiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status P6WiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
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

Status P6WiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status P6WiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR P6WiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    // Set the wifi configuration
    wifi_config_t wifi_config;
    chip::DeviceLayer::Internal::P6Utils::populate_wifi_config_t(
        &wifi_config, WIFI_IF_STA, (const cy_wcm_ssid_t *) ssid, (const cy_wcm_passphrase_t *) key,
        (keyLen) ? CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY : CY_WCM_SECURITY_OPEN);

    err = chip::DeviceLayer::Internal::P6Utils::p6_wifi_set_config(WIFI_IF_STA, &wifi_config);
    SuccessOrExit(err);

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);

exit:
    return err;
}

void P6WiFiDriver::OnConnectWiFiNetwork()
{
    if (mpConnectCallback)
    {
        CommitConfiguration();
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        mpConnectCallback = nullptr;
    }
}

void P6WiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "P6 NetworkCommissioningDelegate: SSID: %s", mStagingNetwork.ssid);
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

uint8_t P6WiFiDriver::ConvertSecuritytype(cy_wcm_security_t security)
{
    uint8_t securityType = EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
    if (security == CY_WCM_SECURITY_OPEN)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
    }
    else if (security & WPA3_SECURITY)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA3;
    }
    else if (security & WPA2_SECURITY)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA2;
    }
    else if (security & WPA_SECURITY)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA;
    }
    else if (security & WEP_ENABLED)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WEP;
    }
    return securityType;
}

void P6WiFiDriver::scan_result_callback(cy_wcm_scan_result_t * result_ptr, void * user_data, cy_wcm_scan_status_t status)
{
    if ((status == CY_WCM_SCAN_COMPLETE) || (NumAP >= kWiFiMaxNetworks))
    {
        /* Sent Scan response after Scan is complete */
        P6WiFiDriver::GetInstance().OnScanWiFiNetworkDone();
    }
    else
    {
        if (result_ptr != NULL)
        {
            /* Copy Scan results and increment the AP count */
            memcpy(&scan_result_list[NumAP], (void *) result_ptr, sizeof(cy_wcm_scan_result_t));
            /* Convert Security type to proper EmberAfSecurityType value */
            scan_result_list[NumAP].security =
                static_cast<cy_wcm_security_t>(P6WiFiDriver::GetInstance().ConvertSecuritytype(scan_result_list[NumAP].security));
            NumAP++;
        } /* end of if ( result_ptr != NULL ) */
    }     /* end of else */
}

CHIP_ERROR P6WiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    cy_rslt_t result;
    /* Reset Number of APs Scanned to Zero */
    NumAP = 0;
    /* Clear Scanned AP list */
    memset(scan_result_list, 0, sizeof(scan_result_list));
    cy_wcm_scan_filter_t scan_filter;
    memset(&scan_filter, 0, sizeof(scan_filter));

    if (!ssid.empty())
    {

        scan_filter.mode = CY_WCM_SCAN_FILTER_TYPE_SSID;
        memcpy(scan_filter.param.SSID, ssid.data(), ssid.size());
        result = cy_wcm_start_scan(scan_result_callback, NULL, &scan_filter);
    }
    else
    {
        scan_filter.mode             = CY_WCM_SCAN_FILTER_TYPE_RSSI;
        scan_filter.param.rssi_range = CY_WCM_SCAN_RSSI_GOOD;
        result                       = cy_wcm_start_scan(scan_result_callback, NULL, &scan_filter);
    }
    if (result != CY_RSLT_SUCCESS)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CHIP_NO_ERROR;
}

void P6WiFiDriver::OnScanWiFiNetworkDone()
{
    uint8_t ap_num = NumAP;
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
    cy_wcm_scan_result_t * ScanResult = &scan_result_list[0];

    if (ScanResult)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_num, ScanResult]() {
                P6ScanResponseIterator iter(ap_num, ScanResult);
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
}

void P6WiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
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
    cy_wcm_associated_ap_info_t ap_info;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cy_wcm_get_associated_ap_info(&ap_info);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "cy_wcm_get_associated_ap_info failed: %d", (int) result);
        return CHIP_ERROR_INTERNAL;
    }
    uint8_t length = strnlen(reinterpret_cast<const char *>(ap_info.SSID), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, ap_info.SSID, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

void P6WiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staEnabled = false, staConnected = false;
    VerifyOrReturn(P6Utils::IsStationEnabled(staEnabled) == CHIP_NO_ERROR);
    VerifyOrReturn(staEnabled && mpStatusChangeCallback != nullptr);
    CHIP_ERROR err = GetConnectedNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }
    VerifyOrReturn(P6Utils::IsStationConnected(staConnected) == CHIP_NO_ERROR);
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

CHIP_ERROR P6WiFiDriver::SetLastDisconnectReason(int32_t reason)
{
    mLastDisconnectedReason = reason;
    return CHIP_NO_ERROR;
}

int32_t P6WiFiDriver::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t P6WiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool P6WiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
