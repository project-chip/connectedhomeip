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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include <platform/NetworkCommissioning.h>
#include <wfx_host_events.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
} // namespace

template <typename T>
class SlScanResponseIterator : public Iterator<T>
{
public:
    SlScanResponseIterator(T * apScanResponse) : mpScanResponse(apScanResponse) {}
    size_t Count() override { return itemCount; }
    bool Next(T & item) override
    {
        if (mpScanResponse == nullptr || currentIterating >= itemCount)
        {
            return false;
        }
        item = mpScanResponse[currentIterating];
        currentIterating++;
        return true;
    }
    void Release() override
    {
        itemCount = currentIterating = 0;
        Platform::MemoryFree(mpScanResponse);
        mpScanResponse = nullptr;
    }

    void Add(T * pResponse)
    {
        size_t tempCount = itemCount + 1;
        mpScanResponse   = static_cast<T *>(Platform::MemoryRealloc(mpScanResponse, kItemSize * tempCount));
        if (mpScanResponse)
        {
            // first item at index. update after the copy.
            memcpy(&(mpScanResponse[itemCount]), pResponse, kItemSize);
            itemCount = tempCount;
        }
    }

private:
    size_t currentIterating           = 0;
    size_t itemCount                  = 0;
    static constexpr size_t kItemSize = sizeof(T);
    T * mpScanResponse;
};

class SlWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(SlWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        SlWiFiDriver * mDriver;
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

    chip::BitFlags<WiFiSecurity> ConvertSecuritytype(wfx_sec_t security);

    void OnConnectWiFiNetwork();
    void UpdateNetworkingStatus();
    static SlWiFiDriver & GetInstance()
    {
        static SlWiFiDriver instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);
    bool StartScanWiFiNetworks(ByteSpan ssid);
    static void OnScanWiFiNetworkDone(wfx_wifi_scan_result_t * aScanResult);

    WiFiNetwork mSavedNetwork   = {};
    WiFiNetwork mStagingNetwork = {};
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
#endif
