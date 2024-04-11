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
#include <app/AttributeAccessInterfaceRegistry.h>

#include <app/AttributeAccessInterfaceCache.h>

using namespace chip::app;

namespace {

AttributeAccessInterface * gAttributeAccessOverrides = nullptr;
AttributeAccessInterfaceCache gAttributeAccessInterfaceCache;

// shouldUnregister returns true if the given AttributeAccessInterface should be
// unregistered.
template <typename F>
void UnregisterMatchingAttributeAccessInterfaces(F shouldUnregister)
{
    AttributeAccessInterface * prev = nullptr;
    AttributeAccessInterface * cur  = gAttributeAccessOverrides;
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
                gAttributeAccessOverrides = next;
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

void unregisterAttributeAccessOverride(AttributeAccessInterface * attrOverride)
{
    gAttributeAccessInterfaceCache.Invalidate();
    UnregisterMatchingAttributeAccessInterfaces([attrOverride](AttributeAccessInterface * entry) { return entry == attrOverride; });
}

void unregisterAttributeAccessOverrideForEndpoint(EmberAfDefinedEndpoint * definedEndpoint)
{
    UnregisterMatchingAttributeAccessInterfaces(
        [endpoint = definedEndpoint->endpoint](AttributeAccessInterface * entry) { return entry->MatchesEndpoint(endpoint); });
}

bool registerAttributeAccessOverride(AttributeAccessInterface * attrOverride)
{
    gAttributeAccessInterfaceCache.Invalidate();
    for (auto * cur = gAttributeAccessOverrides; cur; cur = cur->GetNext())
    {
        if (cur->Matches(*attrOverride))
        {
            ChipLogError(Zcl, "Duplicate attribute override registration failed");
            return false;
        }
    }
    attrOverride->SetNext(gAttributeAccessOverrides);
    gAttributeAccessOverrides = attrOverride;
    return true;
}

namespace chip {
namespace app {

app::AttributeAccessInterface * GetAttributeAccessOverride(EndpointId endpointId, ClusterId clusterId)
{
    using CacheResult = AttributeAccessInterfaceCache::CacheResult;

    AttributeAccessInterface * cached = nullptr;
    CacheResult result                = gAttributeAccessInterfaceCache.Get(endpointId, clusterId, &cached);
    switch (result)
    {
    case CacheResult::kDefinitelyUnused:
        return nullptr;
    case CacheResult::kDefinitelyUsed:
        return cached;
    case CacheResult::kCacheMiss:
    default:
        // Did not cache yet, search set of AAI registered, and cache if found.
        for (app::AttributeAccessInterface * cur = gAttributeAccessOverrides; cur; cur = cur->GetNext())
        {
            if (cur->Matches(endpointId, clusterId))
            {
                gAttributeAccessInterfaceCache.MarkUsed(endpointId, clusterId, cur);
                return cur;
            }
        }

        // Did not find AAI registered: mark as definitely not using.
        gAttributeAccessInterfaceCache.MarkUnused(endpointId, clusterId);
    }

    return nullptr;
}

} // namespace app
} // namespace chip
