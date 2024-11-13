/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Pool.h>

#include <optional>

namespace chip {
namespace app {

/**
 * Stores Thread network information via a PersistentStorageDelegate.
 */
class DefaultThreadNetworkDirectoryStorage : public ThreadNetworkDirectoryStorage
{
public:
    DefaultThreadNetworkDirectoryStorage(PersistentStorageDelegate & storage) : mStorage(storage) {}

    uint8_t Capacity() override { return kCapacity; }
    ExtendedPanIdIterator * IterateNetworkIds() override;
    bool ContainsNetwork(const ExtendedPanId & exPanId) override;
    CHIP_ERROR GetNetworkDataset(const ExtendedPanId & exPanId, MutableByteSpan & dataset) override;
    CHIP_ERROR AddOrUpdateNetwork(const ExtendedPanId & exPanId, ByteSpan dataset) override;
    CHIP_ERROR RemoveNetwork(const ExtendedPanId & exPanId) override;

private:
    using index_t                         = uint8_t;
    static constexpr index_t kCapacity    = CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CAPACITY;
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_THREAD_NETWORK_DIRECTORY_STORAGE_CONCURRENT_ITERATORS;

    void InitializedIfNeeded();
    CHIP_ERROR StoreIndex();
    bool FindNetwork(const ExtendedPanId & exPanId, index_t & outIndex);

    struct IteratorImpl final : public ExtendedPanIdIterator
    {
        IteratorImpl(DefaultThreadNetworkDirectoryStorage & storage) : mContainer(storage) {}

        size_t Count() override { return mContainer.mCount; }

        bool Next(ExtendedPanId & item) override
        {
            VerifyOrReturnValue(mIndex < Count(), false);
            item = mContainer.mExtendedPanIds[mIndex++];
            return true;
        }

        void Release() override { mContainer.mIterators.ReleaseObject(this); }

        DefaultThreadNetworkDirectoryStorage & mContainer;
        index_t mIndex = 0;
    };

    PersistentStorageDelegate & mStorage;
    ObjectPool<IteratorImpl, kIteratorsMax> mIterators;
    ExtendedPanId mExtendedPanIds[kCapacity];
    index_t mCount    = 0;
    bool mInitialized = false;
};

} // namespace app
} // namespace chip
