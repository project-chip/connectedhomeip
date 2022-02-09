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
#include <platform/mbed/NetworkCommissioningDriver.h>

#include <net_common.h>
#include <netsocket/WiFiInterface.h>

#include <limits>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

CHIP_ERROR MbedWiFiDriver::Init()
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

    mStagingNetwork  = mSavedNetwork;
    mScanCallback    = nullptr;
    mConnectCallback = nullptr;
    mScanSpecific    = false;

    return err;
}

CHIP_ERROR MbedWiFiDriver::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedWiFiDriver::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool MbedWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status MbedWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials)
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

Status MbedWiFiDriver::RemoveNetwork(ByteSpan networkId)
{
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status MbedWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index)
{
    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR MbedWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    return ConnectivityMgrImpl().ProvisionWiFiNetwork(ssid, key);
}

void MbedWiFiDriver::OnConnectWiFiNetwork()
{
    const char * ssid = mStagingNetwork.ssid;
    const char * key  = mStagingNetwork.credentials;

    // Set WiFi credentials
    auto error = mWiFiInterface->set_credentials(ssid, key, mSecurityType);
    if (error)
    {
        ChipLogError(DeviceLayer, "Set WiFi credentials failed %d", error);
        if (mConnectCallback)
        {
            mConnectCallback->OnResult(Status::kUnknownError, CharSpan(), 0);
            mConnectCallback = nullptr;
        }
        return;
    }

    // Connect Wifi network
    error = mWiFiInterface->connect();
    if (error)
    {
        ChipLogError(DeviceLayer, "Connect WiFi network failed %d", error);
        if (mConnectCallback)
        {
            mConnectCallback->OnResult(Status::kOtherConnectionFailure, CharSpan(), 0);
            mConnectCallback = nullptr;
        }
        return;
    }
}

void MbedWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "Mbed WiFi driver connect network: SSID: %s", networkId.data());

    err              = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                             reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);
    mConnectCallback = callback;
exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
        mConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

void MbedWiFiDriver::ExecuteScanNetwork()
{
    WiFiInterface * wifi_interface;
    uint16_t ap_number, ap_index = 0;
    WiFiAccessPoint * ap_buffer;
    MbedScanResponseIterator * scan_resp_iter;

    auto net_if = get_mbed_net_if();
    if (get_mbed_net_if() == nullptr && get_mbed_net_if()->wifiInterface() == nullptr)
    {
        ChipLogError(DeviceLayer, "No WiFi network interface available");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kNetworkNotFound, CharSpan(), nullptr);
        }
        goto exit;
    }

    wifi_interface = get_mbed_net_if()->wifiInterface();
    // Get APs number
    ap_number = wifi_interface->scan(nullptr, 0);
    if (!ap_number)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

    ap_buffer = new WiFiAccessPoint[ap_number];
    if (ap_buffer == nullptr)
    {
        ChipLogError(DeviceLayer, "Can't malloc memory for AP list");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

    // Get APs details
    if (wifi_interface->scan(ap_buffer, ap_number) < 0)
    {
        if (mScanSpecific)
        {
            while (ap_index < ap_number)
            {
                if (strcmp(ap_buffer->get_ssid(), mScanSSID) == 0)
                {
                    break;
                }
                ap_buffer++;
                ap_index++;
            }
            if (ap_index == ap_number)
            {
                ChipLogError(DeviceLayer, "Specific AP not found");
                if (mScanCallback)
                {
                    mScanCallback->OnFinished(Status::kNetworkNotFound, CharSpan(), nullptr);
                }
                goto exit;
            }
            ap_number = 1;
        }
        scan_resp_iter = new MbedScanResponseIterator(ap_number, ap_buffer);
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kSuccess, CharSpan(), scan_resp_iter);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Scan Wifi AP failed");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

exit:
    if (ap_buffer)
    {
        delete[] ap_buffer;
    }
    mScanCallback = nullptr;
    mScanSpecific = false;
}

void MbedWiFiDriver::OnScanNetwork(intptr_t arg)
{
    GetInstance().ExecuteScanNetwork();
}

void MbedWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (ssid.data())
    {
        memset(mScanSSID, 0, sizeof(mScanSSID));
        memcpy(mScanSSID, ssid.data(), ssid.size());
        mScanSpecific = true;
    }
    mScanCallback = callback;
    PlatformMgr().ScheduleWork(OnScanNetwork, 0);
}

size_t MbedWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool MbedWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = get_mbed_net_if()->wifiInterface()->get_connection_status() == NSAPI_STATUS_LOCAL_UP ||
        get_mbed_net_if()->wifiInterface()->get_connection_status() == NSAPI_STATUS_GLOBAL_UP;
    mExhausted = true;

    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
