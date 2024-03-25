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

#include <app/AttributePathExpandIterator.h>

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/GlobalAttributes.h>
#include <app/util/att-storage.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

// TODO: Need to make it so that declarations of things that don't depend on generated files are not intermixed in af.h with
// dependencies on generated files, so we don't have to re-declare things here.
// Note: Some of the generated files that depended by af.h are gen_config.h and gen_tokens.h
typedef uint8_t EmberAfClusterMask;

extern uint16_t emberAfEndpointCount();
extern uint16_t emberAfIndexFromEndpoint(EndpointId endpoint);
extern uint8_t emberAfClusterCount(EndpointId endpoint, bool server);
extern uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster);
extern uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                            chip::AttributeId attributeId);
extern chip::EndpointId emberAfEndpointFromIndex(uint16_t index);
extern Optional<ClusterId> emberAfGetNthClusterId(chip::EndpointId endpoint, uint8_t n, bool server);
extern Optional<AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                uint16_t attributeIndex);
extern uint8_t emberAfClusterIndex(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask);
extern bool emberAfEndpointIndexIsEnabled(uint16_t index);

namespace chip {
namespace app {

AttributePathExpandIterator::AttributePathExpandIterator(SingleLinkedListNode<AttributePathParams> * aAttributePath)
{
    mpAttributePath = aAttributePath;

    // Reset iterator state
    mEndpointIndex  = UINT16_MAX;
    mClusterIndex   = UINT8_MAX;
    mAttributeIndex = UINT16_MAX;

    static_assert(std::numeric_limits<decltype(mGlobalAttributeIndex)>::max() >= ArraySize(GlobalAttributesNotInMetadata),
                  "Our index won't be able to hold the value we need to hold.");
    static_assert(std::is_same<decltype(mGlobalAttributeIndex), uint8_t>::value,
                  "If this changes audit all uses where we set to UINT8_MAX");
    mGlobalAttributeIndex = UINT8_MAX;

    // Make the iterator ready to emit the first valid path in the list.
    Next();
}

void AttributePathExpandIterator::PrepareEndpointIndexRange(const AttributePathParams & aAttributePath)
{
    if (aAttributePath.HasWildcardEndpointId())
    {
        mEndpointIndex    = 0;
        mEndEndpointIndex = emberAfEndpointCount();
    }
    else
    {
        mEndpointIndex = emberAfIndexFromEndpoint(aAttributePath.mEndpointId);
        // If the given cluster id does not exist on the given endpoint, it will return uint16(0xFFFF), then endEndpointIndex
        // will be 0, means we should iterate a null endpoint set (skip it).
        mEndEndpointIndex = static_cast<uint16_t>(mEndpointIndex + 1);
    }
}

void AttributePathExpandIterator::PrepareClusterIndexRange(const AttributePathParams & aAttributePath, EndpointId aEndpointId)
{
    if (aAttributePath.HasWildcardClusterId())
    {
        mClusterIndex    = 0;
        mEndClusterIndex = emberAfClusterCount(aEndpointId, true /* server */);
    }
    else
    {
        mClusterIndex = emberAfClusterIndex(aEndpointId, aAttributePath.mClusterId, CLUSTER_MASK_SERVER);
        // If the given cluster id does not exist on the given endpoint, it will return uint8(0xFF), then endClusterIndex
        // will be 0, means we should iterate a null cluster set (skip it).
        mEndClusterIndex = static_cast<uint8_t>(mClusterIndex + 1);
    }
}

void AttributePathExpandIterator::PrepareAttributeIndexRange(const AttributePathParams & aAttributePath, EndpointId aEndpointId,
                                                             ClusterId aClusterId)
{
    if (aAttributePath.HasWildcardAttributeId())
    {
        mAttributeIndex          = 0;
        mEndAttributeIndex       = emberAfGetServerAttributeCount(aEndpointId, aClusterId);
        mGlobalAttributeIndex    = 0;
        mGlobalAttributeEndIndex = ArraySize(GlobalAttributesNotInMetadata);
    }
    else
    {
        mAttributeIndex = emberAfGetServerAttributeIndexByAttributeId(aEndpointId, aClusterId, aAttributePath.mAttributeId);
        // If the given attribute id does not exist on the given endpoint, it will return uint16(0xFFFF), then endAttributeIndex
        // will be 0, means we should iterate a null attribute set (skip it).
        mEndAttributeIndex = static_cast<uint16_t>(mAttributeIndex + 1);
        if (mAttributeIndex == UINT16_MAX)
        {
            // Check whether this is a non-metadata global attribute.
            //
            // Default to the max value, which will correspond (after we add 1
            // and overflow to 0 for the max index) to us not going through
            // non-metadata global attributes for this attribute.
            mGlobalAttributeIndex = UINT8_MAX;

            static_assert(ArraySize(GlobalAttributesNotInMetadata) <= UINT8_MAX, "Iterating over at most 256 array entries");

            const uint8_t arraySize = static_cast<uint8_t>(ArraySize(GlobalAttributesNotInMetadata));
            for (uint8_t idx = 0; idx < arraySize; ++idx)
            {
                if (GlobalAttributesNotInMetadata[idx] == aAttributePath.mAttributeId)
                {
                    mGlobalAttributeIndex = idx;
                    break;
                }
            }
            mGlobalAttributeEndIndex = static_cast<uint8_t>(mGlobalAttributeIndex + 1);
        }
        else
        {
            mGlobalAttributeIndex    = UINT8_MAX;
            mGlobalAttributeEndIndex = 0;
        }
    }
}

void AttributePathExpandIterator::ResetCurrentCluster()
{
    // If this is a null iterator, or the attribute id of current cluster info is not a wildcard attribute id, then this function
    // will do nothing, since we won't be expanding the wildcard attribute ids under a cluster.
    VerifyOrReturn(mpAttributePath != nullptr && mpAttributePath->mValue.HasWildcardAttributeId());

    // Otherwise, we will reset the index for iterating the attributes, so we report the attributes for this cluster again. This
    // will ensure that the client sees a coherent view of the cluster from the reports generated by a single (wildcard) attribute
    // path in the request.
    //
    // Note that when Next() returns, we must be in one of the following states:
    // - This is not a wildcard path
    // - We just expanded some attribute id field
    // - We have exhausted all paths
    // Only the second case will happen here since the above check will fail for 1 and 3, so the following Next() call must result
    // in a valid path, which is the first attribute id we will emit for the current cluster.
    mAttributeIndex       = UINT16_MAX;
    mGlobalAttributeIndex = UINT8_MAX;
    Next();
}

bool AttributePathExpandIterator::Next()
{
    for (; mpAttributePath != nullptr; (mpAttributePath = mpAttributePath->mpNext, mEndpointIndex = UINT16_MAX))
    {
        mOutputPath.mExpanded = mpAttributePath->mValue.IsWildcardPath();

        if (mEndpointIndex == UINT16_MAX)
        {
            // Special case: If this is a concrete path, we just return its value as-is.
            if (!mpAttributePath->mValue.IsWildcardPath())
            {
                mOutputPath.mEndpointId  = mpAttributePath->mValue.mEndpointId;
                mOutputPath.mClusterId   = mpAttributePath->mValue.mClusterId;
                mOutputPath.mAttributeId = mpAttributePath->mValue.mAttributeId;

                // Prepare for next iteration
                mEndpointIndex = mEndEndpointIndex = 0;
                return true;
            }

            PrepareEndpointIndexRange(mpAttributePath->mValue);
            mClusterIndex = UINT8_MAX;
        }

        for (; mEndpointIndex < mEndEndpointIndex;
             (mEndpointIndex++, mClusterIndex = UINT8_MAX, mAttributeIndex = UINT16_MAX, mGlobalAttributeIndex = UINT8_MAX))
        {
            if (!emberAfEndpointIndexIsEnabled(mEndpointIndex))
            {
                // Not an enabled endpoint; skip it.
                continue;
            }

            EndpointId endpointId = emberAfEndpointFromIndex(mEndpointIndex);

            if (mClusterIndex == UINT8_MAX)
            {
                PrepareClusterIndexRange(mpAttributePath->mValue, endpointId);
                mAttributeIndex       = UINT16_MAX;
                mGlobalAttributeIndex = UINT8_MAX;
            }

            for (; mClusterIndex < mEndClusterIndex;
                 (mClusterIndex++, mAttributeIndex = UINT16_MAX, mGlobalAttributeIndex = UINT8_MAX))
            {
                // emberAfGetNthClusterId must return a valid cluster id here since we have verified the mClusterIndex does
                // not exceed the mEndClusterIndex.
                ClusterId clusterId = emberAfGetNthClusterId(endpointId, mClusterIndex, true /* server */).Value();
                if (mAttributeIndex == UINT16_MAX && mGlobalAttributeIndex == UINT8_MAX)
                {
                    PrepareAttributeIndexRange(mpAttributePath->mValue, endpointId, clusterId);
                }

                if (mAttributeIndex < mEndAttributeIndex)
                {
                    // GetServerAttributeIdByIdex must return a valid attribute here since we have verified the mAttributeIndex does
                    // not exceed the mEndAttributeIndex.
                    mOutputPath.mAttributeId = emberAfGetServerAttributeIdByIndex(endpointId, clusterId, mAttributeIndex).Value();
                    mOutputPath.mClusterId   = clusterId;
                    mOutputPath.mEndpointId  = endpointId;
                    mAttributeIndex++;
                    // We found a valid attribute path, now return and increase the attribute index for next iteration.
                    // Return true will skip the increment of mClusterIndex, mEndpointIndex and mpAttributePath.
                    return true;
                }
                if (mGlobalAttributeIndex < mGlobalAttributeEndIndex)
                {
                    // Return a path pointing to the next global attribute.
                    mOutputPath.mAttributeId = GlobalAttributesNotInMetadata[mGlobalAttributeIndex];
                    mOutputPath.mClusterId   = clusterId;
                    mOutputPath.mEndpointId  = endpointId;
                    mGlobalAttributeIndex++;
                    return true;
                }
                // We have exhausted all attributes of this cluster, continue iterating over attributes of next cluster.
            }
            // We have exhausted all clusters of this endpoint, continue iterating over clusters of next endpoint.
        }
        // We have exhausted all endpoints in this cluster info, continue iterating over next cluster info item.
    }

    // Reset to default, invalid value.
    mOutputPath = ConcreteReadAttributePath();
    return false;
}
} // namespace app
} // namespace chip
