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

#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryStorage.h>

#include <cstring>
#include <vector>

namespace chip {
namespace app {
namespace Testing {

/**
 * An in-memory implementation of ThreadNetworkDirectoryStorage for use in unit tests.
 *
 * Uses std::vector internally so it is NOT production-suitable. Intended for testing only.
 */
class FakeThreadNetworkDirectoryStorage : public ThreadNetworkDirectoryStorage
{
public:
    static constexpr uint8_t kDefaultCapacity = 4;

    explicit FakeThreadNetworkDirectoryStorage(uint8_t capacity = kDefaultCapacity) : mCapacity(capacity) {}

    uint8_t Capacity() override { return mCapacity; }

    ExtendedPanIdIterator * IterateNetworkIds() override { return new IteratorImpl(mNetworks); }

    CHIP_ERROR GetNetworkDataset(const ExtendedPanId & exPanId, MutableByteSpan & dataset) override
    {
        for (const auto & entry : mNetworks)
        {
            if (entry.panId == exPanId)
            {
                if (dataset.size() < entry.dataset.size())
                {
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
                memcpy(dataset.data(), entry.dataset.data(), entry.dataset.size());
                dataset.reduce_size(entry.dataset.size());
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR AddOrUpdateNetwork(const ExtendedPanId & exPanId, ByteSpan dataset) override
    {
        if (dataset.empty() || dataset.size() > kMaxThreadDatasetLen)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        for (auto & entry : mNetworks)
        {
            if (entry.panId == exPanId)
            {
                entry.dataset.assign(dataset.data(), dataset.data() + dataset.size());
                return CHIP_NO_ERROR;
            }
        }

        if (static_cast<uint8_t>(mNetworks.size()) >= mCapacity)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        mNetworks.push_back({ exPanId, std::vector<uint8_t>(dataset.data(), dataset.data() + dataset.size()) });
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveNetwork(const ExtendedPanId & exPanId) override
    {
        for (auto it = mNetworks.begin(); it != mNetworks.end(); ++it)
        {
            if (it->panId == exPanId)
            {
                mNetworks.erase(it);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

private:
    struct NetworkEntry
    {
        ExtendedPanId panId;
        std::vector<uint8_t> dataset;
    };

    class IteratorImpl : public ExtendedPanIdIterator
    {
    public:
        explicit IteratorImpl(const std::vector<NetworkEntry> & networks) : mNetworks(networks), mIndex(0) {}

        size_t Count() override { return mNetworks.size(); }

        bool Next(ExtendedPanId & item) override
        {
            if (mIndex < mNetworks.size())
            {
                item = mNetworks[mIndex++].panId;
                return true;
            }
            return false;
        }

        void Release() override { delete this; }

    private:
        // Copy of the network list to ensure stable iteration even if the storage is modified.
        const std::vector<NetworkEntry> mNetworks;
        size_t mIndex;
    };

    uint8_t mCapacity;
    std::vector<NetworkEntry> mNetworks;
};

} // namespace Testing
} // namespace app
} // namespace chip
