/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

void NetworkEventHandler(const ChipDeviceEvent * event, intptr_t arg);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace {
inline constexpr uint8_t kMaxWiFiNetworks                  = 1;
inline constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
inline constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 20;
} // namespace

template <typename RawType, typename Converter>
class BflbScanResponseIterator : public Iterator<WiFiScanResponse>
{
public:
    BflbScanResponseIterator(size_t size, const RawType * pScanResults, Converter conv) :
        mSize(size), mpRawResults(pScanResults), mConvert(std::move(conv))
    {}
    size_t Count() override { return mSize; }
    void Release() override {}

    bool Next(WiFiScanResponse & item) override
    {
        if (mIternum >= mSize || mpRawResults == nullptr)
        {
            return false;
        }

        item = mConvert(mpRawResults[mIternum]);
        ++mIternum;
        return true;
    }

private:
    const size_t mSize;
    size_t mIternum = 0;

    const RawType * mpRawResults;
    Converter mConvert;
};

template <typename RawType, typename Converter>
auto makeBflbScanIterator(size_t count, const RawType * results, Converter && conv)
{
    return BflbScanResponseIterator<RawType, std::decay_t<Converter>>(count, results, std::forward<Converter>(conv));
}

class BflbWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(BflbWiFiDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        BflbWiFiDriver * mDriver;
        bool mExhausted = false;
    };

    struct WiFiNetwork
    {
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        char credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
    };
    enum WiFiCredentialLength
    {
        kOpen      = 0,
        kWEP64     = 5,
        kMinWPAPSK = 8,
        kMaxWPAPSK = 63,
        kWPAPSKHex = 64,
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new WiFiNetworkIterator(this); }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown();

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
    void OnConnectWiFiNetwork(bool isConnected);
    void OnScanWiFiNetworkDone(void);
    void OnNetworkStatusChange(void);
    CHIP_ERROR SetLastDisconnectReason(const ChipDeviceEvent * event);

    void OnScanWiFiNetworkDone(void * opaque);
    int32_t GetLastDisconnectReason();

    static BflbWiFiDriver & GetInstance()
    {
        static BflbWiFiDriver instance;
        return instance;
    }

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);

#if CHIP_DEVICE_LAYER_TARGET_BL702
    WiFiScanResponse * mScanResponse = nullptr;
    size_t mScanResponseNum          = 0;
#endif

    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
    ScanCallback * mpScanCallback;
    ConnectCallback * mpConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;
    int32_t mLastDisconnectedReason;

    char mScanSSID[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    int mScanSSIDlength;
};
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
class BflbEthernetDriver final : public EthernetDriver
{
public:
    struct EthernetNetworkIterator final : public NetworkIterator
    {
        EthernetNetworkIterator() = default;
        size_t Count() override { return interfaceNameLen > 0 ? 1 : 0; }
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
        ~EthernetNetworkIterator() override = default;

        // Public, but cannot be accessed via NetworkIterator interface.
        uint8_t interfaceName[kMaxNetworkIDLen];
        uint8_t interfaceNameLen = 0;
        bool exhausted           = false;
    };

    uint8_t GetMaxNetworks() override { return 1; };
    NetworkIterator * GetNetworks() override;
    CHIP_ERROR Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;

    static BflbEthernetDriver & GetInstance()
    {
        static BflbEthernetDriver instance;
        return instance;
    }
};
#endif

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
