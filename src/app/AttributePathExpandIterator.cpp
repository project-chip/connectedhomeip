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
#include "lib/core/DataModelTypes.h"
#include <app/AttributePathExpandIterator.h>

#include <app/GlobalAttributes.h>
#include <lib/support/CodeUtils.h>

#include <optional>

using namespace chip::app::DataModel;

namespace chip {
namespace app {

AttributePathExpandIterator::AttributePathExpandIterator(DataModel::Provider * dataModel, Position & position) :
    mDataModelProvider(dataModel), mPosition(position)
{
    // TODO: initialize position iterators if position is set
    if (mPosition.mOutputPath.mEndpointId != kInvalidEndpointId)
    {
        mEndpointIterator = mDataModelProvider->GetEndpoints();
        mEndpointIterator->SeekTo(mPosition.mOutputPath.mEndpointId);

        if (mPosition.mOutputPath.mClusterId != kInvalidClusterId)
        {
            mClusterIterator = mDataModelProvider->GetServerClusters(mPosition.mOutputPath.mEndpointId);

            // slow manual seek as server clusters are not seekable
            for (std::optional<ClusterId> clusterId = mClusterIterator->Next(); clusterId.has_value();
                 clusterId                          = mClusterIterator->Next())
            {
                if (*clusterId == mPosition.mOutputPath.mClusterId)
                {
                    break;
                }
            }

            if (mPosition.mOutputPath.mAttributeId != kInvalidAttributeId)
            {
                mAttributeIterator = mDataModelProvider->GetAttributes(mPosition.mOutputPath);
                mAttributeIterator->SeekTo(mPosition.mOutputPath.mAttributeId);
            }
        }
    }
}

bool AttributePathExpandIterator::AdvanceOutputPath()
{
    /// Output path invariants
    ///    - kInvalid* constants are used to define "no value available (yet)" and
    ///      iteration loop will fill the first value when such a value is seen (fixed for non-wildcard
    ///      or iteration-based in case of wildcards).
    ///    - Iteration of the output path is done in breadth-first order: first endpoint, then cluster, then attribute.
    /// Processing works like:
    ///    - Initial state is kInvalidEndpointId/kInvalidClusterId/kInvalidAttributeId
    ///    - First loop pass fills-in endointID, followed by clusterID, followed by attributeID
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

        // No valid cluster, try advance the endpoint, see if a suitable on exists.
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

std::optional<AttributeId> AttributePathExpandIterator::NextAttributeId()
{
    if (mPosition.mOutputPath.mAttributeId == kInvalidAttributeId)
    {
        if (mPosition.mAttributePath->mValue.HasWildcardAttributeId())
        {
            mAttributeIterator = mDataModelProvider->GetAttributes(mPosition.mOutputPath);
        }
        else
        {
            // At this point, the attributeID is NOT a wildcard (i.e. it is fixed)
            //
            // For wildcard expansion, we validate that this is a valid attribute for for the given
            // cluster on the given endpoint. If not a wildcard expansion, return it as-is
            if (mPosition.mAttributePath->mValue.IsWildcardPath())
            {
                const auto attributeId = mPosition.mAttributePath->mValue.mAttributeId;

                if (!IsSupportedGlobalAttributeNotInMetadata(attributeId) &&
                    !mDataModelProvider->GetAttributes(mPosition.mOutputPath)->SeekTo(attributeId))
                {
                    return std::nullopt;
                }
            }
            return mPosition.mAttributePath->mValue.mAttributeId;
        }
    }

    // Advance the existing attribute id if it can be advanced.
    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardAttributeId(), std::nullopt);

    // Ensure (including ordering) that GlobalAttributesNotInMetadata is reported as needed
    for (unsigned i = 0; i < ArraySize(GlobalAttributesNotInMetadata); i++)
    {
        if (GlobalAttributesNotInMetadata[i] != mPosition.mOutputPath.mAttributeId)
        {
            continue;
        }

        unsigned nextAttributeIndex = i + 1;
        if (nextAttributeIndex < ArraySize(GlobalAttributesNotInMetadata))
        {
            return GlobalAttributesNotInMetadata[nextAttributeIndex];
        }

        // Reached the end of global attributes. Since global attributes are
        // reported last, finishing global attributes means everything completed.
        return std::nullopt;
    }

    std::optional<AttributeId> attributeId = mAttributeIterator->Next();
    if (attributeId.has_value())
    {
        return attributeId;
    }

    // Finished the data model, start with global attributes
    static_assert(ArraySize(GlobalAttributesNotInMetadata) > 0);
    return GlobalAttributesNotInMetadata[0];
}

std::optional<ClusterId> AttributePathExpandIterator::NextClusterId()
{

    if (mPosition.mOutputPath.mClusterId == kInvalidClusterId)
    {
        if (!mPosition.mAttributePath->mValue.HasWildcardClusterId())
        {
            // At this point, the clusterID is NOT a wildcard (i.e. is fixed).
            //
            // For wildcard expansion, we validate that this is a valid cluster for the endpoint.
            // If non-wildcard expansion, we return as-is.
            if (mPosition.mAttributePath->mValue.IsWildcardPath())
            {

                // check if for the current endpoint the given cluster is valid
                mClusterIterator                   = mDataModelProvider->GetServerClusters(mPosition.mOutputPath.mEndpointId);
                std::optional<ClusterId> clusterId = mClusterIterator->Next();

                while (clusterId.has_value() && (clusterId != mPosition.mAttributePath->mValue.mClusterId))
                {
                    clusterId = mClusterIterator->Next();
                }

                // we should have found this cluster id on the given endpoint, otherwise wildcard expansion fails
                VerifyOrReturnValue(clusterId.has_value(), std::nullopt);
            }
            return mPosition.mAttributePath->mValue.mClusterId;
        }

        mClusterIterator = mDataModelProvider->GetServerClusters(mPosition.mOutputPath.mEndpointId);
    }

    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardClusterId(), std::nullopt);

    return mClusterIterator->Next();
}

std::optional<ClusterId> AttributePathExpandIterator::NextEndpointId()
{
    if (mPosition.mOutputPath.mEndpointId == kInvalidEndpointId)
    {
        if (!mPosition.mAttributePath->mValue.HasWildcardEndpointId())
        {
            return mPosition.mAttributePath->mValue.mEndpointId;
        }

        mEndpointIterator = mDataModelProvider->GetEndpoints();
    }

    // Expand endpoints only if it is a wildcard on the endpoint specifically.
    VerifyOrReturnValue(mPosition.mAttributePath->mValue.HasWildcardEndpointId(), std::nullopt);

    return mEndpointIterator->Next();
}

} // namespace app
} // namespace chip
