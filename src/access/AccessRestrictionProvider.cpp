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

#include "AccessRestrictionProvider.h"

#include <algorithm>
#include <lib/core/Global.h>

using namespace chip::Platform;

namespace chip {
namespace Access {

void AccessRestrictionProvider::AddListener(Listener & listener)
{
    if (mListeners == nullptr)
    {
        mListeners     = &listener;
        listener.mNext = nullptr;
        return;
    }

    for (Listener * l = mListeners; /**/; l = l->mNext)
    {
        if (l == &listener)
        {
            return;
        }

        if (l->mNext == nullptr)
        {
            l->mNext       = &listener;
            listener.mNext = nullptr;
            return;
        }
    }
}

void AccessRestrictionProvider::RemoveListener(Listener & listener)
{
    if (mListeners == &listener)
    {
        mListeners     = listener.mNext;
        listener.mNext = nullptr;
        return;
    }

    for (Listener * l = mListeners; l != nullptr; l = l->mNext)
    {
        if (l->mNext == &listener)
        {
            l->mNext       = listener.mNext;
            listener.mNext = nullptr;
            return;
        }
    }
}

CHIP_ERROR AccessRestrictionProvider::SetCommissioningEntries(const std::vector<Entry> & entries)
{
    // check that the input entries are valid
    for (auto & entry : entries)
    {
        if (!IsEntryValid(entry))
        {
            ChipLogError(DataManagement, "AccessRestrictionProvider: invalid entry");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    mCommissioningEntries = entries;

    for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
    {
        listener->CommissioningRestrictionListChanged();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessRestrictionProvider::SetEntries(const FabricIndex fabricIndex, const std::vector<Entry> & entries)
{
    std::vector<Entry> updatedEntries;

    // check that the input entries are valid
    for (auto & entry : entries)
    {
        if (!IsEntryValid(entry))
        {
            ChipLogError(DataManagement, "AccessRestrictionProvider: invalid entry");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        Entry updatedEntry = entry;
        updatedEntry.fabricIndex = fabricIndex;
        updatedEntries.push_back(updatedEntry);
    }

    mFabricEntries[fabricIndex] = std::move(updatedEntries);

    for (Listener * listener = mListeners; listener != nullptr; listener = listener->mNext)
    {
        listener->RestrictionListChanged(fabricIndex);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessRestrictionProvider::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath)
{
    ChipLogProgress(DataManagement, "AccessRestrictionProvider: action %d", static_cast<int>(requestPath.requestType));

    if (requestPath.requestType == RequestType::kRequestTypeUnknown)
    {
        ChipLogError(DataManagement, "AccessRestrictionProvider: RequestPath type is unknown");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // wildcard event subscriptions are allowed since wildcard is only used when setting up the subscription and
    // we want that request to succeed (when generating the report, this method will be called with the specific
    // event id). All other requests require an entity id
    if (!requestPath.entityId.has_value())
    {
        if (requestPath.requestType == RequestType::kEventReadOrSubscribeRequest)
        {
            return CHIP_NO_ERROR;
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    for (auto & entry : mFabricEntries[subjectDescriptor.fabricIndex])
    {
        if (entry.endpointNumber != requestPath.endpoint || entry.clusterId != requestPath.cluster)
        {
            continue;
        }

        for (auto & restriction : entry.restrictions)
        {
            // a missing id is a wildcard
            bool idMatch = !restriction.id.HasValue() || restriction.id.Value() == requestPath.entityId.value();
            if (!idMatch)
            {
                continue;
            }

            switch (restriction.restrictionType)
            {
            case Type::kAttributeAccessForbidden:
                if (requestPath.requestType == RequestType::kAttributeReadRequest ||
                    requestPath.requestType == RequestType::kAttributeWriteRequest)
                {
                    return CHIP_ERROR_ACCESS_DENIED;
                }
                break;
            case Type::kAttributeWriteForbidden:
                if (requestPath.requestType == RequestType::kAttributeWriteRequest)
                {
                    return CHIP_ERROR_ACCESS_DENIED;
                }
                break;
            case Type::kCommandForbidden:
                if (requestPath.requestType == RequestType::kCommandInvokeRequest)
                {
                    return CHIP_ERROR_ACCESS_DENIED;
                }
                break;
            case Type::kEventForbidden:
                if (requestPath.requestType == RequestType::kEventReadOrSubscribeRequest)
                {
                    return CHIP_ERROR_ACCESS_DENIED;
                }
                break;
            }
        }
    }

    return CHIP_NO_ERROR;
}

bool AccessRestrictionProvider::IsEntryValid(const Entry & entry) const
{
    return entry.endpointNumber != 0 && entry.clusterId != app::Clusters::NetworkCommissioning::Id &&
        entry.clusterId != app::Clusters::Descriptor::Id;
}

} // namespace Access
} // namespace chip
