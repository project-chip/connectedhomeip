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

// NOTE: For WiFiDriver, we uses two network configs, one is mSavedNetworks, and another is mStagingNetworks, during init, it will
// load the network config from k-v storage, and loads it into both mSavedNetworks and mStagingNetworks. When updating the networks,
// all changed are made on the staging network, and when the network is committed, it will update the mSavedNetworks to
// mStagingNetworks and persist the changes.

// NOTE: The number of network can be configured by updating the CHIP_CONFIG_MAX_WIFI_NETWORK value. The two network
// configs will be an array of the configured size.

// NOTE: LinuxWiFiDriver uses network config with empty ssid (ssidLen = 0) for empty network config.

// NOTE: For now, the LinuxWiFiDriver only supports one network, this can be fixed by using the wpa_supplicant API directly (then
// wpa_supplicant will manage the networks for us.)

CHIP_ERROR LinuxWiFiDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    WiFiNetwork network[kMaxNetworks];
    size_t valueLen = 0;

    auto & kvs = PersistedStorage::KeyValueStoreMgr();

    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        SuccessOrExit(err = IgnoreNotFound(
                          kvs.Get(kWiFiSSIDKeyName, network[networkIndex].ssid, sizeof(network[networkIndex].ssid), &valueLen)));
        if (valueLen != 0)
        {
            network[networkIndex].ssidLen = valueLen;

            err = kvs.Get(kWiFiCredentialsKeyName, network[networkIndex].credentials, sizeof(network[networkIndex].credentials),
                          &valueLen);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                SuccessOrExit(err = kvs.Get(kWifiNetworkIdentityKeyName, network[networkIndex].networkIdentity,
                                            sizeof(network[networkIndex].networkIdentity), &valueLen));
                VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
                network[networkIndex].networkIdentityLen = valueLen;

                SuccessOrExit(err = kvs.Get(kWifiClientIdentityKeyName, network[networkIndex].clientIdentity,
                                            sizeof(network[networkIndex].clientIdentity), &valueLen));
                VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
                network.[networkIndex].clientIdentityLen = valueLen;

                P256SerializedKeypair serializedKeypair;
                SuccessOrExit(err = kvs.Get(kWifiClientIdentityKeypairKeyName, serializedKeypair.Bytes(),
                                            serializedKeypair.Capacity(), &valueLen));
                serializedKeypair.SetLength(valueLen);
                network[networkIndex].clientIdentityKeypair = Platform::MakeShared<P256Keypair>();
                SuccessOrExit(err = network[networkIndex].clientIdentityKeypair->Deserialize(serializedKeypair));
            }
            else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            {
                SuccessOrExit(err);
                network[networkIndex].credentialsLen = valueLen;
            }

            mStagingNetworks[networkIndex] = mSavedNetworks[networkIndex] = network[networkIndex];
            mCurrentNetworkIndex++;
        }
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

    for (int networkIndex = 0; networkIndex < mCurrentNetworkIndex; networkIndex++)
    {
        ReturnErrorOnFailure(kvs.Put(kWiFiSSIDKeyName, mStagingNetworks[networkIndex].ssid, mStagingNetworks[networkIndex].ssidLen));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (mStagingNetworks[networkIndex].UsingPDC())
        {
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWiFiCredentialsKeyName)));
            ReturnErrorOnFailure(kvs.Put(kWifiNetworkIdentityKeyName, mStagingNetworks[networkIndex].networkIdentity,
                                         mStagingNetworks[networkIndex].networkIdentityLen));
            ReturnErrorOnFailure(kvs.Put(kWifiClientIdentityKeyName, mStagingNetworks[networkIndex].clientIdentity,
                                         mStagingNetworks[networkIndex].clientIdentityLen));

            P256SerializedKeypair serializedKeypair;
            ReturnErrorOnFailure(mStagingNetworks[networkIndex].clientIdentityKeypair->Serialize(serializedKeypair));
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
            ReturnErrorOnFailure(kvs.Put(kWiFiCredentialsKeyName, mStagingNetworks[networkIndex].credentials,
                                         mStagingNetworks[networkIndex].credentialsLen));
        }
    }

    ReturnErrorOnFailure(ConnectivityMgrImpl().CommitConfig());
    memset(mSavedNetworks, 0, sizeof(mSavedNetworks));
    memcpy(mSavedNetworks, mStagingNetworks, sizeof(mSavedNetworks));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxWiFiDriver::RevertConfiguration()
{
    mCurrentNetworkIndex = 0;

    memcpy(mStagingNetworks, mSavedNetworks, sizeof(mStagingNetworks));
    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        // ssidlen will be 0 if entry is not populated
        if (!mStagingNetworks[networkIndex].Empty())
        {
            mCurrentNetworkIndex++;
        }
    }
    return CHIP_NO_ERROR;
}

Status LinuxWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    VerifyOrReturnError(mCurrentNetworkIndex < kMaxNetworks, Status::kBoundsExceeded);
    VerifyOrReturnError(mStagingNetworks[mCurrentNetworkIndex].Empty() || mStagingNetworks[mCurrentNetworkIndex].Matches(ssid),
                        Status::kBoundsExceeded);

    // Do the check before setting the values, so the data is not updated on error.
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetworks[mCurrentNetworkIndex].credentials), Status::kOutOfRange);
    VerifyOrReturnError(!ssid.empty() && ssid.size() <= sizeof(mStagingNetworks[mCurrentNetworkIndex].ssid), Status::kOutOfRange);

    memcpy(mStagingNetworks[mCurrentNetworkIndex].credentials, credentials.data(), credentials.size());
    mStagingNetworks[mCurrentNetworkIndex].credentialsLen =
        static_cast<decltype(mStagingNetworks[mCurrentNetworkIndex].credentialsLen)>(credentials.size());

    memcpy(mStagingNetworks[mCurrentNetworkIndex].ssid, ssid.data(), ssid.size());
    mStagingNetworks[mCurrentNetworkIndex].ssidLen = static_cast<decltype(mStagingNetworks[mCurrentNetworkIndex].ssidLen)>(ssid.size());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mStagingNetworks[mCurrentNetworkIndex].networkIdentityLen = 0;
    mStagingNetworks[mCurrentNetworkIndex].clientIdentityLen  = 0;
    mStagingNetworks[mCurrentNetworkIndex].clientIdentityKeypair.reset();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mCurrentNetworkIndex++;

    return Status::kSuccess;
}

void LinuxWiFiDriver::ShiftNetworkAfterRemove()
{
    WiFiNetwork tempNetwork[kMaxNetworks];
    uint8_t count = 0;

    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        // ssidlen will be 0 if entry is not populated
        if (!mStagingNetworks[networkIndex].Empty())
        {
            tempNetwork[count] = mStagingNetworks[networkIndex];
            count++;
        }
    }

    memcpy(mStagingNetworks, tempNetwork, sizeof(mStagingNetworks));
}

Status LinuxWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex   = 0;
    bool foundNetwork = false;

    VerifyOrReturnError(mCurrentNetworkIndex != 0, Status::kBoundsExceeded);

    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (mStagingNetworks[networkIndex].Matches(networkId))
        {
            foundNetwork    = true;
            outNetworkIndex = networkIndex;
            break;
        }
    }

    VerifyOrReturnError(foundNetwork, Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetworks[outNetworkIndex].ssidLen = 0;
    ShiftNetworkAfterRemove();
    mCurrentNetworkIndex--;

    return Status::kSuccess;
}

bool LinuxWiFiDriver::StartReorderingEntries(uint8_t index, int8_t foundNetworkAtIndex)
{
    WiFiNetwork tempNetwork[kMaxNetworks];
    int count      = 0;
    int indexCount = 0;

    if (foundNetworkAtIndex < 0 || index == foundNetworkAtIndex)
    {
        return false;
    }

    // Directly assing the network to the new position
    tempNetwork[index] = mStagingNetworks[foundNetworkAtIndex];

    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (indexCount < kMaxNetworks && index != indexCount)
        {
            if (count == foundNetworkAtIndex)
            {
                count++;
            }

            tempNetwork[indexCount] = mStagingNetworks[count];

            if (count != foundNetworkAtIndex)
            {
                count++;
            }
        }

        if (count > (kMaxNetworks - 1))
        {
            count = 0;
        }
        indexCount++;
    }

    memcpy(mStagingNetworks, tempNetwork, sizeof(mStagingNetworks));

    return true;
}

Status LinuxWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    bool foundNetwork          = false;
    int8_t foundNetworkAtIndex = -1;
    outDebugText.reduce_size(0);

    // Check if network does not contain a matching entry
    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (mStagingNetworks[networkIndex].Matches(networkId))
        {
            foundNetwork        = true;
            foundNetworkAtIndex = networkIndex;
            break;
        }
    }
    VerifyOrReturnError(foundNetwork, Status::kNetworkIDNotFound);
    VerifyOrReturnError(index < mCurrentNetworkIndex, Status::kOutOfRange);
    VerifyOrReturnError(StartReorderingEntries(index, foundNetworkAtIndex), Status::kUnknownError);
    // We only support one network, so reorder is actually no-op.
    return Status::kSuccess;
}

void LinuxWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;
    bool foundNetwork       = false;

    const auto & network = mStagingNetworks;
    // Check if network does not contain a matching entry
    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (network[networkIndex].Matches(networkId))
        {
            foundNetwork          = true;
            mConnectedNetworkIndex = networkIndex;
            break;
        }
    }
    VerifyOrExit(foundNetwork, networkingStatus = Status::kNetworkIDNotFound);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    if (network.UsingPDC())
    {
        ChipLogProgress(NetworkProvisioning, "LinuxWiFiDriver: ConnectNetwork (PDC) '%.*s'", network[mConnectedNetworkIndex].ssidLen,
                        network[mConnectedNetworkIndex].ssid);
        err = ConnectivityMgrImpl().ConnectWiFiNetworkWithPDCAsync(
            ByteSpan(network[mConnectedNetworkIndex].ssid, network[mConnectedNetworkIndex].ssidLen),
            ByteSpan(network[mConnectedNetworkIndex].networkIdentity, network[mConnectedNetworkIndex].networkIdentityLen),
            ByteSpan(network[mConnectedNetworkIndex].clientIdentity, network[mConnectedNetworkIndex].clientIdentityLen),
            *network[mConnectedNetworkIndex].clientIdentityKeypair, callback);
    }
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
        ChipLogProgress(NetworkProvisioning, "LinuxWiFiDriver: ConnectNetwork '%.*s'", network[mConnectedNetworkIndex].ssidLen,
                        network[mConnectedNetworkIndex].ssid);

        err = ConnectivityMgrImpl().ConnectWiFiNetworkAsync(
            ByteSpan(network[mConnectedNetworkIndex].ssid, network[mConnectedNetworkIndex].ssidLen),
            ByteSpan(network[mConnectedNetworkIndex].credentials, network[mConnectedNetworkIndex].credentialsLen), callback);
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
    size_t count = 0;
    for (int networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (!driver->mStagingNetworks[networkIndex].Empty())
        {
            count++;
        }
    }
    return count;
}

bool LinuxWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (exhausted || driver->mStagingNetworks[networkIteratorIndex].Empty() || networkIteratorIndex > (kMaxNetworks - 1))
    {
        return false;
    }
    memcpy(item.networkID, driver->mStagingNetworks[networkIteratorIndex].ssid,
           driver->mStagingNetworks[networkIteratorIndex].ssidLen);
    item.networkIDLen = driver->mStagingNetworks[networkIteratorIndex].ssidLen;
    item.connected    = false;
    if (networkIteratorIndex == (kMaxNetworks - 1))
    {
        exhausted            = true;
        networkIteratorIndex = 0;
    }

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
    networkIteratorIndex++;

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
    VerifyOrExit(mStagingNetworks[mCurrentNetworkIndex].Empty() || mStagingNetworks[mCurrentNetworkIndex].Matches(ssid),
                 outStatus = Status::kBoundsExceeded);

    VerifyOrExit(!ssid.empty() && ssid.size() <= sizeof(WiFiNetwork::ssid), outStatus = Status::kOutOfRange);
    VerifyOrExit(!networkIdentity.empty() && networkIdentity.size() <= sizeof(WiFiNetwork::networkIdentity),
                 outStatus = Status::kOutOfRange);
    VerifyOrExit(!clientIdentityNetworkIndex.HasValue() ||
                     (clientIdentityNetworkIndex.Value() == 0 && mStagingNetworks[mCurrentNetworkIndex].UsingPDC()),
                 outStatus = Status::kOutOfRange);

    {
        WiFiNetwork network = mStagingNetworks; // update a copy first in case of errors

        memcpy(network[mCurrentNetworkIndex].ssid, ssid.data(), network[mCurrentNetworkIndex].ssidLen = ssid.size());
        memcpy(network[mCurrentNetworkIndex].networkIdentity, networkIdentity.data(),
               network[mCurrentNetworkIndex].networkIdentityLen = networkIdentity.size());

        // If an existing client identity is being reused, we would need to copy it here,
        // but since we're only supporting a single network we simply don't overwrite it.
        if (!clientIdentityNetworkIndex.HasValue())
        {
            network[mCurrentNetworkIndex].clientIdentityKeypair = Platform::MakeShared<P256Keypair>();
            SuccessOrExit(err = network[mCurrentNetworkIndex].clientIdentityKeypair->Initialize(ECPKeyTarget::ECDSA));

            MutableByteSpan clientIdentity(network[mCurrentNetworkIndex].clientIdentity);
            SuccessOrExit(err = NewChipNetworkIdentity(*network[mCurrentNetworkIndex].clientIdentityKeypair, clientIdentity));
            network[mCurrentNetworkIndex].clientIdentityLen = clientIdentity.size();
        }

        network[mCurrentNetworkIndex].credentialsLen = 0;

        SuccessOrExit(
            err = CopySpanToMutableSpan(ByteSpan(network[mCurrentNetworkIndex].clientIdentity, network[mCurrentNetworkIndex].clientIdentityLen),
                                        outClientIdentity));
        mCurrentNetworkIndex++;
        mStagingNetworks = network;
        outNetworkIndex = mCurrentNetworkIndex;
        outStatus       = Status::kSuccess;
    }

exit:
    outDebugText.reduce_size(0);
    return err;
}

CHIP_ERROR LinuxWiFiDriver::GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
{
    VerifyOrReturnError(!mStagingNetworks[mCurrentNetworkIndex].Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[mCurrentNetworkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(
        ByteSpan(mStagingNetworks[mCurrentNetworkIndex].networkIdentity, mStagingNetworks[mCurrentNetworkIndex].networkIdentityLen),
        outNetworkIdentity);
}

CHIP_ERROR LinuxWiFiDriver::GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
{
    VerifyOrReturnError(!mStagingNetworks[mCurrentNetworkIndex].Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[mCurrentNetworkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(
        ByteSpan(mStagingNetworks[mCurrentNetworkIndex].clientIdentity, mStagingNetworks[mCurrentNetworkIndex].clientIdentityLen), outClientIdentity);
}

CHIP_ERROR LinuxWiFiDriver::SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                                   P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(!mStagingNetworks[mCurrentNetworkIndex].Empty() && networkIndex == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[mCurrentNetworkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return mStagingNetworks[mCurrentNetworkIndex].clientIdentityKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
