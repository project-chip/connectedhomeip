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

#include "DefaultThreadNetworkDirectoryStorage.h"
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {

void DefaultThreadNetworkDirectoryStorage::InitializedIfNeeded()
{
    VerifyOrReturn(!mInitialized);
    mInitialized = true;

    CHIP_ERROR err;
    StorageKeyName key = DefaultStorageKeyAllocator::ThreadNetworkDirectoryIndex();
    uint16_t size      = sizeof(mExtendedPanIds);
    SuccessOrExit(err = mStorage.SyncGetKeyValue(key.KeyName(), mExtendedPanIds, size));
    VerifyOrExit(size % ExtendedPanId::size() == 0, err = CHIP_ERROR_INTERNAL);
    mCount = static_cast<index_t>(size / ExtendedPanId::size());
    return;
exit:
    if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "Failed to load Thread Network Directory storage: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

CHIP_ERROR DefaultThreadNetworkDirectoryStorage::StoreIndex()
{
    VerifyOrDie(mInitialized);
    StorageKeyName key = DefaultStorageKeyAllocator::ThreadNetworkDirectoryIndex();
    static_assert(kCapacity * ExtendedPanId::size() <= UINT16_MAX); // kCapacity >= mCount
    return mStorage.SyncSetKeyValue(key.KeyName(), mExtendedPanIds, static_cast<uint16_t>(mCount * ExtendedPanId::size()));
}

bool DefaultThreadNetworkDirectoryStorage::FindNetwork(const ExtendedPanId & exPanId, index_t & outIndex)
{
    for (index_t idx = 0; idx < mCount; idx++)
    {
        if (mExtendedPanIds[idx] == exPanId)
        {
            outIndex = idx;
            return true;
        }
    }
    return false;
}

ThreadNetworkDirectoryStorage::ExtendedPanIdIterator * DefaultThreadNetworkDirectoryStorage::IterateNetworkIds()
{
    InitializedIfNeeded();
    return mIterators.CreateObject(*this);
}

bool DefaultThreadNetworkDirectoryStorage::ContainsNetwork(const ExtendedPanId & exPanId)
{
    InitializedIfNeeded();
    index_t unused;
    return FindNetwork(exPanId, unused);
}

CHIP_ERROR DefaultThreadNetworkDirectoryStorage::GetNetworkDataset(const ExtendedPanId & exPanId, MutableByteSpan & dataset)
{
    VerifyOrReturnError(ContainsNetwork(exPanId), CHIP_ERROR_NOT_FOUND);
    uint16_t size      = static_cast<uint16_t>(CanCastTo<uint16_t>(dataset.size()) ? dataset.size() : UINT16_MAX);
    StorageKeyName key = DefaultStorageKeyAllocator::ThreadNetworkDirectoryDataset(exPanId.AsNumber());
    ReturnErrorOnFailure(mStorage.SyncGetKeyValue(key.KeyName(), dataset.data(), size));
    dataset.reduce_size(size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultThreadNetworkDirectoryStorage::AddOrUpdateNetwork(const ExtendedPanId & exPanId, ByteSpan dataset)
{
    VerifyOrReturnError(0 < dataset.size() && dataset.size() <= kMaxThreadDatasetLen, CHIP_ERROR_INVALID_ARGUMENT);
    bool update = ContainsNetwork(exPanId);
    VerifyOrReturnError(update || mCount < kCapacity, CHIP_ERROR_NO_MEMORY);

    // Store the dataset first
    StorageKeyName key = DefaultStorageKeyAllocator::ThreadNetworkDirectoryDataset(exPanId.AsNumber());
    ReturnErrorOnFailure(mStorage.SyncSetKeyValue(key.KeyName(), dataset.data(), static_cast<uint16_t>(dataset.size())));

    // Update the index if we're adding a new network, rolling back on failure.
    if (!update)
    {
        mExtendedPanIds[mCount++] = exPanId;
        CHIP_ERROR err            = StoreIndex();
        if (err != CHIP_NO_ERROR)
        {
            mCount--;
            mStorage.SyncDeleteKeyValue(key.KeyName());
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultThreadNetworkDirectoryStorage::RemoveNetwork(const ExtendedPanId & exPanId)
{
    InitializedIfNeeded();
    index_t index;
    VerifyOrReturnError(FindNetwork(exPanId, index), CHIP_ERROR_NOT_FOUND);

    // Move subsequent elements down to fill the deleted slot
    static_assert(std::is_trivially_copyable_v<ExtendedPanId>);
    size_t subsequentCount = mCount - (index + 1u);
    auto * element         = &mExtendedPanIds[index];
    memmove(element, element + 1, subsequentCount * sizeof(*element));
    mCount--;

    CHIP_ERROR err = StoreIndex();
    if (err != CHIP_NO_ERROR)
    {
        // Roll back the change to our in-memory state
        memmove(element + 1, element, subsequentCount * sizeof(*element));
        mExtendedPanIds[index] = exPanId;
        mCount++;
        return err;
    }

    // Delete the dataset itself. Ignore errors since we successfully updated the index.
    StorageKeyName key = DefaultStorageKeyAllocator::ThreadNetworkDirectoryDataset(exPanId.AsNumber());
    mStorage.SyncDeleteKeyValue(key.KeyName());
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
