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
#include <app/clusters/ota-requestor/OTARequestorEventHandlerRegistry.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

CHIP_ERROR OTARequestorEventHandlerRegistry::Register(OTARequestorEventHandlerRegistration & entry)
{
    VerifyOrReturnError(entry.next == nullptr, CHIP_ERROR_INVALID_ARGUMENT);                  // Should not be part of another list
    VerifyOrReturnError(entry.eventHandler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);          // Should not be null
    VerifyOrReturnError(entry.endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT); // Should not be invalid ID
    VerifyOrReturnError(Get(entry.endpointId) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID);       // Check for duplicates

    entry.next     = mRegistrations;
    mRegistrations = &entry;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorEventHandlerRegistry::Unregister(EndpointId endpointId)
{
    VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    OTARequestorEventHandlerRegistration * prev    = nullptr;
    OTARequestorEventHandlerRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->endpointId == endpointId)
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

            return CHIP_NO_ERROR;
        }
        prev    = current;
        current = current->next;
    }
    return CHIP_ERROR_NOT_FOUND;
}

OTARequestorEventHandler * OTARequestorEventHandlerRegistry::Get(EndpointId endpointId)
{
    // The endpoint searched for is not cached, do a linear search for it
    OTARequestorEventHandlerRegistration * current = mRegistrations;

    while (current != nullptr)
    {
        if (current->endpointId == endpointId)
        {
            return current->eventHandler;
        }
        current = current->next;
    }

    // Not found
    return nullptr;
}

} // namespace app
} // namespace chip
