/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <string>
#include <vector>

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
class TizenWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(TizenWiFiDriver * aDriver) : driver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() = default;

    private:
        TizenWiFiDriver * driver;
        bool exhausted = false;
    };

    struct WiFiNetwork
    {
        uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
        uint8_t ssidLen = 0;
        uint8_t credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
        uint8_t credentialsLen = 0;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new WiFiNetworkIterator(this); }
    CHIP_ERROR Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback) override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return 1; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return 10; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return 20; }

    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;

    Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override;
    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override;

    // WiFiDriver
    Status AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                              uint8_t & outNetworkIndex) override;
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override;

private:
    bool NetworkMatch(const WiFiNetwork & network, ByteSpan networkId);

    WiFiNetwork mSavedNetwork;
    WiFiNetwork mStagingNetwork;
};
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
class TizenThreadDriver final : public ThreadDriver
{
public:
    class ThreadNetworkIterator final : public NetworkIterator
    {
    public:
        ThreadNetworkIterator(TizenThreadDriver * aDriver) : driver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~ThreadNetworkIterator() = default;

    private:
        TizenThreadDriver * driver;
        bool exhausted = false;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new ThreadNetworkIterator(this); }
    CHIP_ERROR Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback) override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return 1; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return 10; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return 20; }

    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;

    Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override;
    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override;

    // ThreadDriver
    Status AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    void ScanNetworks(ThreadDriver::ScanCallback * callback) override;

private:
    ThreadNetworkIterator mThreadIterator = ThreadNetworkIterator(this);
    Thread::OperationalDataset mSavedNetwork;
    Thread::OperationalDataset mStagingNetwork;
};

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

class TizenEthernetDriver final : public EthernetDriver
{
public:
    class EthernetNetworkIterator final : public NetworkIterator
    {
    public:
        EthernetNetworkIterator(TizenEthernetDriver * aDriver);
        ~EthernetNetworkIterator() override = default;
        size_t Count() override { return mInterfaces.size(); }
        bool Next(Network & item) override;
        void Release() override { delete this; }

    private:
        TizenEthernetDriver * mDriver;
        std::vector<std::string> mInterfaces;
        size_t mInterfacesIdx = 0;
    };

    uint8_t GetMaxNetworks() override { return 1; };
    NetworkIterator * GetNetworks() override { return new EthernetNetworkIterator(this); };
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
