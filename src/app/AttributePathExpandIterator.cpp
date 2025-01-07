/*
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
#include <app/AttributePathExpandIterator.h>

#include <app/GlobalAttributes.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::DataModel;

namespace chip {
namespace app {

bool AttributePathExpandIterator::AdvanceOutputPath()
{
    if (!mState.mAttributePath->mValue.IsWildcardPath())
    {
        if (mState.mLastOutputPath.mEndpointId != kInvalidEndpointId)
        {
            return false; // cannot expand non-wildcard path
        }

        mState.mLastOutputPath.mEndpointId  = mState.mAttributePath->mValue.mEndpointId;
        mState.mLastOutputPath.mClusterId   = mState.mAttributePath->mValue.mClusterId;
        mState.mLastOutputPath.mAttributeId = mState.mAttributePath->mValue.mAttributeId;
        mState.mLastOutputPath.mExpanded    = false;
        return true;
    }

    while (true)
    {
        if (mState.mLastOutputPath.mClusterId != kInvalidClusterId)
        {
            std::optional<AttributeId> nextAttribute = NextAttributeId();
            if (nextAttribute.has_value())
            {
                mState.mLastOutputPath.mAttributeId = *nextAttribute;
                return true;
            }
        }

        // no valid attribute, try to advance the cluster, see if a suitable one exists
        if (mState.mLastOutputPath.mEndpointId != kInvalidEndpointId)
        {
            std::optional<ClusterId> nextCluster = NextClusterId();
            if (nextCluster.has_value())
            {
                // A new cluster ID is to be processed. This sets the cluster ID to the new value and
                // ALSO resets the attribute ID to "invalid", to trigger an attribute set/expansion from
                // the beginning.
                mState.mLastOutputPath.mClusterId   = *nextCluster;
                mState.mLastOutputPath.mAttributeId = kInvalidAttributeId;
                continue;
            }
        }

        // no valid cluster, try advance the endpoint, see if a suitable on exists
        std::optional<EndpointId> nextEndpoint = NextEndpointId();
        if (nextEndpoint.has_value())
        {
            // A new endpoint ID is to be processed. This sets the endpoint ID to the new value and
            // ALSO resets the cluster ID to "invalid", to trigger a cluster set/expansion from
            // the beginning.
            mState.mLastOutputPath.mEndpointId = *nextEndpoint;
            mState.mLastOutputPath.mClusterId  = kInvalidClusterId;
            continue;
        }
        return false;
    }
}

bool AttributePathExpandIterator::Next(ConcreteAttributePath & path)
{
    while (mState.mAttributePath != nullptr)
    {
        if (AdvanceOutputPath())
        {
            path = mState.mLastOutputPath;
            return true;
        }
        mState.mAttributePath            = mState.mAttributePath->mpNext;
        mState.mLastOutputPath           = ConcreteReadAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId);
        mState.mLastOutputPath.mExpanded = true; // this is reset to false on advancement if needed
    }

    return false;
}

bool AttributePathExpandIterator::IsValidAttributeId(AttributeId attributeId)
{
    switch (attributeId)
    {
    case Clusters::Globals::Attributes::GeneratedCommandList::Id:
    case Clusters::Globals::Attributes::AcceptedCommandList::Id:
    case Clusters::Globals::Attributes::AttributeList::Id:
        return true;
    default:
        break;
    }

    const ConcreteAttributePath attributePath(mState.mLastOutputPath.mEndpointId, mState.mLastOutputPath.mClusterId, attributeId);
    return mDataModelProvider->GetAttributeInfo(attributePath).has_value();
}

std::optional<AttributeId> AttributePathExpandIterator::NextAttributeId()
{
    if (mState.mLastOutputPath.mAttributeId == kInvalidAttributeId)
    {
        if (mState.mAttributePath->mValue.HasWildcardAttributeId())
        {
            AttributeEntry entry = mDataModelProvider->FirstAttribute(mState.mLastOutputPath);
            return entry.IsValid()                                         //
                ? entry.path.mAttributeId                                  //
                : Clusters::Globals::Attributes::GeneratedCommandList::Id; //
        }

        // We allow fixed attribute IDs if and only if they are valid:
        //    - they may be GLOBAL attributes OR
        //    - they are valid attributes for this cluster
        if (IsValidAttributeId(mState.mAttributePath->mValue.mAttributeId))
        {
            return mState.mAttributePath->mValue.mAttributeId;
        }

        return std::nullopt;
    }

    // advance the existing attribute id if it can be advanced
    VerifyOrReturnValue(mState.mAttributePath->mValue.HasWildcardAttributeId(), std::nullopt);

    // Ensure (including ordering) that GlobalAttributesNotInMetadata is reported as needed
    for (unsigned i = 0; i < ArraySize(GlobalAttributesNotInMetadata); i++)
    {
        if (GlobalAttributesNotInMetadata[i] != mState.mLastOutputPath.mAttributeId)
        {
            continue;
        }

        unsigned nextAttributeIndex = i + 1;
        if (nextAttributeIndex < ArraySize(GlobalAttributesNotInMetadata))
        {
            return GlobalAttributesNotInMetadata[nextAttributeIndex];
        }

        // reached the end of global attributes
        return std::nullopt;
    }

    AttributeEntry entry = mDataModelProvider->NextAttribute(mState.mLastOutputPath);
    if (entry.IsValid())
    {
        return entry.path.mAttributeId;
    }

    // Finished the data model, start with global attributes
    static_assert(ArraySize(GlobalAttributesNotInMetadata) > 0);
    return GlobalAttributesNotInMetadata[0];
}

std::optional<ClusterId> AttributePathExpandIterator::NextClusterId()
{

    if (mState.mLastOutputPath.mClusterId == kInvalidClusterId)
    {
        if (mState.mAttributePath->mValue.HasWildcardClusterId())
        {
            ClusterEntry entry = mDataModelProvider->FirstServerCluster(mState.mLastOutputPath.mEndpointId);
            return entry.IsValid() ? std::make_optional(entry.path.mClusterId) : std::nullopt;
        }

        // only return a cluster if it is valid
        const ConcreteClusterPath clusterPath(mState.mLastOutputPath.mEndpointId, mState.mAttributePath->mValue.mClusterId);
        if (!mDataModelProvider->GetServerClusterInfo(clusterPath).has_value())
        {
            return std::nullopt;
        }

        return mState.mAttributePath->mValue.mClusterId;
    }

    VerifyOrReturnValue(mState.mAttributePath->mValue.HasWildcardClusterId(), std::nullopt);

    ClusterEntry entry = mDataModelProvider->NextServerCluster(mState.mLastOutputPath);
    return entry.IsValid() ? std::make_optional(entry.path.mClusterId) : std::nullopt;
}

std::optional<ClusterId> AttributePathExpandIterator::NextEndpointId()
{
    if (mState.mLastOutputPath.mEndpointId == kInvalidEndpointId)
    {
        if (mState.mAttributePath->mValue.HasWildcardEndpointId())
        {
            EndpointEntry ep = mDataModelProvider->FirstEndpoint();
            return (ep.id != kInvalidEndpointId) ? std::make_optional(ep.id) : std::nullopt;
        }

        return mState.mAttributePath->mValue.mEndpointId;
    }

    VerifyOrReturnValue(mState.mAttributePath->mValue.HasWildcardEndpointId(), std::nullopt);

    EndpointEntry ep = mDataModelProvider->NextEndpoint(mState.mLastOutputPath.mEndpointId);
    return (ep.id != kInvalidEndpointId) ? std::make_optional(ep.id) : std::nullopt;
}

} // namespace app
} // namespace chip
