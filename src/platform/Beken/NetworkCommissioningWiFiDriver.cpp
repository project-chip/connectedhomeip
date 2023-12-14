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
#include <platform/Beken/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>

#include <limits>
#include <string>

#include "wlan_ui_pub.h"

using namespace ::chip;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

CHIP_ERROR BekenWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::Init\r\n");

    err = GetSavedNetWorkConfig(&mSavedNetwork);

    mStagingNetwork        = mSavedNetwork;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;
    return err;
}

CHIP_ERROR BekenWiFiDriver::GetSavedNetWorkConfig(WiFiNetwork * WifiNetconf)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    memset(WifiNetconf, 0x0, sizeof(WiFiNetwork));
    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, WifiNetconf->credentials,
                                                   sizeof(WifiNetconf->credentials), &credentialsLen);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, WifiNetconf->ssid, sizeof(WifiNetconf->ssid), &ssidLen);
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    WifiNetconf->credentialsLen = credentialsLen;
    WifiNetconf->ssidLen        = ssidLen;
    return err;
}

void BekenWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR BekenWiFiDriver::CommitConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::CommitConfiguration\r\n");
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR BekenWiFiDriver::RevertConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::RevertConfiguration\r\n");
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool BekenWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::NetworkMatch\r\n");
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status BekenWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::AddOrUpdateNetwork\r\n");

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

Status BekenWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::RemoveNetwork\r\n");

    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status BekenWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::ReorderNetwork\r\n");
    // Only one network is supported now
    outDebugText.reduce_size(0);
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR BekenWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::ConnectWiFiNetwork....ssid:%s", StringOrNullMarker(ssid));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));

    wifi_sta_config_t sta_config;
    memset(&sta_config, 0x0, sizeof(sta_config));
    sta_config.security = WIFI_SECURITY_AUTO; // can't use WIFI_DEFAULT_STA_CONFIG because of C99 designator error
    Platform::CopyString(sta_config.ssid, ssidLen, ssid);
    Platform::CopyString(sta_config.password, keyLen, key);

    BK_LOG_ON_ERR(bk_wifi_sta_set_config(&sta_config));
    BK_LOG_ON_ERR(bk_wifi_sta_start());
    BK_LOG_ON_ERR(bk_wifi_sta_connect());

    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

void BekenWiFiDriver::OnConnectWiFiNetwork()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::OnConnectWiFiNetwork\r\n");
    if (mpConnectCallback)
    {
        // chip::DeviceLayer::PlatformMgr().LockChipStack();
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        // chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        mpConnectCallback = nullptr;
    }
}

void BekenWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::ConnectNetwork\r\n");
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);

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
        // chip::DeviceLayer::PlatformMgr().LockChipStack();
        callback->OnResult(networkingStatus, CharSpan(), 0);
        // chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
}

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::GetConnectedNetwork\r\n");
    wifi_link_status_t wifi_setting;
    memset(&wifi_setting, 0x0, sizeof(wifi_setting));
    bk_wifi_sta_get_link_status(&wifi_setting);
    uint8_t length = strnlen(reinterpret_cast<const char *>(wifi_setting.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);

    os_memcpy(network.networkID, wifi_setting.ssid, length);
    ChipLogProgress(NetworkProvisioning, "networkID:[%s][%d]\r\n", network.networkID, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}

void BekenWiFiDriver::OnNetworkStatusChange()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::OnNetworkStatusChange\r\n");
    Network configuredNetwork = { 0 };

    VerifyOrReturn(mpStatusChangeCallback != nullptr);
    GetConnectedNetwork(configuredNetwork);

    if (configuredNetwork.networkIDLen)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
        return;
    }
    mpStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
        MakeOptional(GetLastDisconnectReason()));
}

CHIP_ERROR BekenWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::SetLastDisconnectReason\r\n");
    mLastDisconnectedReason = event->Platform.BKSystemEvent.Data.WiFiStaDisconnected;
    return CHIP_NO_ERROR;
}

int32_t BekenWiFiDriver::GetLastDisconnectReason()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::GetLastDisconnectReason\r\n");
    return mLastDisconnectedReason;
}

static beken_semaphore_t matter_scan_sema = NULL;

int scan_done_handler(void * arg, event_module_t event_module, int event_id, void * event_data)
{
    if (matter_scan_sema)
    {
        rtos_set_semaphore(&matter_scan_sema);
    }
    return BK_OK;
}

CHIP_ERROR BekenWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::StartScanWiFiNetworks\r\n");
    wifi_scan_config_t config = { 0 };
    if (ssid.data() == NULL) // non-directed scanning
    {
        ChipLogProgress(NetworkProvisioning, "non-directed scanning...\r\n");
        BK_LOG_ON_ERR(bk_wifi_scan_start(NULL));
    }
    else // directed scanning
    {
        os_memcpy(config.ssid, ssid.data(), ssid.size());
        ChipLogProgress(NetworkProvisioning, "directed scanning... ssid:%s ; %d \r\n", config.ssid, ssid.size());
        BK_LOG_ON_ERR(bk_wifi_scan_start(&config));
    }
    return CHIP_NO_ERROR;
}

void BekenWiFiDriver::OnScanWiFiNetworkDone()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::OnScanWiFiNetworkDone\r\n");
    if (!GetInstance().mpScanCallback)
    {
        ChipLogProgress(NetworkProvisioning, "can't find the ScanCallback function\r\n");
        return;
    }
    wifi_scan_result_t scan_result = { 0 };
    int scan_rst_ap_num            = 0;
    BK_LOG_ON_ERR(bk_wifi_scan_get_result(&scan_result));
    scan_rst_ap_num = scan_result.ap_num;
    if (scan_rst_ap_num < 2) // beken scan result > = 1
    {
        ChipLogProgress(NetworkProvisioning, "NULL AP\r\n");
        GetInstance().mpScanCallback->OnFinished(Status::kNetworkNotFound, CharSpan(), nullptr);
    }
    else
    {
        ChipLogProgress(NetworkProvisioning, "AP num = %d\r\n", scan_rst_ap_num);
        BKScanResponseIterator iter(scan_rst_ap_num, &scan_result);
        GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
    }
    GetInstance().mpScanCallback = nullptr;
    bk_wifi_scan_free_result(&scan_result);
}

void BekenWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::ScanNetworks\r\n");
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (matter_scan_sema == NULL)
        {
            BK_LOG_ON_ERR(rtos_init_semaphore(&matter_scan_sema, 1));
        }
        BK_LOG_ON_ERR(bk_event_register_cb(EVENT_MOD_WIFI, EVENT_WIFI_SCAN_DONE, scan_done_handler, NULL));
        if (StartScanWiFiNetworks(ssid) != CHIP_NO_ERROR)
        {
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        BK_LOG_ON_ERR(rtos_get_semaphore(&matter_scan_sema, 4000)); // timeout 4000ms
        BK_LOG_ON_ERR(bk_event_unregister_cb(EVENT_MOD_WIFI, EVENT_WIFI_SCAN_DONE, scan_done_handler));
        BK_LOG_ON_ERR(rtos_deinit_semaphore(&matter_scan_sema));
        matter_scan_sema = NULL;
        OnScanWiFiNetworkDone();
    }
}

size_t BekenWiFiDriver::WiFiNetworkIterator::Count()
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::WiFiNetworkIterator::Count\r\n");
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool BekenWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    ChipLogProgress(NetworkProvisioning, "BekenWiFiDriver::WiFiNetworkIterator::Next\r\n");
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    uint8_t length =
        strnlen(reinterpret_cast<const char *>(mDriver->mStagingNetwork.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, length);
    item.networkIDLen        = length;
    item.connected           = false;
    mExhausted               = true;
    Network connectedNetwork = { 0 };
    CHIP_ERROR err           = GetConnectedNetwork(connectedNetwork);
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
