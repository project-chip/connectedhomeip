/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright 2023 NXP
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
#include <platform/CHIPDeviceLayer.h>

#include "NetworkCommissioningDriver.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <wlan.h>

using namespace ::chip;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

class NXPScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    NXPScanResponseIterator(const size_t size, WiFiScanResponse * responses) : mSize(size), mResponses(responses) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }

        item = mResponses[mIternum];
        mIternum++;

        return true;
    }
    void Release() override {}

private:
    const size_t mSize;
    WiFiScanResponse * mResponses;
    size_t mIternum = 0;
};

CHIP_ERROR NXPWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), &ssidLen);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "WiFi network SSID not retrieved from persisted storage: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "WiFi network credentials not retrieved from persisted storage: %" CHIP_ERROR_FORMAT,
                        err.Format());
        return err;
    }

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork        = mSavedNetwork;
    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    // Connect to saved network
    err = ConnectWiFiNetwork(mSavedNetwork.ssid, ssidLen, mSavedNetwork.credentials, credentialsLen);

    return err;
}

void NXPWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR NXPWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;

    return CHIP_NO_ERROR;
}

CHIP_ERROR NXPWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;

    return CHIP_NO_ERROR;
}

bool NXPWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status NXPWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
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

Status NXPWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;

    return Status::kSuccess;
}

/* Returns the network SSID. User needs to allocate a buffer of size >= DeviceLayer::Internal::kMaxWiFiSSIDLength.
 * ssid - pointer to the returned SSID
 */
Status NXPWiFiDriver::GetNetworkSSID(char * ssid)
{
    VerifyOrReturnError(ssid != NULL, Status::kOutOfRange);

    memcpy(ssid, mStagingNetwork.ssid, mStagingNetwork.ssidLen);
    return Status::kSuccess;
}

/* Returns the network password. User needs to allocate a buffer of size >= DeviceLayer::Internal::kMaxWiFiKeyLength.
 * credentials - pointer to the returned password
 */
Status NXPWiFiDriver::GetNetworkPassword(char * credentials)
{
    VerifyOrReturnError(credentials != NULL, Status::kOutOfRange);

    memcpy(credentials, mStagingNetwork.credentials, mStagingNetwork.credentialsLen);
    return Status::kSuccess;
}

Status NXPWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    return Status::kSuccess;
}

CHIP_ERROR NXPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    return ConnectivityMgrImpl().ProvisionWiFiNetwork(ssid, ssidLen, key, keyLen);
}

void NXPWiFiDriver::OnConnectWiFiNetwork(Status commissioningError, CharSpan debugText, int32_t connectStatus)
{
    CommitConfiguration();

    if (mpConnectCallback != nullptr)
    {
        mpConnectCallback->OnResult(commissioningError, debugText, connectStatus);
        mpConnectCallback = nullptr;
    }
}

void NXPWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    ChipLogProgress(NetworkProvisioning, "Connecting to WiFi network: SSID: %.*s", static_cast<int>(networkId.size()),
                    networkId.data());

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);

    mpConnectCallback = callback;
    err               = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                                           reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);

exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }

    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to start connecting to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
        mpConnectCallback = nullptr;
    }

    /* Always inform the cluster of the network status so that in case of success,
     * we have time send the response to the controller before switching to a new network.
     */
    if (callback != nullptr)
    {
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

CHIP_ERROR NXPWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    wlan_scan_params_v2_t wlan_scan_param;

    ChipLogProgress(DeviceLayer, "Scan for WiFi network(s) requested");

    (void) memset(&wlan_scan_param, 0, sizeof(wlan_scan_params_v2_t));
    wlan_scan_param.cb = &NXPWiFiDriver::_OnScanWiFiNetworkDoneCallBack;

    if ((ssid.size() > 0) && (ssid.size() < MLAN_MAX_SSID_LENGTH))
    {
#ifdef CONFIG_COMBO_SCAN
        (void) memcpy(wlan_scan_param.ssid[0], ssid.data(), ssid.size());
#else
        (void) memcpy(wlan_scan_param.ssid, ssid.data(), ssid.size());
#endif
    }

    int status = wlan_scan_with_opt(wlan_scan_param);
    if (status != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to start scan for WiFi network(s): %d", status);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

int NXPWiFiDriver::_OnScanWiFiNetworkDoneCallBack(unsigned int count)
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kPlatformNxpScanWiFiNetworkDoneEvent;
    event.Platform.ScanWiFiNetworkCount = count;
    CHIP_ERROR err                      = PlatformMgr().PostEvent(&event);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to schedule work: %" CHIP_ERROR_FORMAT, err.Format());
        return WM_FAIL;
    }
    return WM_SUCCESS;
}

// The processing of the scan callback should be done in the context of the Matter stack, as the scan callback will call Matter
// stack APIs
int NXPWiFiDriver::ScanWiFINetworkDoneFromMatterTaskContext(unsigned int count)
{
    ChipLogProgress(DeviceLayer, "Scan for WiFi network(s) done, found: %u", count);

    if (count == 0)
    {
        if (GetInstance().mpScanCallback != nullptr)
        {
            GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
            GetInstance().mpScanCallback = nullptr;
        }
        return WM_SUCCESS;
    }

    std::unique_ptr<struct WiFiScanResponse[]> response_list_ptr(new struct WiFiScanResponse[count]);
    if (response_list_ptr == nullptr)
    {
        ChipLogError(DeviceLayer, "Can't allocate memory for scan response_list");
        if (GetInstance().mpScanCallback)
        {
            GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            GetInstance().mpScanCallback = nullptr;
        }
        return WM_SUCCESS;
    }

    struct WiFiScanResponse * response_list = response_list_ptr.get();

    int valid = 0;

    for (int i = 0; i < count; i++)
    {
        struct wlan_scan_result res;
        struct WiFiScanResponse response;

        int status = wlan_get_scan_result(i, &res);

        if (status != WM_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Can't get scan result: %d", status);
            continue;
        }

        response.security = WiFiSecurity::kUnencrypted;

        if (res.wep != 0U)
        {
            response.security = WiFiSecurity::kWep;
        }
        else if (res.wpa != 0U)
        {
            response.security = WiFiSecurity::kWpaPersonal;
        }
        else if (res.wpa2 != 0U)
        {
            response.security = WiFiSecurity::kWpa2Personal;
        }
        else if (res.wpa3_sae != 0U)
        {
            response.security = WiFiSecurity::kWpa3Personal;
        }
        else if (res.wpa2_entp != 0U)
        {
            // ChipLogProgress(DeviceLayer, "Ignoring scanned network with WPA2-Enterprise security");
            // continue;
        }

        if (res.ssid_len > DeviceLayer::Internal::kMaxWiFiSSIDLength)
        {
            ChipLogError(DeviceLayer, "Ignoring scanned network with too long SSID: %u", res.ssid_len);
            continue;
        }
        memcpy(response.ssid, res.ssid, res.ssid_len);
        response.ssidLen = res.ssid_len;

        memcpy(response.bssid, res.bssid, DeviceLayer::Internal::kWiFiBSSIDLength);

        response.channel  = (uint16_t) res.channel;
        response.wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4; // TODO 5 GHz also possible, but results don't
                                                                                     // show this information
        response.rssi = -static_cast<int8_t>(res.rssi);

        response_list[valid] = response;

        valid++;
    }

    if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([valid, response_list]() {
            std::unique_ptr<struct WiFiScanResponse[]> auto_free(response_list);
            NXPScanResponseIterator iter(valid, response_list);
            if (GetInstance().mpScanCallback)
            {
                GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
                GetInstance().mpScanCallback = nullptr;
            }
            else
            {
                ChipLogError(DeviceLayer, "Can't find the ScanCallback function");
            }
        }))
    {
        response_list_ptr.release();
    }

    return WM_SUCCESS;
}

void NXPWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
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

uint32_t NXPWiFiDriver::GetSupportedWiFiBandsMask() const
{
    uint32_t bands = static_cast<uint32_t>(1UL << chip::to_underlying(WiFiBandEnum::k2g4));
#ifdef CONFIG_5GHz_SUPPORT
    bands |= (1UL << chip::to_underlying(WiFiBandEnum::k5g));
#endif
    return bands;
}

static CHIP_ERROR GetConnectedNetwork(Network & network)
{
    struct wlan_network wlan_network;
    int result;

    result = wlan_get_current_network(&wlan_network);
    if (result != WM_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint8_t length = strnlen(reinterpret_cast<const char *>(wlan_network.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);

    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, wlan_network.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

size_t NXPWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool NXPWiFiDriver::WiFiNetworkIterator::Next(Network & item)
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
