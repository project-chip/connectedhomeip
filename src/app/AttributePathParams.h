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

#include <app/ClusterInfo.h>

namespace chip {
namespace app {
struct AttributePathParams
{
    //
    // TODO: (Issue #10596) Need to ensure that we do not encode the NodeId over the wire
    // if it is either not 'set', or is set to a value that matches accessing fabric
    // on which the interaction is undertaken.
    //
    // TODO: (#11420) This class is overlapped with ClusterInfo class, need to do a clean up.
    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId) :
        AttributePathParams(aEndpointId, aClusterId, kInvalidAttributeId, kInvalidListIndex)
    {}

    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId, AttributeId aAttributeId) :
        AttributePathParams(aEndpointId, aClusterId, aAttributeId, kInvalidListIndex)
    {}

    AttributePathParams(ClusterId aClusterId, AttributeId aAttributeId) :
        AttributePathParams(kInvalidEndpointId, aClusterId, aAttributeId, kInvalidListIndex)
    {}

    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId, AttributeId aAttributeId, ListIndex aListIndex) :
        mEndpointId(aEndpointId), mClusterId(aClusterId), mAttributeId(aAttributeId), mListIndex(aListIndex)
    {}

    AttributePathParams() {}

    bool HasAttributeWildcard() const { return HasWildcardEndpointId() || HasWildcardClusterId() || HasWildcardAttributeId(); }

    /**
     * SPEC 8.9.2.2
     * Check that the path meets some basic constraints of an attribute path: If list index is not wildcard, then field id must not
     * be wildcard. This does not verify that the attribute being targeted is actually of list type when the list index is not
     * wildcard.
     */
    bool IsValidAttributePath() const { return HasWildcardListIndex() || !HasWildcardAttributeId(); }

    inline bool HasWildcardEndpointId() const { return mEndpointId == kInvalidEndpointId; }
    inline bool HasWildcardClusterId() const { return mClusterId == kInvalidClusterId; }
    inline bool HasWildcardAttributeId() const { return mAttributeId == kInvalidAttributeId; }
    inline bool HasWildcardListIndex() const { return mListIndex == kInvalidListIndex; }

    EndpointId mEndpointId   = kInvalidEndpointId;
    ClusterId mClusterId     = kInvalidClusterId;
    AttributeId mAttributeId = kInvalidAttributeId;
    ListIndex mListIndex     = kInvalidListIndex;
};
} // namespace app
} // namespace chip
