/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <stddef.h>

#include <app/AttributeAccessInterface.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/**
 * @brief Cache to make look-up of AttributeAccessInterface (AAI) instances faster.
 *
 * @tparam N - The size of the cache (current suggested value == 1)
 *
 * This cache makes use of the fact that looking-up AttributeAccessInterface
 * instances is usually done in loops, during read/subscription wildcard
 * expansion, and there is a significant amount of locality.
 *
 * This cache records both "used" (i.e. uses AAI) and the single last
 * "unused" (i.e. does NOT use AAI) entries. Combining positive/negative
 * lookup led to factor of ~10 reduction of AAI lookups in total for wildcard
 * reads on chip-all-clusters-app, with a cache size of 1. The size did not
 * significantly improve the performance, but `N` is left to support better
 * hashing/storage algorithms in the future if needed.
 */
template <size_t N>
class AttributeAccessInterfaceCache
{
public:
    AttributeAccessInterfaceCache() { Invalidate(); }

    /**
     * @brief Invalidate the whole cache. Must be called every time list of AAI registrations changes.
     */
    void Invalidate()
    {
        for (auto & entry : mCacheSlots)
        {
            entry.Invalidate();
        }
        mLastUnusedEntry.Invalidate();
    }

    /**
     * @brief Mark that we know a given <`endpointId`, `clusterId`> uses AAI, with instance `attrInterface`
     */
    void MarkUsed(EndpointId endpointId, ClusterId clusterId, AttributeAccessInterface * attrInterface)
    {
        GetCacheSlot(endpointId, clusterId)->Set(endpointId, clusterId, attrInterface);
    }

    /**
     * @brief Mark that we know a given <`endpointId`, `clusterId`> does NOT use AAI.
     */
    void MarkUnused(EndpointId endpointId, ClusterId clusterId) { mLastUnusedEntry.Set(endpointId, clusterId, nullptr); }

    /**
     * @brief Get the AttributeAccessInterface instance for a given <`endpointId`, `clusterId`>, if present in cache.
     *
     * @param endpointId - Endpoint ID to look-up.
     * @param clusterId - Cluster ID to look-up.
     * @return the instance pointer on cache hit, or nullptr on cache miss.
     */
    AttributeAccessInterface * Get(EndpointId endpointId, ClusterId clusterId)
    {
        AttributeAccessCacheEntry * cacheSlot = GetCacheSlot(endpointId, clusterId);
        if (cacheSlot->Matches(endpointId, clusterId) && (cacheSlot->accessor != nullptr))
        {
            return cacheSlot->accessor;
        }

        return nullptr;
    }

    /**
     * @brief returns true if <`endpointId`, `clusterId`> was last marked as NOT to using AAI.
     *
     * May return false even though it doesn't use AAI, on cache miss of unused slot.
     */
    bool IsUnused(EndpointId endpointId, ClusterId clusterId) const { return mLastUnusedEntry.Matches(endpointId, clusterId); }

private:
    struct AttributeAccessCacheEntry
    {
        EndpointId endpointId               = kInvalidEndpointId;
        ClusterId clusterId                 = kInvalidClusterId;
        AttributeAccessInterface * accessor = nullptr;

        void Invalidate()
        {
            endpointId = kInvalidEndpointId;
            clusterId  = kInvalidClusterId;
            accessor   = nullptr;
        }

        void Set(EndpointId theEndpointId, ClusterId theClusterId, AttributeAccessInterface * theAccessor)
        {
            endpointId = theEndpointId;
            clusterId  = theClusterId;
            accessor   = theAccessor;
        }

        bool Matches(EndpointId theEndpointId, ClusterId theClusterId) const
        {
            return (endpointId == theEndpointId) && (clusterId == theClusterId);
        }
    };

    AttributeAccessCacheEntry * GetCacheSlot(EndpointId endpointId, ClusterId clusterId)
    {
        // A future implementation may use a better method.
        size_t slotId = (clusterId ^ endpointId) & (N - 1);
        return &mCacheSlots[slotId];
    }

    AttributeAccessCacheEntry mCacheSlots[N];
    AttributeAccessCacheEntry mLastUnusedEntry;
};

} // namespace app
} // namespace chip
