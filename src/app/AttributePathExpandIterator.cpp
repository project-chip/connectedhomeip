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
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

#include <optional>

using namespace chip::app::DataModel;

namespace chip {
namespace app {

AttributePathExpandIterator::AttributePathExpandIterator(DataModel::Provider * dataModel, Position & position) :
    mDataModelProvider(dataModel), mPosition(position)
{}

bool AttributePathExpandIterator::AdvanceOutputPath()
{
    /// Output path invariants
    ///    - kInvalid* constants are used to define "no value available (yet)" and
    ///      iteration loop will fill the first value when such a value is seen (fixed for non-wildcard
    ///      or iteration-based in case of wildcards).
    ///    - Iteration of the output path is done in order: first endpoint, then cluster, then attribute.
    /// Processing works like:
    ///    - Initial state is kInvalidEndpointId/kInvalidClusterId/kInvalidAttributeId
    ///    - First loop pass fills-in endpointID, followed by clusterID, followed by attributeID
    ///    - Whenever one level is done iterating (there is no "next") the following
    ///      "higher path component" is updated:
    ///         - once a valid path exists, try to advance attributeID
    ///         - if attributeID fails to advance, try to advance clusterID (and restart attributeID)
    ///         - if clusterID fails to advance, try to advance endpointID (and restart clusterID)
    ///         - if endpointID fails to advance, iteration is done
    while (true)
    {
        if (mPosition.mOutputPath.mClusterId != kInvalidClusterId)
        {
            std::optional<AttributeId> nextAttribute = NextAttributeId();
            if (nextAttribute.has_value())
            {
                mPosition.mOutputPath.mAttributeId = *nextAttribute;
                mPosition.mOutputPath.mExpanded    = mPosition.mAttributePath->mValue.IsWildcardPath();
                return true;
            }
        }

        // no valid attribute, try to advance the cluster, see if a suitable one exists
        if (mPosition.mOutputPath.mEndpointId != kInvalidEndpointId)
        {
            std::optional<ClusterId> nextCluster = NextClusterId();
            if (nextCluster.has_value())
            {
                // A new cluster ID is to be processed. This sets the cluster ID to the new value and
                // ALSO resets the attribute ID to "invalid", to trigger an attribute set/expansion from
                // the beginning.
                mPosition.mOutputPath.mClusterId   = *nextCluster;
                mPosition.mOutputPath.mAttributeId = kInvalidAttributeId;
                continue;
            }
        }

        // No valid cluster, try advance the endpoint, see if a suitable one exists.
        std::optional<EndpointId> nextEndpoint = NextEndpointId();
        if (nextEndpoint.has_value())
        {
            // A new endpoint ID is to be processed. This sets the endpoint ID to the new value and
            // ALSO resets the cluster ID to "invalid", to trigger a cluster set/expansion from
            // the beginning.
            mPosition.mOutputPath.mEndpointId = *nextEndpoint;
            mPosition.mOutputPath.mClusterId  = kInvalidClusterId;
            continue;
        }
        return false;
    }
}

bool AttributePathExpandIterator::Next(ConcreteAttributePath & path)
{
    while (mPosition.mAttributePath != nullptr)
    {
        if (AdvanceOutputPath())
        {
            path = mPosition.mOutputPath;
            return true;
        }
        mPosition.mAttributePath = mPosition.mAttributePath->mpNext;
        mPosition.mOutputPath    = ConcreteReadAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId);
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

    DataModel::AttributeFinder finder(mDataModelProvider);

    const ConcreteAttributePath attributePath(mPosition.mOutputPath.mEndpointId, mPosition.mOutputPath.mClusterId, attributeId);
    return finder.Find(attributePath).has_value();
}

std::optional<AttributeId> AttributePathExpandIterator::NextAttributeId()
{
    if (mPosition.mOutputPath.mAttributeId == kInvalidAttributeId)
    {
        // Attribute ID is tied to attribute index. If no attribute id is available yet
        // this means the index is invalid. Processing logic in output advance only resets
        // attribute ID to invalid when resetting iteration.
        mAttributeIndex = kInvalidIndex;
    }

    if (mAttributeIndex == kInvalidIndex)
    {
        // start a new iteration of attributes on the current cluster path.
        mAttributes = mDataModelProvider->AttributesIgnoreError(mPosition.mOutputPath);

        if (mPosition.mOutputPath.mAttributeId != kInvalidAttributeId)
        {
            // Position on the correct attribute if we have a start point
            mAttributeIndex = 0;
            while ((mAttributeIndex < mAttributes.size()) &&
                   (mAttributes[mAttributeIndex].attributeId != mPosition.mOutputPath.mAttributeId))
            {
                mAttributeIndex++;
            }
        }
    }

    if (mPosition.mOutputPath.mAttributeId == kInvalidAttributeId)
    {
        if (!mPosition.mAttributePath->mValue.HasWildcardAttributeId())
        {
            // The attributeID is NOT a wildcard (i.e. it is fixed).
            //
            // For wildcard expansion, we validate that this is a valid attribute for the given
            // cluster on the given endpoint. If not a wildcard expansion, return it as-is.
            if (mPosition.mAttributePath->mValue.IsWildcardPath())
            {
                if (!IsValidAttributeId(mPosition.mAttributePath->mValue.mAttributeId))
                {
                    return std::nullopt;
                }
            }
            return mPosition.mAttributePath->mValue.mAttributeId;
        }
        mAttributeIndex = 0;
    }
    else
    {
        mAttributeIndex++;
    }

    // Advance the existing attribute id if it can be advanced.
    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardAttributeId(), std::nullopt);

    // Ensure (including ordering) that GlobalAttributesNotInMetadata is reported as needed
    for (unsigned i = 0; i < MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata); i++)
    {
        if (GlobalAttributesNotInMetadata[i] != mPosition.mOutputPath.mAttributeId)
        {
            continue;
        }

        unsigned nextAttributeIndex = i + 1;
        if (nextAttributeIndex < MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata))
        {
            return GlobalAttributesNotInMetadata[nextAttributeIndex];
        }

        // Reached the end of global attributes. Since global attributes are
        // reported last, finishing global attributes means everything completed.
        return std::nullopt;
    }

    if (mAttributeIndex < mAttributes.size())
    {
        return mAttributes[mAttributeIndex].attributeId;
    }

    // Finished the data model, start with global attributes
    static_assert(MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata) > 0);
    return GlobalAttributesNotInMetadata[0];
}

std::optional<ClusterId> AttributePathExpandIterator::NextClusterId()
{
    if (mPosition.mOutputPath.mClusterId == kInvalidClusterId)
    {
        // Cluster ID is tied to cluster index. If no cluster id available yet
        // this means index is invalid. Processing logic in output advance only resets
        // cluster ID to invalid when resetting iteration.
        mClusterIndex = kInvalidIndex;
    }

    if (mClusterIndex == kInvalidIndex)
    {
        // start a new iteration on the current endpoint
        mClusters = mDataModelProvider->ServerClustersIgnoreError(mPosition.mOutputPath.mEndpointId);

        if (mPosition.mOutputPath.mClusterId != kInvalidClusterId)
        {
            // Position on the correct cluster if we have a start point
            mClusterIndex = 0;
            while ((mClusterIndex < mClusters.size()) && (mClusters[mClusterIndex].clusterId != mPosition.mOutputPath.mClusterId))
            {
                mClusterIndex++;
            }
        }
    }

    if (mPosition.mOutputPath.mClusterId == kInvalidClusterId)
    {

        if (!mPosition.mAttributePath->mValue.HasWildcardClusterId())
        {
            // The clusterID is NOT a wildcard (i.e. is fixed).
            //
            // For wildcard expansion, we validate that this is a valid cluster for the endpoint.
            // If non-wildcard expansion, we return as-is.
            if (mPosition.mAttributePath->mValue.IsWildcardPath())
            {
                const ClusterId clusterId = mPosition.mAttributePath->mValue.mClusterId;

                bool found = false;
                for (auto & entry : mClusters)
                {
                    if (entry.clusterId == clusterId)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    return std::nullopt;
                }
            }

            return mPosition.mAttributePath->mValue.mClusterId;
        }
        mClusterIndex = 0;
    }
    else
    {
        mClusterIndex++;
    }

    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardClusterId(), std::nullopt);
    VerifyOrReturnValue(mClusterIndex < mClusters.size(), std::nullopt);

    return mClusters[mClusterIndex].clusterId;
}

std::optional<EndpointId> AttributePathExpandIterator::NextEndpointId()
{
    if (mEndpointIndex == kInvalidIndex)
    {
        // index is missing, have to start a new iteration
        mEndpoints = mDataModelProvider->EndpointsIgnoreError();

        if (mPosition.mOutputPath.mEndpointId != kInvalidEndpointId)
        {
            // Position on the correct endpoint if we have a start point
            mEndpointIndex = 0;
            while ((mEndpointIndex < mEndpoints.size()) && (mEndpoints[mEndpointIndex].id != mPosition.mOutputPath.mEndpointId))
            {
                mEndpointIndex++;
            }
        }
    }

    if (mPosition.mOutputPath.mEndpointId == kInvalidEndpointId)
    {
        if (!mPosition.mAttributePath->mValue.HasWildcardEndpointId())
        {
            return mPosition.mAttributePath->mValue.mEndpointId;
        }

        // start from the beginning
        mEndpointIndex = 0;
    }
    else
    {
        mEndpointIndex++;
    }

    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardEndpointId(), std::nullopt);
    VerifyOrReturnValue(mEndpointIndex < mEndpoints.size(), std::nullopt);

    return mEndpoints[mEndpointIndex].id;
}

} // namespace app
} // namespace chip
