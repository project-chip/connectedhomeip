/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AttributePathParams.h>
#include <app/util/basic-types.h>
#include <assert.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * ClusterInfo is the representation of an attribute path or an event path used by ReacHandler, ReadClient, WriteHandler,
 * Report::Engine etc, it contains a mpNext field so it can be used as a linked list. It uses some invalid values for representing
 * the wildcard value for its field.
 */
// TODO: The cluster info should be breaked into AttributeInfo and EventInfo.
// Note: The change will happen after #11171 with a better linked list.
struct ClusterInfo
{
    bool IsAttributePathSupersetOf(const ClusterInfo & other) const
    {
        VerifyOrReturnError(!mEndpointId.HasValue() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(!mClusterId.HasValue() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(!mFieldId.HasValue() || mFieldId == other.mFieldId, false);
        VerifyOrReturnError(!mListIndex.HasValue() || mListIndex == other.mListIndex, false);

        return true;
    }

    bool HasWildcard() const { return !mEndpointId.HasValue() || !mClusterId.HasValue() || !mFieldId.HasValue(); }

    ClusterInfo() {}

    Optional<NodeId> mNodeId;
    Optional<EndpointId> mEndpointId;
    Optional<ClusterId> mClusterId;
    Optional<AttributeId> mFieldId;
    Optional<EventId> mEventId;
    Optional<ListIndex> mListIndex;
    ClusterInfo * mpNext = nullptr; // pointer width (uint32 or uint64)
};
} // namespace app
} // namespace chip
