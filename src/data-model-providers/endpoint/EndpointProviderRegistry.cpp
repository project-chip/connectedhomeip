/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <data-model-providers/endpoint/EndpointProviderRegistry.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

CHIP_ERROR EndpointProviderRegistry::Register(EndpointProviderRegistration & entry)
{
    VerifyOrReturnError(entry.next == nullptr, CHIP_ERROR_INVALID_ARGUMENT); // Should not be part of another list
    VerifyOrReturnError(entry.endpointProviderInterface != nullptr, CHIP_ERROR_INVALID_ARGUMENT); // Should not be null

    auto newEndpointId = entry.endpointProviderInterface->GetEndpointEntry().id;
    VerifyOrReturnError(newEndpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(Get(newEndpointId) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID); // Check for duplicates

    entry.next     = mRegistrations;
    mRegistrations = &entry;

    return CHIP_NO_ERROR;
}

CHIP_ERROR EndpointProviderRegistry::Unregister(EndpointId endpointId)
{
    VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    EndpointProviderRegistration * prev    = nullptr;
    EndpointProviderRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->endpointProviderInterface->GetEndpointEntry().id == endpointId)
        {
            if (prev == nullptr) // Node to remove is the head
            {
                mRegistrations = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            current->next = nullptr; // Clear the registration's next pointer

            if (mCachedEndpointId == endpointId) // Invalidate cache if the unregistered endpoint was cached
            {
                mCachedInterface  = nullptr;
                mCachedEndpointId = kInvalidEndpointId;
            }
            return CHIP_NO_ERROR;
        }
        prev    = current;
        current = current->next;
    }
    return CHIP_NO_ERROR;
}

EndpointProviderInterface * EndpointProviderRegistry::Get(EndpointId endpointId)
{
    if (mCachedEndpointId == endpointId && mCachedInterface != nullptr)
    {
        return mCachedInterface;
    }

    // The endpoint searched for is not cached, do a linear search for it
    EndpointProviderRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->endpointProviderInterface->GetEndpointEntry().id == endpointId)
        {
            mCachedInterface  = current->endpointProviderInterface;
            mCachedEndpointId = endpointId;
            return mCachedInterface;
        }
        current = current->next;
    }

    // Not found
    return nullptr;
}

} // namespace app
} // namespace chip
