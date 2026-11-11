/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <posix/emulated_network/EmulatedWiFiDriver.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

using namespace chip::DeviceLayer::NetworkCommissioning;

namespace chip::app {

namespace {

[[maybe_unused]] NullTerminated SsidForLog(ByteSpan ssid)
{
    return NullTerminated(reinterpret_cast<const char *>(ssid.data()), ssid.size());
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
void ExtractKeyIdentifier(Optional<Credentials::CertificateKeyIdStorage> & identifier, ByteSpan identity)
{
    if (Credentials::ExtractIdentifierFromChipNetworkIdentity(identity, identifier.Emplace()) != CHIP_NO_ERROR)
    {
        identifier.ClearValue();
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

} // namespace

class EmulatedWiFiDriver::NetworkIterator final : public DeviceLayer::NetworkCommissioning::NetworkIterator
{
public:
    explicit NetworkIterator(EmulatedWiFiDriver & driver) : mDriver(driver) {}

    size_t Count() override { return mDriver.mStaged.size(); }

    bool Next(Network & item) override
    {
        VerifyOrReturnValue(mIndex < mDriver.mStaged.size(), false);
        const NetworkEntry & network = mDriver.mStaged[mIndex++];

        memcpy(item.networkID, network.ssid, network.ssidLen);
        item.networkIDLen = network.ssidLen;
        item.connected    = mDriver.IsConnectedTo(network.Ssid());
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        item.networkIdentifier.ClearValue();
        item.clientIdentifier.ClearValue();
        if (network.UsingPDC())
        {
            ExtractKeyIdentifier(item.networkIdentifier, ByteSpan(network.networkIdentity, network.networkIdentityLen));
            ExtractKeyIdentifier(item.clientIdentifier, ByteSpan(network.clientIdentity, network.clientIdentityLen));
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        return true;
    }

    void Release() override { delete this; }

private:
    EmulatedWiFiDriver & mDriver;
    size_t mIndex = 0;
};

CHIP_ERROR EmulatedWiFiDriver::Init(NetworkStatusChangeCallback * /* networkStatusChangeCallback */)
{
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: Init");
    mStaged = mCommitted;
    return CHIP_NO_ERROR;
}

void EmulatedWiFiDriver::Shutdown() {}

DeviceLayer::NetworkCommissioning::NetworkIterator * EmulatedWiFiDriver::GetNetworks()
{
    // The cluster always consumes and releases the iterator synchronously, so mStaged
    // cannot change while the iterator is in use.
    return new NetworkIterator(*this);
}

CHIP_ERROR EmulatedWiFiDriver::CommitConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: CommitConfiguration (%u networks)",
                    static_cast<unsigned>(mStaged.size()));
    mCommitted = mStaged;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedWiFiDriver::RevertConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: RevertConfiguration (%u networks)",
                    static_cast<unsigned>(mCommitted.size()));
    mStaged = mCommitted;
    if (mConnectedSsidLen != 0 && FindStagedNetwork(ByteSpan(mConnectedSsid, mConnectedSsidLen)) == nullptr)
    {
        mConnectedSsidLen = 0;
    }
    return CHIP_NO_ERROR;
}

Status EmulatedWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                              uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    VerifyOrReturnError(credentials.size() <= sizeof(NetworkEntry::credentials), Status::kOutOfRange);

    uint8_t index;
    NetworkEntry network;
    Status status = PrepareAddOrUpdate(ssid, index, network);
    VerifyOrReturnValue(status == Status::kSuccess, status);

    memcpy(network.credentials, credentials.data(), credentials.size());
    network.credentialsLen = static_cast<uint8_t>(credentials.size());
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    network.networkIdentityLen = 0;
    network.clientIdentityLen  = 0;
    network.clientIdentityKeypair.reset();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    StoreStagedNetwork(index, network);
    outNetworkIndex = index;
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: AddOrUpdateNetwork '%s' at index %u", SsidForLog(ssid).c_str(),
                    index);
    return Status::kSuccess;
}

Status EmulatedWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    uint8_t index;
    VerifyOrReturnError(FindStagedNetwork(networkId, &index) != nullptr, Status::kNetworkIDNotFound);

    // Like a real station, removing the network configuration does not disconnect.
    mStaged.erase(mStaged.begin() + index);
    outNetworkIndex = index;
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: RemoveNetwork '%s' at index %u", SsidForLog(networkId).c_str(),
                    index);
    return Status::kSuccess;
}

Status EmulatedWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    uint8_t currentIndex;
    VerifyOrReturnError(FindStagedNetwork(networkId, &currentIndex) != nullptr, Status::kNetworkIDNotFound);
    VerifyOrReturnError(index < mStaged.size(), Status::kOutOfRange);

    auto current = mStaged.begin() + currentIndex;
    auto target  = mStaged.begin() + index;
    if (currentIndex < index)
    {
        std::rotate(current, current + 1, target + 1);
    }
    else
    {
        std::rotate(target, current, current + 1);
    }
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: ReorderNetwork '%s' from index %u to %u",
                    SsidForLog(networkId).c_str(), currentIndex, index);
    return Status::kSuccess;
}

void EmulatedWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    const NetworkEntry * network = FindStagedNetwork(networkId);
    Status status                = (network != nullptr) ? ResolveConnectOutcome(*network) : Status::kNetworkIDNotFound;
    if (status == Status::kSuccess)
    {
        memcpy(mConnectedSsid, network->ssid, network->ssidLen);
        mConnectedSsidLen = network->ssidLen;
    }

    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: ConnectNetwork '%s' -> %u", SsidForLog(networkId).c_str(),
                    to_underlying(status));
    callback->OnResult(status, CharSpan(), 0);
}

Status EmulatedWiFiDriver::ResolveConnectOutcome(const NetworkEntry & /* network */)
{
    return Status::kSuccess;
}

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
CHIP_ERROR EmulatedWiFiDriver::DisconnectFromNetwork()
{
    if (mConnectedSsidLen != 0)
    {
        ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: DisconnectFromNetwork '%s'",
                        SsidForLog(ByteSpan(mConnectedSsid, mConnectedSsidLen)).c_str());
        mConnectedSsidLen = 0;
    }
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION

void EmulatedWiFiDriver::ScanNetworks(ByteSpan ssid, ScanCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: ScanNetworks '%s' -> no results", SsidForLog(ssid).c_str());
    callback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
}

uint32_t EmulatedWiFiDriver::GetSupportedWiFiBandsMask() const
{
    return (1UL << to_underlying(WiFiBandEnum::k2g4)) | (1UL << to_underlying(WiFiBandEnum::k5g));
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
CHIP_ERROR EmulatedWiFiDriver::AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity,
                                                         Optional<uint8_t> clientIdentityNetworkIndex, Status & outStatus,
                                                         MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                                         uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outStatus = Status::kOutOfRange;
    VerifyOrReturnError(!networkIdentity.empty() && networkIdentity.size() <= sizeof(NetworkEntry::networkIdentity), CHIP_NO_ERROR);

    const NetworkEntry * reusedIdentityNetwork = nullptr;
    if (clientIdentityNetworkIndex.HasValue())
    {
        reusedIdentityNetwork = StagedPDCNetworkAt(clientIdentityNetworkIndex.Value());
        VerifyOrReturnError(reusedIdentityNetwork != nullptr, CHIP_NO_ERROR);
    }

    uint8_t index;
    NetworkEntry network;
    outStatus = PrepareAddOrUpdate(ssid, index, network);
    VerifyOrReturnError(outStatus == Status::kSuccess, CHIP_NO_ERROR);
    outStatus = Status::kUnknownError;

    network.credentialsLen = 0;
    memcpy(network.networkIdentity, networkIdentity.data(), networkIdentity.size());
    network.networkIdentityLen = static_cast<uint8_t>(networkIdentity.size());

    const bool reuseClientIdentity = (reusedIdentityNetwork != nullptr);
    if (reuseClientIdentity)
    {
        memcpy(network.clientIdentity, reusedIdentityNetwork->clientIdentity, reusedIdentityNetwork->clientIdentityLen);
        network.clientIdentityLen     = reusedIdentityNetwork->clientIdentityLen;
        network.clientIdentityKeypair = reusedIdentityNetwork->clientIdentityKeypair;
    }
    else
    {
        network.clientIdentityKeypair = Platform::MakeShared<Crypto::P256Keypair>();
        VerifyOrReturnError(network.clientIdentityKeypair, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(network.clientIdentityKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA));

        MutableByteSpan clientIdentity(network.clientIdentity);
        ReturnErrorOnFailure(Credentials::NewChipNetworkIdentity(*network.clientIdentityKeypair, clientIdentity));
        network.clientIdentityLen = static_cast<uint8_t>(clientIdentity.size());
    }

    ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(network.clientIdentity, network.clientIdentityLen), outClientIdentity));

    StoreStagedNetwork(index, network);
    outNetworkIndex = index;
    outStatus       = Status::kSuccess;
    ChipLogProgress(NetworkProvisioning, "EmulatedWiFiDriver: AddOrUpdateNetworkWithPDC '%s' at index %u (%s client identity)",
                    SsidForLog(ssid).c_str(), index, reuseClientIdentity ? "reused" : "new");
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmulatedWiFiDriver::GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
{
    const NetworkEntry * network = StagedPDCNetworkAt(networkIndex);
    VerifyOrReturnError(network != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(network->networkIdentity, network->networkIdentityLen), outNetworkIdentity);
}

CHIP_ERROR EmulatedWiFiDriver::GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
{
    const NetworkEntry * network = StagedPDCNetworkAt(networkIndex);
    VerifyOrReturnError(network != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(network->clientIdentity, network->clientIdentityLen), outClientIdentity);
}

CHIP_ERROR EmulatedWiFiDriver::SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                                      Crypto::P256ECDSASignature & outSignature)
{
    const NetworkEntry * network = StagedPDCNetworkAt(networkIndex);
    VerifyOrReturnError(network != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return network->clientIdentityKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

const EmulatedWiFiDriver::NetworkEntry * EmulatedWiFiDriver::StagedPDCNetworkAt(uint8_t networkIndex) const
{
    VerifyOrReturnValue(networkIndex < mStaged.size() && mStaged[networkIndex].UsingPDC(), nullptr);
    return &mStaged[networkIndex];
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

const EmulatedWiFiDriver::NetworkEntry * EmulatedWiFiDriver::FindStagedNetwork(ByteSpan ssid, uint8_t * outIndex) const
{
    for (size_t i = 0; i < mStaged.size(); i++)
    {
        if (mStaged[i].Matches(ssid))
        {
            if (outIndex != nullptr)
            {
                *outIndex = static_cast<uint8_t>(i);
            }
            return &mStaged[i];
        }
    }
    return nullptr;
}

Status EmulatedWiFiDriver::PrepareAddOrUpdate(ByteSpan ssid, uint8_t & outIndex, NetworkEntry & outNetwork) const
{
    VerifyOrReturnError(!ssid.empty() && ssid.size() <= sizeof(NetworkEntry::ssid), Status::kOutOfRange);

    const NetworkEntry * existing = FindStagedNetwork(ssid, &outIndex);
    if (existing != nullptr)
    {
        outNetwork = *existing;
        return Status::kSuccess;
    }

    VerifyOrReturnError(mStaged.size() < mConfig.maxNetworks, Status::kBoundsExceeded);
    outIndex = static_cast<uint8_t>(mStaged.size());
    memcpy(outNetwork.ssid, ssid.data(), ssid.size());
    outNetwork.ssidLen = static_cast<uint8_t>(ssid.size());
    return Status::kSuccess;
}

void EmulatedWiFiDriver::StoreStagedNetwork(uint8_t index, const NetworkEntry & network)
{
    if (index < mStaged.size())
    {
        mStaged[index] = network;
    }
    else
    {
        mStaged.push_back(network);
    }
}

} // namespace chip::app
