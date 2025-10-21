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

void NodeAddressCache::CacheNode(NodeId nodeId, const ResolveResult & result)
{
    // Check if nodeId already exists in cache to update it
    RemoveCachedNodeAddress(nodeId);

    // If cache is full, overwrite oldest entry (FIFO)
    if (mCacheCount >= kMaxCacheSize)
    {
        ChipLogProgress(Discovery, "Cache full, overwriting oldest entry");
        RemoveCachedNodeAddress(mCache[0].nodeId);
        if (mCacheCount >= kMaxCacheSize) {
            ChipLogProgress(Discovery, "Cache removal failed");
            return;
        }
    }

    // Add new entry
    mCache[mCacheCount].nodeId = nodeId;
    mCache[mCacheCount].result = result;
    ++mCacheCount;

    ChipLogProgress(Discovery, "Cached address for NodeId: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    return;
}

CHIP_ERROR NodeAddressCache::GetCachedNodeAddress(NodeId nodeId, ResolveResult & result) const
{
    for (size_t i = 0; i < mCacheCount; ++i)
    {
        if (mCache[i].nodeId == nodeId)
        {
            result = mCache[i].result;
            ChipLogProgress(Discovery, "Retrieved cached address for NodeId: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
            return CHIP_NO_ERROR;
        }
    }
    ChipLogError(Discovery, "No cached address for NodeId: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR NodeAddressCache::RemoveCachedNodeAddress(NodeId nodeId)
{
    for (size_t i = 0; i < mCacheCount; ++i)
    {
        if (mCache[i].nodeId == nodeId)
        {
            // Shift all entries after the removed one to maintain FIFO order
            for (size_t j = i; j < mCacheCount - 1; ++j)
            {
                mCache[j] = mCache[j + 1];
            }
            
            mCache[mCacheCount - 1].nodeId = 0;
            --mCacheCount;
            
            ChipLogProgress(Discovery, "Removed cached address for NodeId: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
            return CHIP_NO_ERROR;
        }
    }
    ChipLogError(Discovery, "No cached address to remove for NodeId: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    return CHIP_ERROR_KEY_NOT_FOUND;
}

void NodeAddressCache::Clear()
{
    mCacheCount = 0;
}

} // namespace AddressResolve
} // namespace chip
