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
 * This cache makes use of the fact that looking-up AttributeAccessInterface
 * instances is usually done in loops, during read/subscription wildcard
 * expansion, and there is a significant amount of locality.
 *
 * This cache records both "used" (i.e. uses AAI) and the single last
 * "unused" (i.e. does NOT use AAI) entries. Combining positive/negative
 * lookup led to factor of ~10 reduction of AAI lookups in total for wildcard
 * reads on chip-all-clusters-app, with a cache size of 1. Increasing the size did not
 * significantly improve the performance.
 */
class AttributeAccessInterfaceCache
{
public:
    enum class CacheResult
    {
        kCacheMiss,
        kDefinitelyUnused,
        kDefinitelyUsed
    };

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
     * @param outAttributeAccess - If not null, and Get returns `kDefinitelyUsed`, then this is set to the instance pointer.
     * @return a for whether the entry is actually used or not.
     */
    CacheResult Get(EndpointId endpointId, ClusterId clusterId, AttributeAccessInterface ** outAttributeAccess)
    {
        if (mLastUnusedEntry.Matches(endpointId, clusterId))
        {
            return CacheResult::kDefinitelyUnused;
        }

        AttributeAccessCacheEntry * cacheSlot = GetCacheSlot(endpointId, clusterId);
        if (cacheSlot->Matches(endpointId, clusterId) && (cacheSlot->accessor != nullptr))
        {
            if (outAttributeAccess != nullptr)
            {
                *outAttributeAccess = cacheSlot->accessor;
            }
            return CacheResult::kDefinitelyUsed;
        }

        return CacheResult::kCacheMiss;
    }

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
        (void) endpointId;
        (void) clusterId;
        return &mCacheSlots[0];
    }

    AttributeAccessCacheEntry mCacheSlots[1];
    AttributeAccessCacheEntry mLastUnusedEntry;
};

} // namespace app
} // namespace chip
