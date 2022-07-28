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

#include <platform/NetworkCommissioning.h>
#include <vector>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

template <typename T>
class LinuxScanResponseIterator : public Iterator<T>
{
public:
    LinuxScanResponseIterator(std::vector<T> * apScanResponse) : mpScanResponse(apScanResponse) {}
    size_t Count() override { return mpScanResponse != nullptr ? mpScanResponse->size() : 0; }
    bool Next(T & item) override
    {
        if (mpScanResponse == nullptr || currentIterating >= mpScanResponse->size())
        {
            return false;
        }
        item = (*mpScanResponse)[currentIterating];
        currentIterating++;
        return true;
    }
    void Release() override
    { /* nothing to do, we don't hold the ownership of the vector, and users is not expected to hold the ownership in OnFinished for
         scan. */
    }

private:
    size_t currentIterating = 0;
    // Note: We cannot post a event in ScheduleLambda since std::vector is not trivial copyable.
    std::vector<T> * mpScanResponse;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
class LinuxWiFiDriver final : public WiFiDriver
{
public:
    class WiFiNetworkIterator final : public NetworkIterator
    {
    public:
        WiFiNetworkIterator(LinuxWiFiDriver * aDriver) : driver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~WiFiNetworkIterator() override = default;

    private:
        LinuxWiFiDriver * driver;
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
    void Shutdown() override;

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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
class LinuxThreadDriver final : public ThreadDriver
{
public:
    class ThreadNetworkIterator final : public NetworkIterator
    {
    public:
        ThreadNetworkIterator(LinuxThreadDriver * aDriver) : driver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~ThreadNetworkIterator() override = default;

    private:
        LinuxThreadDriver * driver;
        bool exhausted = false;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new ThreadNetworkIterator(this); }
    CHIP_ERROR Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown() override;

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

class LinuxEthernetDriver final : public EthernetDriver
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
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
