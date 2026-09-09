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

#pragma once
#include <esp_wifi.h>
#include <platform/NetworkCommissioning.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/ScopedMemoryBuffer.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

using chip::BitFlags;
using chip::app::Clusters::NetworkCommissioning::WiFiSecurityBitmap;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
namespace {
inline constexpr uint8_t kMaxWiFiNetworks                  = 1;
inline constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
inline constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 30;
} // namespace

BitFlags<WiFiSecurityBitmap> ConvertSecurityType(wifi_auth_mode_t authMode);

class ESPScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
    ESPScanResponseIterator(const size_t size) : mSize(size) {}
#else
    ESPScanResponseIterator(const size_t size, const wifi_ap_record_t * scanResults) : mSize(size), mpScanResults(scanResults) {}
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
    size_t Count() override { return mSize; }

    bool Next(WiFiScanResponse & item) override
    {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
        wifi_ap_record_t ap_record;
        VerifyOrReturnValue(esp_wifi_scan_get_ap_record(&ap_record) == ESP_OK, false);
        SetApData(item, ap_record);
#else
        if (mIternum >= mSize)
        {
            return false;
        }
        SetApData(item, mpScanResults[mIternum]);
        mIternum++;
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
        return true;
    }

    void Release() override
    {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
        esp_wifi_clear_ap_list();
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
    }

private:
    const size_t mSize;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 3)
    const wifi_ap_record_t * mpScanResults;
    size_t mIternum = 0;
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 3)

    void SetApData(WiFiScanResponse & item, wifi_ap_record_t ap_record)
    {
        item.security = ConvertSecurityType(ap_record.authmode);
        static_assert(chip::DeviceLayer::Internal::kMaxWiFiSSIDLength <= UINT8_MAX, "SSID length might not fit in item.ssidLen");
        item.ssidLen = static_cast<uint8_t>(
            strnlen(reinterpret_cast<const char *>(ap_record.ssid), chip::DeviceLayer::Internal::kMaxWiFiSSIDLength));
        item.channel         = ap_record.primary;
        item.wiFiBand        = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
        item.signal.type     = NetworkCommissioning::WirelessSignalType::kdBm;
        item.signal.strength = ap_record.rssi;
        memcpy(item.ssid, ap_record.ssid, item.ssidLen);
        memcpy(item.bssid, ap_record.bssid, sizeof(item.bssid));

        // The raw authmode helps debug how the Wi-Fi driver classifies the RSN configuration of a network.
        ChipLogProgress(NetworkProvisioning, "Scan result: \"%.*s\" authmode %u", static_cast<int>(item.ssidLen),
                        reinterpret_cast<const char *>(item.ssid), static_cast<unsigned>(ap_record.authmode));
    }
};

class ESPWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(ESPWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        ESPWiFiDriver * mDriver;
        bool mExhausted = false;
    };

    struct WiFiNetwork
    {
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        char credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        bool UsingPDC() const { return networkIdentityLen != 0; }

        uint8_t networkIdentity[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        uint8_t networkIdentityLen = 0;
        uint8_t clientIdentity[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        uint8_t clientIdentityLen = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new WiFiNetworkIterator(this); }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return kMaxWiFiNetworks; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return kWiFiScanNetworksTimeOutSeconds; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return kWiFiConnectNetworkTimeoutSeconds; }

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
    bool SupportsPerDeviceCredentials() override { return true; };
    CHIP_ERROR AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity, Optional<uint8_t> clientIdentityNetworkIndex,
                                         Status & outStatus, MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                         uint8_t & outNetworkIndex) override;
    CHIP_ERROR GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity) override;
    CHIP_ERROR GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity) override;
    CHIP_ERROR SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                      Crypto::P256ECDSASignature & outSignature) override;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    CHIP_ERROR ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen);
    void OnConnectWiFiNetwork();
    void OnConnectWiFiNetworkFailed();
    static void OnConnectWiFiNetworkFailed(chip::System::Layer * aLayer, void * aAppState);
    void OnScanWiFiNetworkDone();
    void OnNetworkStatusChange();

    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);
    uint16_t GetLastDisconnectReason();

    static ESPWiFiDriver & GetInstance()
    {
        static ESPWiFiDriver instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);
    CHIP_ERROR StartScanWiFiNetworks(ByteSpan ssid);
    CHIP_ERROR BackupConfiguration();

    // Disconnects and clears the ESP station configuration, so that a new one can be applied.
    static CHIP_ERROR DisableStation();

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // Associates with mStagingNetwork using EAP-TLS, authenticating with the Client Identity
    // and validating the access point against the Network Identity.
    CHIP_ERROR ConnectWiFiNetworkWithPDC();
    CHIP_ERROR InstallEapTlsCredentials();
    void ReleaseEapTlsCredentials();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    WiFiNetwork mStagingNetwork;
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
    uint16_t mLastDisconnectedReason;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // Only valid while mStagingNetwork.UsingPDC(); held here rather than in WiFiNetwork
    // so that copying a WiFiNetwork stays a trivial (heap-free) operation.
    Crypto::P256Keypair mClientIdentityKeypair;

    // DER encodings of the Network Identity, the Client Identity and its key, laid out back to
    // back. esp_eap_client retains the pointers it is handed rather than copying, and reads
    // them again on every (re-)association, so they must outlive the connection. Allocated by
    // InstallEapTlsCredentials(), which documents the layout.
    Platform::ScopedMemoryBuffer<uint8_t> mEapTlsCredentials;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
};

class ESPEthernetDriver : public EthernetDriver
{
public:
    class EthernetNetworkIterator final : public NetworkIterator
    {
    public:
        EthernetNetworkIterator(ESPEthernetDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() { return 1; }
        bool Next(Network & item) override
        {
            if (exhausted)
            {
                return false;
            }
            exhausted = true;
            memcpy(item.networkID, interfaceName, interfaceNameLen);
            item.networkIDLen = interfaceNameLen;
            item.connected    = true;
            return true;
        }
        void Release() override { delete this; }
        ~EthernetNetworkIterator() = default;

        uint8_t interfaceName[kMaxNetworkIDLen];
        uint8_t interfaceNameLen = 0;
        bool exhausted           = false;

    private:
        ESPEthernetDriver * mDriver;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new EthernetNetworkIterator(this); }
    uint8_t GetMaxNetworks() { return 1; }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown()
    {
        // TODO: This method can be implemented if Ethernet is used along with Wifi/Thread.
    }

    static ESPEthernetDriver & GetInstance()
    {
        static ESPEthernetDriver instance;
        return instance;
    }
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
