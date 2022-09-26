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

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

template <typename T>
class otScanResponseIterator : public Iterator<T>
{
public:
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
    T * mpScanResponse                = nullptr;
};

class GenericThreadDriver final : public ThreadDriver
{
public:
    class ThreadNetworkIterator final : public NetworkIterator
    {
    public:
        ThreadNetworkIterator(GenericThreadDriver * aDriver) : driver(aDriver) {}
        size_t Count() override;
        bool Next(Network & item) override;
        void Release() override { delete this; }
        ~ThreadNetworkIterator() = default;

    private:
        GenericThreadDriver * driver;
        bool exhausted = false;
    };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new ThreadNetworkIterator(this); }
    CHIP_ERROR Init(Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback) override;
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
    Status MatchesNetworkId(const Thread::OperationalDataset & dataset, const ByteSpan & networkId) const;
    CHIP_ERROR BackupConfiguration();

    ThreadNetworkIterator mThreadIterator      = ThreadNetworkIterator(this);
    Thread::OperationalDataset mStagingNetwork = {};
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
