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

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteEventPath.h>
#include <app/util/basic-types.h>
#include <assert.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * ClusterInfo is the representation of an attribute path or an event path used by ReadHandler, ReadClient, WriteHandler,
 * Report::Engine etc, it uses some invalid values for representing the wildcard values for its fields and  contains a mpNext field
 * so it can be used as a linked list.
 */
// TODO: The cluster info should be separated into AttributeInfo and EventInfo.
// Note: The user of ClusterInfo today is ReadHandler and ReportEngine, both of them do not accept Null list index (means list
//       append operations)
// Note: The change will happen after #11171 with a better linked list.
struct ClusterInfo
{
public:
    bool IsAttributePathSupersetOf(const ClusterInfo & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardAttributeId() || mAttributeId == other.mAttributeId, false);
        VerifyOrReturnError(HasWildcardListIndex() || mListIndex == other.mListIndex, false);

        return true;
    }

    bool IsAttributePathSupersetOf(const ConcreteAttributePath & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardAttributeId() || mAttributeId == other.mAttributeId, false);

        return true;
    }

    bool IsEventPathSupersetOf(const ConcreteEventPath & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardEventId() || mEventId == other.mEventId, false);

        return true;
    }

    bool HasAttributeWildcard() const { return HasWildcardEndpointId() || HasWildcardClusterId() || HasWildcardAttributeId(); }
    bool HasEventWildcard() const { return HasWildcardEndpointId() || HasWildcardClusterId() || HasWildcardEventId(); }
    /**
     * Check that the path meets some basic constraints of an attribute path: If list index is not wildcard, then field id must not
     * be wildcard. This does not verify that the attribute being targeted is actually of list type when the list index is not
     * wildcard.
     */
    bool IsValidAttributePath() const { return HasWildcardListIndex() || !HasWildcardAttributeId(); }

    // For event, an event id can only be interpreted if the cluster id is known.
    bool IsValidEventPath() const { return !(HasWildcardClusterId() && !HasWildcardEventId()); }

    inline bool HasWildcardNodeId() const { return mNodeId == kUndefinedNodeId; }
    inline bool HasWildcardEndpointId() const { return mEndpointId == kInvalidEndpointId; }
    inline bool HasWildcardClusterId() const { return mClusterId == kInvalidClusterId; }
    inline bool HasWildcardAttributeId() const { return mAttributeId == kInvalidAttributeId; }
    inline bool HasWildcardListIndex() const { return mListIndex == kInvalidListIndex; }
    inline bool HasWildcardEventId() const { return mEventId == kInvalidEventId; }

    ClusterInfo() {}
    /*
     * For better structure alignment
     * Below ordering is by bit-size to ensure least amount of memory alignment padding.
     * Changing order to something more natural (e.g. endpoint id before cluster id) will result
     * in extra memory alignment padding.
     */
    NodeId mNodeId           = kUndefinedNodeId;    // uint64
    ClusterInfo * mpNext     = nullptr;             // pointer width (32/64 bits)
    ClusterId mClusterId     = kInvalidClusterId;   // uint32
    AttributeId mAttributeId = kInvalidAttributeId; // uint32
    EventId mEventId         = kInvalidEventId;     // uint32
    ListIndex mListIndex     = kInvalidListIndex;   // uint16
    EndpointId mEndpointId   = kInvalidEndpointId;  // uint16
};
} // namespace app
} // namespace chip
