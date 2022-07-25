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

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 30;
} // namespace

class ESPScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    ESPScanResponseIterator(const size_t size, const wifi_ap_record_t * scanResults) : mSize(size), mpScanResults(scanResults) {}
    size_t Count() override { return mSize; }
    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize)
        {
            return false;
        }

        item.security.SetRaw(mpScanResults[mIternum].authmode);
        item.ssidLen =
            strnlen(reinterpret_cast<const char *>(mpScanResults[mIternum].ssid), chip::DeviceLayer::Internal::kMaxWiFiSSIDLength);
        item.channel  = mpScanResults[mIternum].primary;
        item.wiFiBand = chip::DeviceLayer::NetworkCommissioning::WiFiBand::k2g4;
        item.rssi     = mpScanResults[mIternum].rssi;
        memcpy(item.ssid, mpScanResults[mIternum].ssid, item.ssidLen);
        memcpy(item.bssid, mpScanResults[mIternum].bssid, 6);

        mIternum++;
        return true;
    }
    void Release() override {}

private:
    const size_t mSize;
    const wifi_ap_record_t * mpScanResults;
    size_t mIternum = 0;
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
    void OnConnectWiFiNetwork();
    void OnConnectWiFiNetworkFailed();
    static void OnConnectWiFiNetworkFailed(chip::System::Layer * aLayer, void * aAppState);
    void OnScanWiFiNetworkDone();
    void OnNetworkStatusChange();

    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);
    int32_t GetLastDisconnectReason();

    static ESPWiFiDriver & GetInstance()
    {
        static ESPWiFiDriver instance;
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
