/*
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


#include <lib/address_resolve/AddressResolve.h>
#include <lib/address_resolve/NodeAddressCache.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace AddressResolve {

void NodeAddressCache::CacheNode(const PeerId & peerId, const ResolveResult & result)
{
    // If already exists, update without changing insertion order
    if (mCache.find(peerId) != mCache.end())
    {
        mCache[peerId] = result;

        ChipLogProgress(Discovery, "Updated cached address for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
        return;
    }

    // If cache is full, remove oldest entry
    if (mCache.size() >= kMaxCacheSize)
    {
        PeerId oldest = mInsertionOrder.front();
        mInsertionOrder.pop();
        mCache.erase(oldest);
        ChipLogProgress(Discovery, "Cache full, removed oldest entry");
    }

    // Add new entry
    mCache[peerId] = result;
    mInsertionOrder.push(peerId);
    ChipLogProgress(Discovery, "Cached address for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
}

bool NodeAddressCache::Lookup(const PeerId & peerId, ResolveResult & result) const
{
    auto it = mCache.find(peerId);
    if (it != mCache.end())
    {
        result = it->second;
        ChipLogProgress(Discovery, "Retrieved cached address for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
        return true;
    }
    ChipLogProgress(Discovery, "No cached address for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
    return false;
}

CHIP_ERROR NodeAddressCache::RemoveCachedNodeAddress(const PeerId & peerId)
{
    if (mCache.erase(peerId) > 0)
    {
        // Note: We don't remove from mInsertionOrder queue as it's expensive
        // The queue will be cleaned up naturally during FIFO eviction
        ChipLogProgress(Discovery, "Removed cached address for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
        return CHIP_NO_ERROR;
    }
    ChipLogProgress(Discovery, "No cached address to remove for " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
    return CHIP_ERROR_KEY_NOT_FOUND;
}

void NodeAddressCache::Clear()
{
    mCache.clear();
    std::queue<PeerId> empty;
    mInsertionOrder.swap(empty);
}

} // namespace AddressResolve
} // namespace chip
