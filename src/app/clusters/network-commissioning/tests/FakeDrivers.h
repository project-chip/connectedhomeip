/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <clusters/NetworkCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/NetworkCommissioning.h>
#include <protocols/interaction_model/StatusCode.h>

#include <vector>

namespace chip {
namespace Testing {

class FakeEthernetDriver : public DeviceLayer::NetworkCommissioning::EthernetDriver
{
public:
    class FakeNetworkIterator : public DeviceLayer::NetworkCommissioning::NetworkIterator
    {
    public:
        void Set(Span<DeviceLayer::NetworkCommissioning::Network> networks)
        {
            mNetworks    = networks;
            currentindex = 0;
        }
        bool Next(DeviceLayer::NetworkCommissioning::Network & item) override
        {
            VerifyOrReturnValue(currentindex < mNetworks.size(), false);
            item = mNetworks[currentindex++];
            return true;
        }
        size_t Count() override { return mNetworks.size(); }
        void Release() override {}

    private:
        Span<DeviceLayer::NetworkCommissioning::Network> mNetworks;
        size_t currentindex = 0;
    };

    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override
    {
        mNetworkStatusChangeCallback = networkStatusChangeCallback;
        return CHIP_NO_ERROR;
    }

    uint8_t GetMaxNetworks() override { return 1; };

    void SetNetwork(ByteSpan interfaceName)
    {
        auto span = MutableByteSpan{ mNetwork.networkID, sizeof(mNetwork.networkID) };
        SuccessOrDie(CopySpanToMutableSpan(interfaceName, span));
        mNetwork.networkIDLen = static_cast<uint8_t>(interfaceName.size());
        mNetwork.connected    = true;
        mNetworkCount         = 1;

        mNetworkStatusChangeCallback->OnNetworkingStatusChange(
            app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum::kSuccess, Optional{ interfaceName },
            testErrorValue);
    }

    void SetNetworkConnected(bool connected) { mNetwork.connected = connected; }

    void SetNoNetwork() { mNetworkCount = 0; }

    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override
    {
        if (mNetworkCount == 0)
        {
            mNetworkIterator.Set({});
        }
        else
        {
            mNetworkIterator.Set({ &mNetwork, 1 });
        }

        return &mNetworkIterator;
    };

    void EnableDisabling(bool enabledAllowed) { mSetEnabledAllowed = enabledAllowed; }

    CHIP_ERROR SetEnabled(bool enabled) override
    {
        if (GetEnabled() == enabled)
        {
            return CHIP_NO_ERROR;
        }
        if (mSetEnabledAllowed)
        {
            mEnabled = enabled;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    bool GetEnabled() override { return mEnabled; };

    Optional<int32_t> testErrorValue{ 123 };

private:
    NetworkStatusChangeCallback * mNetworkStatusChangeCallback;
    FakeNetworkIterator mNetworkIterator;
    DeviceLayer::NetworkCommissioning::Network mNetwork;
    size_t mNetworkCount    = 0;
    bool mSetEnabledAllowed = false;
    bool mEnabled           = true;
};

class FakeWiFiDriver : public DeviceLayer::NetworkCommissioning::WiFiDriver
{
public:
    using NetworkCommissioningStatusEnum = app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum;

    // WiFiDriver
    NetworkCommissioningStatusEnum AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                                      uint8_t & outNetworkIndex) override
    {
        return mAddOrUpdateStatus;
    }
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override {}

    // Internal::WirelessDriver
    CHIP_ERROR CommitConfiguration() override { return CHIP_NO_ERROR; }
    CHIP_ERROR RevertConfiguration() override
    {
        mRevertConfigurationCalled = true;
        return CHIP_NO_ERROR;
    }

    uint8_t GetScanNetworkTimeoutSeconds() override { return 2; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return 2; }

    void SetAddOrUpdateNetworkReturn(NetworkCommissioningStatusEnum value) { mAddOrUpdateStatus = value; }
    void SetEnabledAllowed(bool enabledAllowed) { mEnabledAllowed = enabledAllowed; }

    NetworkCommissioningStatusEnum RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText,
                                                 uint8_t & outNetworkIndex) override
    {
        return NetworkCommissioningStatusEnum::kSuccess;
    }
    NetworkCommissioningStatusEnum ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override
    {
        return NetworkCommissioningStatusEnum::kSuccess;
    }

    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override
    {
        callback->OnResult(NetworkCommissioningStatusEnum::kSuccess, ""_span, 0);
    }

    CHIP_ERROR SetEnabled(bool enabled) override
    {
        if (mEnabledAllowed)
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    // BaseDriver
    uint8_t GetMaxNetworks() override { return 0; }
    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override { return nullptr; }

private:
    NetworkCommissioningStatusEnum mAddOrUpdateStatus = NetworkCommissioningStatusEnum::kUnknownError;
    bool mEnabledAllowed                              = false;

public:
    bool mRevertConfigurationCalled = false;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
/// A WiFi driver that claims Per-Device Credentials support and records what the cluster
/// handed down to it. Whether the identities involved actually make sense is the real
/// driver's problem, so this one just echoes back whatever it was told to return.
class FakePDCWiFiDriver : public FakeWiFiDriver
{
public:
    bool SupportsPerDeviceCredentials() override { return true; }

    CHIP_ERROR AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity, Optional<uint8_t> clientIdentityNetworkIndex,
                                         DeviceLayer::NetworkCommissioning::Status & outStatus, MutableCharSpan & outDebugText,
                                         MutableByteSpan & outClientIdentity, uint8_t & outNetworkIndex) override
    {
        mSsid.assign(ssid.begin(), ssid.end());
        mNetworkIdentity.assign(networkIdentity.begin(), networkIdentity.end());
        mClientIdentityNetworkIndex = clientIdentityNetworkIndex;

        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(mClientIdentity.data(), mClientIdentity.size()), outClientIdentity));
        outDebugText.reduce_size(0);
        outStatus       = mAddOrUpdateWithPDCStatus;
        outNetworkIndex = mNetworkIndex;
        return CHIP_NO_ERROR;
    }

    /// The Client Identity the driver hands back on success.
    void SetClientIdentity(ByteSpan identity) { mClientIdentity.assign(identity.begin(), identity.end()); }

    ByteSpan GetLastSsid() const { return ByteSpan(mSsid.data(), mSsid.size()); }
    ByteSpan GetLastNetworkIdentity() const { return ByteSpan(mNetworkIdentity.data(), mNetworkIdentity.size()); }
    Optional<uint8_t> GetLastClientIdentityNetworkIndex() const { return mClientIdentityNetworkIndex; }

private:
    std::vector<uint8_t> mClientIdentity;
    std::vector<uint8_t> mSsid;
    std::vector<uint8_t> mNetworkIdentity;
    Optional<uint8_t> mClientIdentityNetworkIndex;
    NetworkCommissioningStatusEnum mAddOrUpdateWithPDCStatus = NetworkCommissioningStatusEnum::kSuccess;
    uint8_t mNetworkIndex                                    = 0;
};
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

} // namespace Testing
} // namespace chip
