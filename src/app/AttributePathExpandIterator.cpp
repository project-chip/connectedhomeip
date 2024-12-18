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
#include "lib/support/logging/TextOnlyLogging.h"
#include <app/AttributePathExpandIterator.h>

#include <app/GlobalAttributes.h>
#include <lib/support/CodeUtils.h>

using namespace chip::app::DataModel;

namespace chip {
namespace app {

AttributePathExpandIterator::AttributePathExpandIterator(DataModel::Provider * provider,
                                                         SingleLinkedListNode<AttributePathParams> * attributePath) :
    mDataModelProvider(provider),
    mpAttributePath(attributePath), mOutputPath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)
{
    mOutputPath.mNeedsInitialization = true; // Ensure a Next is called on all public API
    mOutputPath.mExpanded            = true; // this is reset in 'next' if needed
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

    const ConcreteAttributePath attributePath(mOutputPath.mEndpointId, mOutputPath.mClusterId, attributeId);
    return mDataModelProvider->GetAttributeInfo(attributePath).has_value();
}

std::optional<AttributeId> AttributePathExpandIterator::NextAttributeId()
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

std::optional<ClusterId> AttributePathExpandIterator::NextClusterId()
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

AttributePathExpandIterator::SearchSession AttributePathExpandIterator::PrepareSearch()
{
    SearchSession session;

    session.endpoints = mDataModelProvider->GetEndpoints();

    // position on the current endpoint to look up
    if (mOutputPath.mEndpointId != kInvalidEndpointId)
    {
        // we are already positioned on a specific endpoint, so start from there
        if (!session.endpoints->SeekTo(mOutputPath.mEndpointId))
        {
            ChipLogError(InteractionModel, "Endpoint id %d is not valid anymore", mOutputPath.mEndpointId);
        }
    }

    return session;
}

std::optional<ClusterId> AttributePathExpandIterator::NextEndpointId(SearchSession & session)
{
    if (mOutputPath.mEndpointId == kInvalidEndpointId)
    {
        if (!mpAttributePath->mValue.HasWildcardEndpointId())
        {
            // use existing value, we are not wildcarding
            return mpAttributePath->mValue.mEndpointId;
        }
        // start a new iteration
        session.endpoints = mDataModelProvider->GetEndpoints();
    }

    // to advance, we need to be asked for wildcards
    VerifyOrReturnValue(mpAttributePath->mValue.HasWildcardEndpointId(), std::nullopt);

    return session.endpoints->Next();
}

void AttributePathExpandIterator::ResetCurrentCluster(SearchSession & session)
{
    EnsureFirstNextCalled(session);

    // If this is a null iterator, or the attribute id of current cluster info is not a wildcard attribute id, then this function
    // will do nothing, since we won't be expanding the wildcard attribute ids under a cluster.
    VerifyOrReturn(mpAttributePath != nullptr && mpAttributePath->mValue.HasWildcardAttributeId());
    // Reset path expansion to ask for the first attribute of the current cluster
    mOutputPath.mAttributeId = kInvalidAttributeId;
    mOutputPath.mExpanded    = true; // we know this is a wildcard attribute
    Next(session);
}

bool AttributePathExpandIterator::AdvanceOutputPath(SearchSession & session)
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
        std::optional<EndpointId> nextEndpoint = NextEndpointId(session);
        if (nextEndpoint.has_value())
        {
            mOutputPath.mEndpointId = *nextEndpoint;
            mOutputPath.mClusterId  = kInvalidClusterId; // restarts clusters
            continue;
        }
        return false;
    }
}

bool AttributePathExpandIterator::Next(SearchSession & session)
{
    EnsureFirstNextCalled(session);
    while (mpAttributePath != nullptr)
    {
        if (AdvanceOutputPath(session))
        {
            return true;
        }
        mpAttributePath                  = mpAttributePath->mpNext;
        mOutputPath                      = ConcreteReadAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId);
        mOutputPath.mNeedsInitialization = false; // it is final
        mOutputPath.mExpanded            = true;  // this is reset to false on advancement if needed
    }

    mOutputPath                      = ConcreteReadAttributePath();
    mOutputPath.mNeedsInitialization = false; // it is final
    return false;
}

} // namespace app
} // namespace chip
