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
// The length value is the strlen of the keyname plus the number of networks allowed followed by a null terminator.
// Example: "wifi-ssid" has a string len of 9. If the max network supported is 2 then we want to create keys
// "wifi-ssid0" and "wifi-ssid1". With the below method we can only have a max of 10 networks as the lengths would
// change if wanting to have wifi-ssid10.
#define SSID_KEY_NAME_LEN 11 // wifi-ssid
#define PASS_KEY_NAME_LEN 11 // wifi-pass
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
#define NETWORK_ID_KEY_NAME_LEN 9     // wifi-ni
#define CLIENT_ID_KEY_NAME_LEN 9      // wifi-ci
#define CLIENT_ID_KEYPAIR_NAME_LEN 10 // wifi-cik
#endif                                // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

// TODO(#14172): Here, most interfaces are just calling ConnectivityManager interfaces, this is because the ConnectivityProvides
// some bootstrap code for the wpa_supplicant. However, we can wrap the wpa_supplicant dbus api directly (and remove the related
// code in ConnectivityManagerImpl).
namespace {
constexpr char intToChar(int i)
{
    char c = i + '0';
    return c;
}

template <size_t size>
constexpr std::array<char[size], kMaxNetworks> createNetworkStorageKeys(const char keyName[size])
{
    std::array<char[size], kMaxNetworks> arr{};

    for (int i = 0; i < kMaxNetworks; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            arr[i][j] = keyName[j];
        }
        // We assume keyName[size - 1] is null-terminated. Adding this check to the function won't 
        // make it a consteval. The check can be incorporated when compiling the code with C++20.
        arr[i][size - 2] = intToChar(i);
    }
    return arr;
}

constexpr char ssidKeyName[SSID_KEY_NAME_LEN] = "wifi-ssid";
constexpr char passKeyName[PASS_KEY_NAME_LEN] = "wifi-pass";
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
constexpr char networkIDKeyName[NETWORK_ID_KEY_NAME_LEN]          = "wifi-ni";
constexpr char clientIDKeyName[CLIENT_ID_KEY_NAME_LEN]            = "wifi-ci";
constexpr char clientIDKeypairKeyName[CLIENT_ID_KEYPAIR_NAME_LEN] = "wifi-cik";
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

constexpr std::array<char[SSID_KEY_NAME_LEN], kMaxNetworks> kWiFiSSIDKeyName =
    createNetworkStorageKeys<SSID_KEY_NAME_LEN>(ssidKeyName);
constexpr std::array<char[PASS_KEY_NAME_LEN], kMaxNetworks> kWiFiCredentialsKeyName =
    createNetworkStorageKeys<PASS_KEY_NAME_LEN>(passKeyName);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
constexpr std::array<char[NETWORK_ID_KEY_NAME_LEN], kMaxNetworks> kWifiNetworkIdentityKeyName =
    createNetworkStorageKeys<NETWORK_ID_KEY_NAME_LEN>(networkIDKeyName);
constexpr std::array<char[CLIENT_ID_KEY_NAME_LEN], kMaxNetworks> kWifiClientIdentityKeyName =
    createNetworkStorageKeys<CLIENT_ID_KEY_NAME_LEN>(clientIDKeyName);
constexpr std::array<char[CLIENT_ID_KEYPAIR_NAME_LEN], kMaxNetworks> kWiFiSSIDKeyName =
    createNetworkStorageKeys<CLIENT_ID_KEYPAIR_NAME_LEN>(clientIDKeypairKeyName);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

inline CHIP_ERROR IgnoreNotFound(CHIP_ERROR err)
{
    return (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND) ? CHIP_NO_ERROR : err;
}
} // namespace

// NOTE: For WiFiDriver, we use two sets of network configs, one is mSavedNetworks, and another is mStagingNetworks, during init, it
// will load the network config from k-v storage, and loads it into both mSavedNetworks and mStagingNetworks. When updating the
// networks, all changes are made on the staging network, and when the network is committed, it will update the mSavedNetworks to
// mStagingNetworks and persist the changes.

// NOTE: LinuxWiFiDriver uses network config with empty ssid (ssidLen = 0) for empty network config.

CHIP_ERROR LinuxWiFiDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    WiFiNetwork network;
    size_t valueLen = 0;

    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        SuccessOrExit(err = IgnoreNotFound(kvs.Get(kWiFiSSIDKeyName[networkIndex], network.ssid, sizeof(network.ssid), &valueLen)));
        if (valueLen != 0)
        {
            network.ssidLen = valueLen;

            err = kvs.Get(kWiFiCredentialsKeyName[networkIndex], network.credentials, sizeof(network.credentials), &valueLen);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                SuccessOrExit(err = kvs.Get(kWifiNetworkIdentityKeyName[networkIndex], network.networkIdentity,
                                            sizeof(network.networkIdentity), &valueLen));
                VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
                network.networkIdentityLen = valueLen;

                SuccessOrExit(err = kvs.Get(kWifiClientIdentityKeyName[networkIndex], network.clientIdentity,
                                            sizeof(network.clientIdentity), &valueLen));
                VerifyOrExit(valueLen > 0, err = CHIP_ERROR_INTEGRITY_CHECK_FAILED);
                network.clientIdentityLen = valueLen;

                P256SerializedKeypair serializedKeypair;
                SuccessOrExit(err = kvs.Get(kWifiClientIdentityKeypairKeyName[networkIndex], serializedKeypair.Bytes(),
                                            serializedKeypair.Capacity(), &valueLen));
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

            mStagingNetworks[networkIndex] = mSavedNetworks[networkIndex] = network;
            mStagingNetworkCount++;
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

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        // Add entries to persistent storage upto mStagingNetworkCount and delete all other entries if present.
        if (networkIndex < mStagingNetworkCount)
        {
            ReturnErrorOnFailure(kvs.Put(kWiFiSSIDKeyName[networkIndex], mStagingNetworks[networkIndex].ssid,
                                         mStagingNetworks[networkIndex].ssidLen));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            if (mStagingNetworks[networkIndex].UsingPDC())
            {
                ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWiFiCredentialsKeyName[networkIndex])));
                ReturnErrorOnFailure(kvs.Put(kWifiNetworkIdentityKeyName[networkIndex],
                                             mStagingNetworks[networkIndex].networkIdentity,
                                             mStagingNetworks[networkIndex].networkIdentityLen));
                ReturnErrorOnFailure(kvs.Put(kWifiClientIdentityKeyName[networkIndex],
                                             mStagingNetworks[networkIndex].clientIdentity,
                                             mStagingNetworks[networkIndex].clientIdentityLen));

                P256SerializedKeypair serializedKeypair;
                ReturnErrorOnFailure(mStagingNetworks[networkIndex].clientIdentityKeypair->Serialize(serializedKeypair));
                ReturnErrorOnFailure(kvs.Put(kWifiClientIdentityKeypairKeyName[networkIndex], serializedKeypair.ConstBytes(),
                                             serializedKeypair.Length()));
            }
            else
            {
                ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiNetworkIdentityKeyName[networkIndex])));
                ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeyName[networkIndex])));
                ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeypairKeyName[networkIndex])));
#else  // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            {
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
                ReturnErrorOnFailure(kvs.Put(kWiFiCredentialsKeyName[networkIndex], mStagingNetworks[networkIndex].credentials,
                                             mStagingNetworks[networkIndex].credentialsLen));
            }
        }
        else
        {
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWiFiSSIDKeyName[networkIndex])));
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWiFiCredentialsKeyName[networkIndex])));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiNetworkIdentityKeyName[networkIndex])));
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeyName[networkIndex])));
            ReturnErrorOnFailure(IgnoreNotFound(kvs.Delete(kWifiClientIdentityKeypairKeyName[networkIndex])));
#endif
        }
    }

    ReturnErrorOnFailure(ConnectivityMgrImpl().CommitConfig());
    memcpy(mSavedNetworks, mStagingNetworks, sizeof(mSavedNetworks));
    mSavedConnectedNetworkIndex = mStagedConnectedNetworkIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxWiFiDriver::RevertConfiguration()
{
    mStagingNetworkCount = 0;
    Network configuredNetwork;

    memcpy(mStagingNetworks, mSavedNetworks, sizeof(mStagingNetworks));
    mStagedConnectedNetworkIndex = mSavedConnectedNetworkIndex;

    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().GetConfiguredNetwork(configuredNetwork);

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        // ssidlen will be 0 if entry is not populated
        if (!mStagingNetworks[networkIndex].Empty())
        {
            mStagingNetworkCount++;
        }
    }

    if (err != CHIP_NO_ERROR && DeviceLayer::ConnectivityMgrImpl().IsWiFiStationConnected() &&
        !mStagingNetworks[mStagedConnectedNetworkIndex].Matches(
            ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)))
    {
        ConnectNetwork(
            ByteSpan(mStagingNetworks[mStagedConnectedNetworkIndex].ssid, mStagingNetworks[mStagedConnectedNetworkIndex].ssidLen),
            mpConnectCallback);
    }

    return CHIP_NO_ERROR;
}

Status LinuxWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = mStagingNetworkCount;

    VerifyOrReturnError(mStagingNetworkCount < kMaxNetworks, Status::kBoundsExceeded);
    VerifyOrReturnError(mStagingNetworks[mStagingNetworkCount].Empty() || mStagingNetworks[mStagingNetworkCount].Matches(ssid),
                        Status::kBoundsExceeded);

    // Do the check before setting the values, so the data is not updated on error.
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetworks[mStagingNetworkCount].credentials), Status::kOutOfRange);
    VerifyOrReturnError(!ssid.empty() && ssid.size() <= sizeof(mStagingNetworks[mStagingNetworkCount].ssid), Status::kOutOfRange);

    memcpy(mStagingNetworks[mStagingNetworkCount].credentials, credentials.data(), credentials.size());
    mStagingNetworks[mStagingNetworkCount].credentialsLen =
        static_cast<decltype(mStagingNetworks[mStagingNetworkCount].credentialsLen)>(credentials.size());

    memcpy(mStagingNetworks[mStagingNetworkCount].ssid, ssid.data(), ssid.size());
    mStagingNetworks[mStagingNetworkCount].ssidLen =
        static_cast<decltype(mStagingNetworks[mStagingNetworkCount].ssidLen)>(ssid.size());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mStagingNetworks[mStagingNetworkCount].networkIdentityLen = 0;
    mStagingNetworks[mStagingNetworkCount].clientIdentityLen  = 0;
    mStagingNetworks[mStagingNetworkCount].clientIdentityKeypair.reset();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mStagingNetworkCount++;

    return Status::kSuccess;
}

void LinuxWiFiDriver::CompressStagingNetworksList()
{
    WiFiNetwork tempNetwork[kMaxNetworks];
    uint8_t count = 0;

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
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

    VerifyOrReturnError(mStagingNetworkCount != 0, Status::kBoundsExceeded);

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (mStagingNetworks[networkIndex].Matches(networkId))
        {
            foundNetwork    = true;
            outNetworkIndex = networkIndex;
            if (networkIndex == mStagedConnectedNetworkIndex)
            {
                mStagedConnectedNetworkIndex = -1;
            }
            break;
        }
    }

    VerifyOrReturnError(foundNetwork, Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetworks[outNetworkIndex].ssidLen = 0;

    // Will need to reorder the list.
    // Shift networks that are at a lower priority than the network to be removed if present in the list.
    CompressStagingNetworksList();
    mStagingNetworkCount--;

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

    // Directly assigning the network to the new position
    tempNetwork[index] = mStagingNetworks[foundNetworkAtIndex];

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
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
    int8_t foundNetworkAtIndex = -1;
    outDebugText.reduce_size(0);

    // Check if network does not contain a matching entry
    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (mStagingNetworks[networkIndex].Matches(networkId))
        {
            foundNetworkAtIndex = networkIndex;
            break;
        }
    }
    VerifyOrReturnError(foundNetworkAtIndex != -1, Status::kNetworkIDNotFound);

    // Return out of range if the index value is greater than mStagingNetworkCount i.e the number of enties in mStagingNetworks
    VerifyOrReturnError(index < mStagingNetworkCount, Status::kOutOfRange);
    if (foundNetworkAtIndex == mStagedConnectedNetworkIndex)
    {
        mStagedConnectedNetworkIndex = index;
    }
    // Let's start to reorder the entries
    VerifyOrReturnError(StartReorderingEntries(index, foundNetworkAtIndex), Status::kUnknownError);

    return Status::kSuccess;
}

void LinuxWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;
    WiFiNetwork network;

    for (uint8_t networkIndex = 0; networkIndex < kMaxNetworks; networkIndex++)
    {
        if (mStagingNetworks[networkIndex].Matches(networkId))
        {
            mStagedConnectedNetworkIndex = networkIndex;
            network                      = mStagingNetworks[networkIndex];
            break;
        }
    }
    VerifyOrExit(!network.Empty(), networkingStatus = Status::kNetworkIDNotFound);
    mpConnectCallback = callback;

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
    return driver->mStagingNetworkCount;
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
    VerifyOrReturnError(mStagingNetworkCount < kMaxNetworks, Status::kBoundsExceeded);
    VerifyOrExit(mStagingNetworks[mStagingNetworkCount].Empty() || mStagingNetworks[mStagingNetworkCount].Matches(ssid),
                 outStatus = Status::kBoundsExceeded);

    VerifyOrExit(!ssid.empty() && ssid.size() <= sizeof(WiFiNetwork::ssid), outStatus = Status::kOutOfRange);
    VerifyOrExit(!networkIdentity.empty() && networkIdentity.size() <= sizeof(WiFiNetwork::networkIdentity),
                 outStatus = Status::kOutOfRange);
    VerifyOrExit(!clientIdentityNetworkIndex.HasValue() ||
                     (clientIdentityNetworkIndex.Value() < kMaxNetworks && mStagingNetworks[mStagingNetworkCount].UsingPDC()),
                 outStatus = Status::kOutOfRange);

    {
        WiFiNetwork network = mStagingNetworks[mStagingNetworkCount]; // update a copy first in case of errors

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

        mStagingNetworks[mStagingNetworkCount] = network;
        outNetworkIndex                        = 0;
        mStagingNetworkCount++;
        outStatus = Status::kSuccess;
    }

exit:
    outDebugText.reduce_size(0);
    return err;
}

CHIP_ERROR LinuxWiFiDriver::GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
{
    VerifyOrReturnError(!mStagingNetworks[networkIndex].Empty() && networkIndex < kMaxNetworks, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[networkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(
        ByteSpan(mStagingNetworks[networkIndex].networkIdentity, mStagingNetworks[networkIndex].networkIdentityLen),
        outNetworkIdentity);
}

CHIP_ERROR LinuxWiFiDriver::GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
{
    VerifyOrReturnError(!mStagingNetworks[networkIndex].Empty() && networkIndex < kMaxNetworks, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[networkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(
        ByteSpan(mStagingNetworks[networkIndex].clientIdentity, mStagingNetworks[networkIndex].clientIdentityLen),
        outClientIdentity);
}

CHIP_ERROR LinuxWiFiDriver::SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                                   P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(!mStagingNetworks[networkIndex].Empty() && networkIndex < kMaxNetworks, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStagingNetworks[networkIndex].UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return mStagingNetworks[networkIndex].clientIdentityKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
