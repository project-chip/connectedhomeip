/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <credentials/CHIPCert.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
constexpr char kWifiNetworkIdentityKeyName[]       = "wifi-ni";
constexpr char kWifiClientIdentityKeyName[]        = "wifi-ci";
constexpr char kWifiClientIdentityKeypairKeyName[] = "wifi-cik";
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

inline CHIP_ERROR IgnoreNotFound(CHIP_ERROR err)
{
    return (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) ? CHIP_NO_ERROR : err;
}
} // namespace

// NOTE: For WiFiDriver, we uses two network configs, one is mSavedNetwork, and another is mStagingNetwork, during init, it will
// load the network config from k-v storage, and loads it into both mSavedNetwork and mStagingNetwork. When updating the networks,
// all changed are made on the staging network, and when the network is committed, it will update the mSavedNetwork to
// mStagingNetwork and persist the changes.

// NOTE: LinuxWiFiDriver uses network config with empty ssid (ssidLen = 0) for empty network config.

// NOTE: For now, the LinuxWiFiDriver only supports one network, this can be fixed by using the wpa_supplicant API directly (then
// wpa_supplicant will manage the networks for us.)

CHIP_ERROR LinuxWiFiDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    WiFiNetwork network;
    size_t valueLen = 0;

    auto & kvs = PersistedStorage::KeyValueStoreMgr();

    SuccessOrExit(err = IgnoreNotFound(kvs.Get(kWiFiSSIDKeyName, network.ssid, sizeof(network.ssid), &valueLen)));
    if (valueLen != 0)
    {
        network.ssidLen = valueLen;

        err = kvs.Get(kWiFiCredentialsKeyName, network.credentials, sizeof(network.credentials), &valueLen);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            SuccessOrExit(
                err = kvs.Get(kWifiNetworkIdentityKeyName, network.networkIdentity, sizeof(network.networkIdentity), &valueLen));
            VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
            network.networkIdentityLen = valueLen;

            SuccessOrExit(
                err = kvs.Get(kWifiClientIdentityKeyName, network.clientIdentity, sizeof(network.clientIdentity), &valueLen));
            VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
            network.clientIdentityLen = valueLen;

            P256SerializedKeypair serializedKeypair;
            SuccessOrExit(err = kvs.Get(kWifiClientIdentityKeypairKeyName, serializedKeypair.Bytes(), serializedKeypair.Capacity(),
                                        &valueLen));
            serializedKeypair.SetLength(valueLen);
            network.clientIdentityKeypair = Platform::MakeShared<P256Keypair>();
            SuccessOrExit(err = network.clientIdentityKeypair->Deserialize(serializedKeypair));
        }
        else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        {
            SuccessOrExit(err);
            network.credentialsLen = valueLen;
        }

        mStagingNetwork = mSavedNetwork = network;
    }

    ConnectivityMgrImpl().SetNetworkStatusChangeCallback(networkStatusChangeCallback);
    return CHIP_NO_ERROR;

exit:
    ChipLogProgress(NetworkProvisioning, "LinuxWiFiDriver: Failed to load network configuration: %" CHIP_ERROR_FORMAT,
                    err.Format());
    return err;
}

void LinuxWiFiDriver::Shutdown()
{
    ConnectivityMgrImpl().SetNetworkStatusChangeCallback(nullptr);
}

CHIP_ERROR LinuxWiFiDriver::CommitConfiguration()
{
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    ReturnErrorOnFailure(kvs.Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    if (mStagingNetwork.UsingPDC())
    {
        ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWiFiCredentialsKeyName)));
        ReturnErrorOnFailure(
            kvs.Put(kWifiNetworkIdentityKeyName, mStagingNetwork.networkIdentity, mStagingNetwork.networkIdentityLen));
        ReturnErrorOnFailure(
            kvs.Put(kWifiClientIdentityKeyName, mStagingNetwork.clientIdentity, mStagingNetwork.clientIdentityLen));

        P256SerializedKeypair serializedKeypair;
        ReturnErrorOnFailure(mStagingNetwork.clientIdentityKeypair->Serialize(serializedKeypair));
        ReturnErrorOnFailure(
            kvs.Put(kWifiClientIdentityKeypairKeyName, serializedKeypair.ConstBytes(), serializedKeypair.Length()));
    }
    else
    {
        ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiNetworkIdentityKeyName)));
        ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeyName)));
        ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeypairKeyName)));
#else  // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        ReturnErrorOnFailure(kvs.Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials, mStagingNetwork.credentialsLen));
    }

    ReturnErrorOnFailure(ConnectivityMgrImpl().CommitConfig());
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxWiFiDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

Status LinuxWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.Empty() || mStagingNetwork.Matches(ssid), Status::kBoundsExceeded);

    // Do the check before setting the values, so the data is not updated on error.
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(!ssid.empty() && ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mStagingNetwork.networkIdentityLen = 0;
    mStagingNetwork.clientIdentityLen  = 0;
    mStagingNetwork.clientIdentityKeypair.reset();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    return Status::kSuccess;
}

Status LinuxWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.Matches(networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status LinuxWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    VerifyOrReturnError(mStagingNetwork.Matches(networkId), Status::kNetworkIDNotFound);
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    // We only support one network, so reorder is actually no-op.
    return Status::kSuccess;
}

void LinuxWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;

    const auto & network = mStagingNetwork;
    VerifyOrExit(network.Matches(networkId), networkingStatus = Status::kNetworkIDNotFound);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    if (network.UsingPDC())
    {
        ChipLogProgress(NetworkProvisioning, "LinuxWiFiDriver: ConnectNetwork (PDC) '%.*s'", network.ssidLen, network.ssid);
        err = ConnectivityMgrImpl().ConnectWiFiNetworkWithPDCAsync(
            ByteSpan(network.ssid, network.ssidLen), ByteSpan(network.networkIdentity, network.networkIdentityLen),
            ByteSpan(network.clientIdentity, network.clientIdentityLen), *network.clientIdentityKeypair, callback);
    }
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
        ChipLogProgress(NetworkProvisioning, "LinuxWiFiDriver: ConnectNetwork '%.*s'", network.ssidLen, network.ssid);

        err = ConnectivityMgrImpl().ConnectWiFiNetworkAsync(ByteSpan(network.ssid, network.ssidLen),
                                                            ByteSpan(network.credentials, network.credentialsLen), callback);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }

    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

void LinuxWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().StartWiFiScan(ssid, callback);
    if (err != CHIP_NO_ERROR)
    {
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

size_t LinuxWiFiDriver::WiFiNetworkIterator::Count()
{
    return driver->mStagingNetwork.Empty() ? 0 : 1;
}

bool LinuxWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (exhausted || driver->mStagingNetwork.Empty())
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
CHIP_ERROR LinuxWiFiDriver::AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity,
                                                      Optional<uint8_t> clientIdentityNetworkIndex, Status & outStatus,
                                                      MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                                      uint8_t & outNetworkIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    outStatus      = Status::kUnknownError;
    VerifyOrExit(mStagingNetwork.Empty() || mStagingNetwork.Matches(ssid), outStatus = Status::kBoundsExceeded);

    VerifyOrExit(!ssid.empty() && ssid.size() <= sizeof(WiFiNetwork::ssid), outStatus = Status::kOutOfRange);
    VerifyOrExit(!networkIdentity.empty() && networkIdentity.size() <= sizeof(WiFiNetwork::networkIdentity),
                 outStatus = Status::kOutOfRange);
    VerifyOrExit(!clientIdentityNetworkIndex.HasValue() || (clientIdentityNetworkIndex.Value() == 0 && mStagingNetwork.UsingPDC()),
                 outStatus = Status::kOutOfRange);

    {
        WiFiNetwork network = mStagingNetwork; // update a copy first in case of errors

        memcpy(network.ssid, ssid.data(), network.ssidLen = ssid.size());
        memcpy(network.networkIdentity, networkIdentity.data(), network.networkIdentityLen = networkIdentity.size());

        // If an existing client identity is being reused, we would need to copy it here,
        // but since we're only supporting a single network we simply don't overwrite it.
        if (!clientIdentityNetworkIndex.HasValue())
        {
            network.clientIdentityKeypair = Platform::MakeShared<P256Keypair>();
            SuccessOrExit(err = network.clientIdentityKeypair->Initialize(ECPKeyTarget::ECDSA));

            MutableByteSpan clientIdentity(network.clientIdentity);
            SuccessOrExit(err = NewChipNetworkIdentity(*network.clientIdentityKeypair, clientIdentity));
            network.clientIdentityLen = clientIdentity.size();
        }

        network.credentialsLen = 0;

        SuccessOrExit(err = CopySpanToMutableSpan(ByteSpan(network.clientIdentity, network.clientIdentityLen), outClientIdentity));

        mStagingNetwork = network;
        outNetworkIndex = 0;
        outStatus       = Status::kSuccess;
    }

exit:
    outDebugText.reduce_size(0);
    return err;
}

CHIP_ERROR LinuxWiFiDriver::GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
{
    VerifyOrReturnError(!mStagingNetwork.Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(mStagingNetwork.networkIdentity, mStagingNetwork.networkIdentityLen), outNetworkIdentity);
}

CHIP_ERROR LinuxWiFiDriver::GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
{
    VerifyOrReturnError(!mStagingNetwork.Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(mStagingNetwork.clientIdentity, mStagingNetwork.clientIdentityLen), outClientIdentity);
}

CHIP_ERROR LinuxWiFiDriver::SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                                   P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(!mStagingNetwork.Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return mStagingNetwork.clientIdentityKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
