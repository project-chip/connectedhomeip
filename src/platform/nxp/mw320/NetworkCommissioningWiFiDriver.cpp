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

// #include <lib/support/CodeUtils.h>
// #include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/nxp/mw320/NetworkCommissioningDriver.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Thread;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
// TODO(#14172): Here, most interfaces are just calling ConnectivityManager interfaces, this is because the ConnectivityProvides
// some bootstrap code for the wpa_supplicant. However, we can wrap the wpa_supplicant dbus api directly (and remove the related
// code in ConnectivityManagerImpl).
namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

// NOTE: For WiFiDriver, we uses two network configs, one is mSavedNetwork, and another is mStagingNetwork, during init, it will
// load the network config from k-v storage, and loads it into both mSavedNetwork and mStagingNetwork. When updating the networks,
// all changed are made on the staging network, and when the network is committed, it will update the mSavedNetwork to
// mStagingNetwork and persist the changes.

// NOTE: LinuxWiFiDriver uses network config with empty ssid (ssidLen = 0) for empty network config.

// NOTE: For now, the LinuxWiFiDriver only supports one network, this can be fixed by using the wpa_supplicant API directly (then
// wpa_supplicant will manage the networks for us.)

CHIP_ERROR Mw320WiFiDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), &ssidLen);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;

    mStagingNetwork = mSavedNetwork;

    ConnectivityMgrImpl().SetNetworkStatusChangeCallback(networkStatusChangeCallback);
    return CHIP_NO_ERROR;
}

void Mw320WiFiDriver::Shutdown()
{
    ConnectivityMgrImpl().SetNetworkStatusChangeCallback(nullptr);
}

CHIP_ERROR Mw320WiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    ReturnErrorOnFailure(ConnectivityMgrImpl().CommitConfig());
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Mw320WiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool Mw320WiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status Mw320WiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);

    static_assert(sizeof(WiFiNetwork::ssid) <= std::numeric_limits<decltype(WiFiNetwork::ssidLen)>::max(),
                  "Max length of WiFi ssid exceeds the limit of ssidLen field");
    static_assert(sizeof(WiFiNetwork::credentials) <= std::numeric_limits<decltype(WiFiNetwork::credentialsLen)>::max(),
                  "Max length of WiFi credentials exceeds the limit of credentialsLen field");

    // Do the check before setting the values, so the data is not updated on error.
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

#if (MW320_CONNECT_SCAN_SYNC == 1)
    // ConnectNetwork(ssid, nullptr);
    ConnectivityMgrImpl().UpdateNetworkStatus();
#endif // MW320_CONNECT_SCAN_SYNC

    return Status::kSuccess;
}

Status Mw320WiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status Mw320WiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    // We only support one network, so reorder is actually no-op.

    return Status::kSuccess;
}

void Mw320WiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);

    ChipLogProgress(NetworkProvisioning, "Mw320WiFiDriver: SSID: %.*s", static_cast<int>(networkId.size()), networkId.data());

    err = ConnectivityMgrImpl().ConnectWiFiNetworkAsync(ByteSpan(mStagingNetwork.ssid, mStagingNetwork.ssidLen),
                                                        ByteSpan(mStagingNetwork.credentials, mStagingNetwork.credentialsLen),
                                                        callback);
exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }

    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

void Mw320WiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().StartWiFiScan(ssid, callback);
    if (err != CHIP_NO_ERROR)
    {
        mScanStatus.SetValue(Status::kUnknownError);
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
    else
    {
        // On linux platform, once "scan" is started, we can say the result will always be success.
        mScanStatus.SetValue(Status::kSuccess);
    }
}

size_t Mw320WiFiDriver::WiFiNetworkIterator::Count()
{
    return driver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool Mw320WiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (exhausted || driver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, driver->mStagingNetwork.ssid, driver->mStagingNetwork.ssidLen);
    item.networkIDLen = driver->mStagingNetwork.ssidLen;
    item.connected    = false;
    exhausted         = true;

    Network configuredNetwork;
    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().GetConfiguredNetwork(configuredNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (DeviceLayer::ConnectivityMgrImpl().IsWiFiStationConnected() && configuredNetwork.networkIDLen == item.networkIDLen &&
            memcmp(configuredNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }

    return true;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
