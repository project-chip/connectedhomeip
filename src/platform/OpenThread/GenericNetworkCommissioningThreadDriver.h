/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    GenericThreadDriver(uint8_t scanTimeoutSec = 10, uint8_t connectTimeoutSec = 20)
    {
        scanNetworkTimeoutSeconds = scanTimeoutSec;
        connectNetworkTimeout     = connectTimeoutSec;
    }

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new ThreadNetworkIterator(this); }
    CHIP_ERROR Init(Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback) override;
    void Shutdown() override;

    // WirelessDriver
    uint8_t GetMaxNetworks() override { return 1; }
    uint8_t GetScanNetworkTimeoutSeconds() override { return scanNetworkTimeoutSeconds; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return connectNetworkTimeout; }

    void SetScanNetworkTimeoutSeconds(uint8_t scanTimeoutSec) { scanNetworkTimeoutSeconds = scanTimeoutSec; }
    void SetConnectNetworkTimeoutSeconds(uint8_t connectTimeoutSec) { connectNetworkTimeout = connectTimeoutSec; }

    CHIP_ERROR CommitConfiguration() override;
    CHIP_ERROR RevertConfiguration() override;

    Status RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    Status ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override;
    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override;

    // ThreadDriver
    Status AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex) override;
    void ScanNetworks(ThreadDriver::ScanCallback * callback) override;

private:
    uint8_t scanNetworkTimeoutSeconds;
    uint8_t connectNetworkTimeout;
    Status MatchesNetworkId(const Thread::OperationalDataset & dataset, const ByteSpan & networkId) const;
    CHIP_ERROR BackupConfiguration();

    ThreadNetworkIterator mThreadIterator      = ThreadNetworkIterator(this);
    Thread::OperationalDataset mStagingNetwork = {};
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
