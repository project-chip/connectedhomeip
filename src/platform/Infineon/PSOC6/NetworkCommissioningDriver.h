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
#include <cy_wcm.h>
#include <lib/core/ClusterEnums.h>
#include <lib/support/BitFlags.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
constexpr uint8_t kWiFiMaxNetworks                  = 15;
} // namespace

class P6ScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    P6ScanResponseIterator(const size_t size, const cy_wcm_scan_result_t * scanResults) : mSize(size), mpScanResults(scanResults) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }

        item.security.SetRaw(mpScanResults[mIternum].security);
        item.ssidLen =
            strnlen(reinterpret_cast<const char *>(mpScanResults[mIternum].SSID), chip::DeviceLayer::Internal::kMaxWiFiSSIDLength);
        item.channel  = mpScanResults[mIternum].channel;
        item.wiFiBand = (mpScanResults[mIternum].band == CY_WCM_WIFI_BAND_2_4GHZ)
            ? chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4
            : chip::DeviceLayer::NetworkCommissioning::WiFiBand::k5g;
        item.rssi = mpScanResults[mIternum].signal_strength;
        memcpy(item.ssid, mpScanResults[mIternum].SSID, item.ssidLen);
        memcpy(item.bssid, mpScanResults[mIternum].BSSID, 6);

        mIternum++;
        return true;
    }
    void Release() override {}

private:
    const size_t mSize;                         // no of network scanned
    const cy_wcm_scan_result_t * mpScanResults; // list of scanned network info of size mSize
    size_t mIternum = 0;                        // to iterate through mpScanResults of size mSize
};

class P6WiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(P6WiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        P6WiFiDriver * mDriver;
        bool mExhausted = false;
    };

    struct WiFiNetwork
    {
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        char credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
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

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                              uint8_t & outNetworkIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;

    CHIP_ERROR ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen);

    static void scan_result_callback(cy_wcm_scan_result_t * result_ptr, void * user_data, cy_wcm_scan_status_t status);
    BitFlags<app::Clusters::NetworkCommissioning::WiFiSecurity> ConvertSecuritytype(cy_wcm_security_t security);

    void OnConnectWiFiNetwork();
    void OnScanWiFiNetworkDone();
    void OnNetworkStatusChange();

    CHIP_ERROR SetLastDisconnectReason(int32_t reason);
    int32_t GetLastDisconnectReason();

    static P6WiFiDriver & GetInstance()
    {
        static P6WiFiDriver instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);
    CHIP_ERROR StartScanWiFiNetworks(ByteSpan ssid);

    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
    int32_t mLastDisconnectedReason;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
