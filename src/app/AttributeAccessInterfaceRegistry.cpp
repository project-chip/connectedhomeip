/**
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
#include "app/AttributeAccessInterface.h"
#include <app/AttributeAccessInterfaceRegistry.h>

#include <app/AttributeAccessInterfaceCache.h>

namespace {

using chip::app::AttributeAccessInterface;

// shouldUnregister returns true if the given AttributeAccessInterface should be
// unregistered.
template <typename F>
void UnregisterMatchingAttributeAccessInterfaces(F shouldUnregister, AttributeAccessInterface *& list_head)
{
    AttributeAccessInterface * prev = nullptr;
    AttributeAccessInterface * cur  = list_head;
    while (cur)
    {
        AttributeAccessInterface * next = cur->GetNext();
        if (shouldUnregister(cur))
        {
            // Remove it from the list
            if (prev)
            {
                prev->SetNext(next);
            }
            else
            {
                list_head = next;
            }

            cur->SetNext(nullptr);

            // Do not change prev in this case.
        }
        else
        {
            prev = cur;
        }
        cur = next;
    }
}

} // namespace

namespace chip {
namespace app {

AttributeAccessInterfaceRegistry & AttributeAccessInterfaceRegistry::Instance()
{
    static AttributeAccessInterfaceRegistry instance;
    return instance;
}

void AttributeAccessInterfaceRegistry::Unregister(AttributeAccessInterface * attrOverride)
{
    mAttributeAccessInterfaceCache.Invalidate();
    UnregisterMatchingAttributeAccessInterfaces([attrOverride](AttributeAccessInterface * entry) { return entry == attrOverride; },
                                                mAttributeAccessOverrides);
}

void AttributeAccessInterfaceRegistry::UnregisterAllForEndpoint(EndpointId endpointId)
{
    mAttributeAccessInterfaceCache.Invalidate();
    UnregisterMatchingAttributeAccessInterfaces(
        [endpointId](AttributeAccessInterface * entry) { return entry->MatchesEndpoint(endpointId); }, mAttributeAccessOverrides);
}

bool AttributeAccessInterfaceRegistry::Register(AttributeAccessInterface * attrOverride)
{
    mAttributeAccessInterfaceCache.Invalidate();
    for (auto * cur = mAttributeAccessOverrides; cur; cur = cur->GetNext())
    {
        if (cur->Matches(*attrOverride))
        {
            ChipLogError(InteractionModel, "Duplicate attribute override registration failed");
            return false;
        }
    }
    attrOverride->SetNext(mAttributeAccessOverrides);
    mAttributeAccessOverrides = attrOverride;
    return true;
}

AttributeAccessInterface * AttributeAccessInterfaceRegistry::Get(EndpointId endpointId, ClusterId clusterId)
{
    using CacheResult = AttributeAccessInterfaceCache::CacheResult;

    AttributeAccessInterface * cached = nullptr;
    CacheResult result                = mAttributeAccessInterfaceCache.Get(endpointId, clusterId, &cached);
    switch (result)
    {
    case CacheResult::kDefinitelyUnused:
        return nullptr;
    case CacheResult::kDefinitelyUsed:
        return cached;
    case CacheResult::kCacheMiss:
    default:
        // Did not cache yet, search set of AAI registered, and cache if found.
        for (app::AttributeAccessInterface * cur = mAttributeAccessOverrides; cur; cur = cur->GetNext())
        {
            if (cur->Matches(endpointId, clusterId))
            {
                mAttributeAccessInterfaceCache.MarkUsed(endpointId, clusterId, cur);
                return cur;
            }
        }

        // Did not find AAI registered: mark as definitely not using.
        mAttributeAccessInterfaceCache.MarkUnused(endpointId, clusterId);
    }

    return nullptr;
}

} // namespace app
} // namespace chip
