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

#include <app/util/basic-types.h>
#include <assert.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/**
 * ClusterInfo is the representation of an attribute path or an event path used by ReadHandler, ReadClient, WriteHandler,
 * Report::Engine etc, tt uses some invalid values for representing the wildcard value for its field and  contains a mpNext field so
 * it can be used as a linked list.
 */
// TODO: The cluster info should be separated into AttributeInfo and EventInfo.
// Note: The change will happen after #11171 with a better linked list.
struct ClusterInfo
{
    // Endpoint Id is a uint16 number, and should between 0 and 0xFFFE
    static constexpr EndpointId kInvalidEndpointId = 0xFFFF;
    // The ClusterId, AttributeId and EventId are MEIs,
    // 0xFFFF is not a valid manufacturer code, thus 0xFFFF'FFFF is not a valid MEI
    static constexpr ClusterId kInvalidClusterId     = 0xFFFF'FFFF;
    static constexpr AttributeId kInvalidAttributeId = 0xFFFF'FFFF;
    static constexpr EventId kInvalidEventId         = 0xFFFF'FFFF;
    // ListIndex is a uint16 number, thus 0xFFFF is not a valid list index.
    static constexpr ListIndex kInvalidListIndex = 0xFFFF;

    bool IsAttributePathSupersetOf(const ClusterInfo & other) const
    {
        VerifyOrReturnError(!HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(!HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(!HasWildcardAttributeId() || mFieldId == other.mFieldId, false);
        VerifyOrReturnError(!HasWildcardListIndex() || mListIndex == other.mListIndex, false);

        return true;
    }

    bool HasWildcard() const { return !HasWildcardEndpointId() || !HasWildcardClusterId() || !HasWildcardAttributeId(); }

    /**
     * Verify if it mets some basic constrants of a attribute path: If list index is valid, then field id must be valid too.
     * There are other conditions that the path is not a valid path, e.g. the path does not exists, wildcard cluster id with
     * non-wildcard attribute id and valid list index on a non-array attributes etc.
     */
    bool IsValidAttributePath() const { return mListIndex == kInvalidListIndex || mFieldId != kInvalidAttributeId; }

    inline bool HasWildcardNodeId() const { return mNodeId != kUndefinedNodeId; }
    inline bool HasWildcardEndpointId() const { return mEndpointId != kInvalidEndpointId; }
    inline bool HasWildcardClusterId() const { return mClusterId != kInvalidClusterId; }
    inline bool HasWildcardAttributeId() const { return mFieldId != kInvalidAttributeId; }
    inline bool HasWildcardListIndex() const { return mListIndex != kInvalidListIndex; }
    inline bool HasWildcardEventId() const { return mEventId != kInvalidEventId; }

    ClusterInfo() {}
    /*
     * For better structure alignment
     * Above ordering is by bit-size to ensure least amount of memory alignment padding.
     * Changing order to something more natural (e.g. endpoint id before cluster id) will result
     * in extra memory alignment padding.
     */
    ClusterInfo * mpNext   = nullptr;             // pointer width (32/64 bits)
    NodeId mNodeId         = kUndefinedNodeId;    // uint64
    ClusterId mClusterId   = kInvalidClusterId;   // uint32
    AttributeId mFieldId   = kInvalidAttributeId; // uint32
    EventId mEventId       = kInvalidEventId;     // uint32
    ListIndex mListIndex   = kInvalidListIndex;   // uint16
    EndpointId mEndpointId = kInvalidEndpointId;  // uint16
};
} // namespace app
} // namespace chip
