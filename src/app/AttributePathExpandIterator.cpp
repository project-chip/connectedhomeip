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

AttributePathExpandIteratorLegacy::AttributePathExpandIteratorLegacy(DataModel::Provider * provider,
                                                         SingleLinkedListNode<AttributePathParams> * attributePath) :
    mDataModelProvider(provider), mpAttributePath(attributePath),
    mOutputPath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)

{
    mOutputPath.mExpanded = true; // this is reset in 'next' if needed

    // Make the iterator ready to emit the first valid path in the list.
    // TODO: the bool return value here is completely unchecked
    Next();
}

bool AttributePathExpandIteratorLegacy::IsValidAttributeId(AttributeId attributeId)
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

    const ConcreteAttributePath attributePath(mOutputPath.mEndpointId, mOutputPath.mClusterId, attributeId);
    return mDataModelProvider->GetAttributeInfo(attributePath).has_value();
}

std::optional<AttributeId> AttributePathExpandIteratorLegacy::NextAttributeId()
{
    if (mOutputPath.mAttributeId == kInvalidAttributeId)
    {
        if (mpAttributePath->mValue.HasWildcardAttributeId())
        {
            AttributeEntry entry = mDataModelProvider->FirstAttribute(mOutputPath);
            return entry.IsValid()                                         //
                ? entry.path.mAttributeId                                  //
                : Clusters::Globals::Attributes::GeneratedCommandList::Id; //
        }

        // We allow fixed attribute IDs if and only if they are valid:
        //    - they may be GLOBAL attributes OR
        //    - they are valid attributes for this cluster
        if (IsValidAttributeId(mpAttributePath->mValue.mAttributeId))
        {
            return mpAttributePath->mValue.mAttributeId;
        }

        return std::nullopt;
    }

    // advance the existing attribute id if it can be advanced
    VerifyOrReturnValue(mpAttributePath->mValue.HasWildcardAttributeId(), std::nullopt);

    // Ensure (including ordering) that GlobalAttributesNotInMetadata is reported as needed
    for (unsigned i = 0; i < ArraySize(GlobalAttributesNotInMetadata); i++)
    {
        if (GlobalAttributesNotInMetadata[i] != mOutputPath.mAttributeId)
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

    AttributeEntry entry = mDataModelProvider->NextAttribute(mOutputPath);
    if (entry.IsValid())
    {
        return entry.path.mAttributeId;
    }

    // Finished the data model, start with global attributes
    static_assert(ArraySize(GlobalAttributesNotInMetadata) > 0);
    return GlobalAttributesNotInMetadata[0];
}

std::optional<ClusterId> AttributePathExpandIteratorLegacy::NextClusterId()
{

    if (mOutputPath.mClusterId == kInvalidClusterId)
    {
        if (mpAttributePath->mValue.HasWildcardClusterId())
        {
            ClusterEntry entry = mDataModelProvider->FirstServerCluster(mOutputPath.mEndpointId);
            return entry.IsValid() ? std::make_optional(entry.path.mClusterId) : std::nullopt;
        }

        // only return a cluster if it is valid
        const ConcreteClusterPath clusterPath(mOutputPath.mEndpointId, mpAttributePath->mValue.mClusterId);
        if (!mDataModelProvider->GetServerClusterInfo(clusterPath).has_value())
        {
            return std::nullopt;
        }

        return mpAttributePath->mValue.mClusterId;
    }

    VerifyOrReturnValue(mpAttributePath->mValue.HasWildcardClusterId(), std::nullopt);

    ClusterEntry entry = mDataModelProvider->NextServerCluster(mOutputPath);
    return entry.IsValid() ? std::make_optional(entry.path.mClusterId) : std::nullopt;
}

std::optional<ClusterId> AttributePathExpandIteratorLegacy::NextEndpointId()
{
    if (mOutputPath.mEndpointId == kInvalidEndpointId)
    {
        if (mpAttributePath->mValue.HasWildcardEndpointId())
        {
            EndpointEntry ep = mDataModelProvider->FirstEndpoint();
            return (ep.id != kInvalidEndpointId) ? std::make_optional(ep.id) : std::nullopt;
        }

        return mpAttributePath->mValue.mEndpointId;
    }

    VerifyOrReturnValue(mpAttributePath->mValue.HasWildcardEndpointId(), std::nullopt);

    EndpointEntry ep = mDataModelProvider->NextEndpoint(mOutputPath.mEndpointId);
    return (ep.id != kInvalidEndpointId) ? std::make_optional(ep.id) : std::nullopt;
}

void AttributePathExpandIteratorLegacy::ResetCurrentCluster()
{
    // If this is a null iterator, or the attribute id of current cluster info is not a wildcard attribute id, then this function
    // will do nothing, since we won't be expanding the wildcard attribute ids under a cluster.
    VerifyOrReturn(mpAttributePath != nullptr && mpAttributePath->mValue.HasWildcardAttributeId());

    // Reset path expansion to ask for the first attribute of the current cluster
    mOutputPath.mAttributeId = kInvalidAttributeId;
    mOutputPath.mExpanded    = true; // we know this is a wildcard attribute
    Next();
}
bool AttributePathExpandIteratorLegacy::AdvanceOutputPath()
{
    if (!mpAttributePath->mValue.IsWildcardPath())
    {
        if (mOutputPath.mEndpointId != kInvalidEndpointId)
        {
            return false; // cannot expand non-wildcard path
        }

        mOutputPath.mEndpointId  = mpAttributePath->mValue.mEndpointId;
        mOutputPath.mClusterId   = mpAttributePath->mValue.mClusterId;
        mOutputPath.mAttributeId = mpAttributePath->mValue.mAttributeId;
        mOutputPath.mExpanded    = false;
        return true;
    }

    while (true)
    {
        if (mOutputPath.mClusterId != kInvalidClusterId)
        {

            std::optional<AttributeId> nextAttribute = NextAttributeId();
            if (nextAttribute.has_value())
            {
                mOutputPath.mAttributeId = *nextAttribute;
                return true;
            }
        }

        // no valid attribute, try to advance the cluster, see if a suitable one exists
        if (mOutputPath.mEndpointId != kInvalidEndpointId)
        {
            std::optional<ClusterId> nextCluster = NextClusterId();
            if (nextCluster.has_value())
            {
                mOutputPath.mClusterId   = *nextCluster;
                mOutputPath.mAttributeId = kInvalidAttributeId; // restarts attributes
                continue;
            }
        }

        // no valid cluster, try advance the endpoint, see if a suitable on exists
        std::optional<EndpointId> nextEndpoint = NextEndpointId();
        if (nextEndpoint.has_value())
        {
            mOutputPath.mEndpointId = *nextEndpoint;
            mOutputPath.mClusterId  = kInvalidClusterId; // restarts clusters
            continue;
        }
        return false;
    }
}

bool AttributePathExpandIteratorLegacy::Next()
{
    while (mpAttributePath != nullptr)
    {
        if (AdvanceOutputPath())
        {
            return true;
        }
        mpAttributePath = mpAttributePath->mpNext;
        mOutputPath     = ConcreteReadAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId);

        mOutputPath.mExpanded = true; // this is reset to false on advancement if needed
    }

    mOutputPath = ConcreteReadAttributePath();
    return false;
}

///// 2nd implementation

bool AttributePathExpandIterator2::AdvanceOutputPath()
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
                mState.mLastOutputPath.mClusterId   = *nextCluster;
                mState.mLastOutputPath.mAttributeId = kInvalidAttributeId; // restarts attributes
                continue;
            }
        }

        // no valid cluster, try advance the endpoint, see if a suitable on exists
        std::optional<EndpointId> nextEndpoint = NextEndpointId();
        if (nextEndpoint.has_value())
        {
            mState.mLastOutputPath.mEndpointId = *nextEndpoint;
            mState.mLastOutputPath.mClusterId  = kInvalidClusterId; // restarts clusters
            continue;
        }
        return false;
    }
}

bool AttributePathExpandIterator2::Next(ConcreteAttributePath & path)
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

bool AttributePathExpandIterator2::IsValidAttributeId(AttributeId attributeId)
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

std::optional<AttributeId> AttributePathExpandIterator2::NextAttributeId()
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

std::optional<ClusterId> AttributePathExpandIterator2::NextClusterId()
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

std::optional<ClusterId> AttributePathExpandIterator2::NextEndpointId()
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
