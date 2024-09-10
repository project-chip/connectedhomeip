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

#include <app/AppConfig.h>
#include <app/ConcreteAttributePath.h>
#include <app/DataVersionFilter.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_ENABLE_READ_CLIENT
class ReadClient;
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
struct AttributePathParams
{
    AttributePathParams() = default;

    explicit AttributePathParams(EndpointId aEndpointId) :
        AttributePathParams(aEndpointId, kInvalidClusterId, kInvalidAttributeId, kInvalidListIndex)
    {}

    //
    // TODO: (Issue #10596) Need to ensure that we do not encode the NodeId over the wire
    // if it is either not 'set', or is set to a value that matches accessing fabric
    // on which the interaction is undertaken.
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
        mClusterId(aClusterId), mAttributeId(aAttributeId), mEndpointId(aEndpointId), mListIndex(aListIndex)
    {}

    [[nodiscard]] bool IsWildcardPath() const
    {
        return HasWildcardEndpointId() || HasWildcardClusterId() || HasWildcardAttributeId();
    }

    bool operator==(const AttributePathParams & aOther) const
    {
        return mEndpointId == aOther.mEndpointId && mClusterId == aOther.mClusterId && mAttributeId == aOther.mAttributeId &&
            mListIndex == aOther.mListIndex;
    }

    /**
     * SPEC 8.9.2.2
     * Check that the path meets some basic constraints of an attribute path: If list index is not wildcard, then field id must not
     * be wildcard. This does not verify that the attribute being targeted is actually of list type when the list index is not
     * wildcard.
     */
    [[nodiscard]] bool IsValidAttributePath() const { return HasWildcardListIndex() || !HasWildcardAttributeId(); }

    [[nodiscard]] inline bool HasWildcardEndpointId() const { return mEndpointId == kInvalidEndpointId; }
    [[nodiscard]] inline bool HasWildcardClusterId() const { return mClusterId == kInvalidClusterId; }
    [[nodiscard]] inline bool HasWildcardAttributeId() const { return mAttributeId == kInvalidAttributeId; }
    [[nodiscard]] inline bool HasWildcardListIndex() const { return mListIndex == kInvalidListIndex; }
    inline void SetWildcardEndpointId() { mEndpointId = kInvalidEndpointId; }
    inline void SetWildcardClusterId() { mClusterId = kInvalidClusterId; }
    inline void SetWildcardAttributeId()
    {
        mAttributeId = kInvalidAttributeId;
        mListIndex   = kInvalidListIndex;
    }

    [[nodiscard]] bool IsAttributePathSupersetOf(const AttributePathParams & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardAttributeId() || mAttributeId == other.mAttributeId, false);
        VerifyOrReturnError(HasWildcardListIndex() || mListIndex == other.mListIndex, false);

        return true;
    }

    [[nodiscard]] bool IsAttributePathSupersetOf(const ConcreteAttributePath & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardAttributeId() || mAttributeId == other.mAttributeId, false);

        return true;
    }

    bool Intersects(const AttributePathParams & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || other.HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || other.HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardAttributeId() || other.HasWildcardAttributeId() || mAttributeId == other.mAttributeId,
                            false);
        return true;
    }

    bool IncludesAttributesInCluster(const DataVersionFilter & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);

        return true;
    }

    // check if input concrete cluster path is subset of current wildcard attribute
    bool IncludesAllAttributesInCluster(const ConcreteClusterPath & aOther) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == aOther.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == aOther.mClusterId, false);
        return HasWildcardAttributeId();
    }

    ClusterId mClusterId     = kInvalidClusterId;   // uint32
    AttributeId mAttributeId = kInvalidAttributeId; // uint32
    EndpointId mEndpointId   = kInvalidEndpointId;  // uint16
    ListIndex mListIndex     = kInvalidListIndex;   // uint16
};

} // namespace app
} // namespace chip
