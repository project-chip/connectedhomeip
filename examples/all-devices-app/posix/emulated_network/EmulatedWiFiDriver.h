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

#pragma once

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <platform/NetworkCommissioning.h>

#include <cstdint>
#include <vector>

namespace chip::app {

/**
 * A WiFiDriver that keeps network configurations in memory and never touches a real
 * network interface. ConnectNetwork succeeds for any configured network.
 *
 * This allows the Network Commissioning cluster to be exercised on a host that is
 * already connected to the network by other means, e.g. a test harness machine.
 *
 * All methods must be called on the Matter thread. ConnectNetwork and ScanNetworks
 * invoke their callback synchronously, before returning.
 */
class EmulatedWiFiDriver final : public DeviceLayer::NetworkCommissioning::WiFiDriver
{
public:
    using Status = DeviceLayer::NetworkCommissioning::Status;

    /**
     * Behaviour settings. The driver reads these when it needs them, so they can be
     * changed at runtime via `GetConfig()` unless noted otherwise.
     */
    struct Config
    {
        /// Reported as MaxNetworks. Lowering it does not remove existing networks.
        uint8_t maxNetworks = 1;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        /// The cluster reads this once when it is constructed, so changes after
        /// that have no effect.
        bool supportsPerDeviceCredentials = true;
#endif
    };

    EmulatedWiFiDriver() = default;
    explicit EmulatedWiFiDriver(const Config & config) : mConfig(config) {}

    Config & GetConfig() { return mConfig; }

    // BaseDriver
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;
    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return mConfig.maxNetworks; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return 10; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return 20; }
    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;
    Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override;
    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override;
#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    CHIP_ERROR DisconnectFromNetwork() override;
#endif

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                              uint8_t & outNetworkIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;
    uint32_t GetSupportedWiFiBandsMask() const override;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    bool SupportsPerDeviceCredentials() override { return mConfig.supportsPerDeviceCredentials; }
    CHIP_ERROR AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity, Optional<uint8_t> clientIdentityNetworkIndex,
                                         Status & outStatus, MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                         uint8_t & outNetworkIndex) override;
    CHIP_ERROR GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity) override;
    CHIP_ERROR GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity) override;
    CHIP_ERROR SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                      Crypto::P256ECDSASignature & outSignature) override;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

private:
    struct NetworkEntry
    {
        ByteSpan Ssid() const { return ByteSpan(ssid, ssidLen); }
        bool Matches(ByteSpan aSsid) const { return Ssid().data_equal(aSsid); }

        uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;

        uint8_t credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        bool UsingPDC() const { return networkIdentityLen != 0; }

        uint8_t networkIdentity[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        uint8_t networkIdentityLen = 0;

        uint8_t clientIdentity[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        uint8_t clientIdentityLen = 0;

        // Shared between copies of the entry; the keypair never changes after creation.
        Platform::SharedPtr<Crypto::P256Keypair> clientIdentityKeypair;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    };

    class NetworkIterator;

    const NetworkEntry * FindStagedNetwork(ByteSpan ssid, uint8_t * outIndex = nullptr) const;
    // Validates the SSID and finds the staged entry to update, or the index to append at.
    // outNetwork is a copy of the existing entry, or a new entry with only the SSID set.
    Status PrepareAddOrUpdate(ByteSpan ssid, uint8_t & outIndex, NetworkEntry & outNetwork) const;
    void StoreStagedNetwork(uint8_t index, const NetworkEntry & network);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    const NetworkEntry * StagedPDCNetworkAt(uint8_t networkIndex) const;
#endif
    bool IsConnectedTo(ByteSpan ssid) const
    {
        return mConnectedSsidLen != 0 && ByteSpan(mConnectedSsid, mConnectedSsidLen).data_equal(ssid);
    }
    Status ResolveConnectOutcome(const NetworkEntry & network);

    Config mConfig;
    std::vector<NetworkEntry> mCommitted;
    std::vector<NetworkEntry> mStaged; // edited by commands and reported by GetNetworks
    uint8_t mConnectedSsid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    uint8_t mConnectedSsidLen = 0; // 0 when not connected
};

} // namespace chip::app
