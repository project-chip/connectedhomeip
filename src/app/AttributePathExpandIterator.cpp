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

#include <app-common/zap-generated/att-storage.h>
#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

// TODO: Here we use forward declaration for these symbols used, there should be some reorganize for code in app/util so they can be
// used with generated files or some mock files.
// Note: including headers from app/util does not work since it will include some app specific generged files.
typedef uint8_t EmberAfClusterMask;

extern uint16_t emberAfEndpointCount(void);
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

namespace chip {
namespace app {

AttributePathExpandIterator::AttributePathExpandIterator(ClusterInfo * aClusterInfo)
{
    mpClusterInfo = aClusterInfo;

    // Reset iterator state
    mEndpointIndex  = UINT16_MAX;
    mClusterIndex   = UINT8_MAX;
    mAttributeIndex = UINT16_MAX;

    // Make the iterator ready to emit the first valid path in the list.
    Next();
}

void AttributePathExpandIterator::PrepareEndpointIndexRange(const ClusterInfo & aClusterInfo)
{
    if (aClusterInfo.HasWildcardEndpointId())
    {
        mBeginEndpointIndex = 0;
        mEndEndpointIndex   = emberAfEndpointCount();
    }
    else
    {
        mBeginEndpointIndex = emberAfIndexFromEndpoint(aClusterInfo.mEndpointId);
        // If the given cluster id does not exist on the given endpoint, it will return uint16(0xFFFF), then endEndpointIndex
        // will be 0, means we should iterate a null endpoint set (skip it).
        mEndEndpointIndex = static_cast<uint16_t>(mBeginEndpointIndex + 1);
    }
}

void AttributePathExpandIterator::PrepareClusterIndexRange(const ClusterInfo & aClusterInfo, EndpointId aEndpointId)
{
    if (aClusterInfo.HasWildcardClusterId())
    {
        mBeginClusterIndex = 0;
        mEndClusterIndex   = emberAfClusterCount(aEndpointId, true /* server */);
    }
    else
    {
        mBeginClusterIndex = emberAfClusterIndex(aEndpointId, aClusterInfo.mClusterId, CLUSTER_MASK_SERVER);
        // If the given cluster id does not exist on the given endpoint, it will return uint8(0xFF), then endClusterIndex
        // will be 0, means we should iterate a null cluster set (skip it).
        mEndClusterIndex = static_cast<uint8_t>(mBeginClusterIndex + 1);
    }
}

void AttributePathExpandIterator::PrepareAttributeIndexRange(const ClusterInfo & aClusterInfo, EndpointId aEndpointId,
                                                             ClusterId aClusterId)
{
    if (aClusterInfo.HasWildcardAttributeId())
    {
        mBeginAttributeIndex = 0;
        mEndAttributeIndex   = emberAfGetServerAttributeCount(aEndpointId, aClusterId);
    }
    else
    {
        mBeginAttributeIndex = emberAfGetServerAttributeIndexByAttributeId(aEndpointId, aClusterId, aClusterInfo.mFieldId);
        // If the given attribute id does not exist on the given endpoint, it will return uint16(0xFFFF), then endAttributeIndex
        // will be 0, means we should iterate a null attribute set (skip it).
        mEndAttributeIndex = static_cast<uint16_t>(mBeginAttributeIndex + 1);
    }
}

bool AttributePathExpandIterator::Next()
{
    for (; mpClusterInfo != nullptr; (mpClusterInfo = mpClusterInfo->mpNext, mEndpointIndex = UINT16_MAX))
    {
        // Special case: If this is a concrete path, we just return its value as-is.
        if (!mpClusterInfo->HasWildcard())
        {
            mOutputPath.mEndpointId  = mpClusterInfo->mEndpointId;
            mOutputPath.mClusterId   = mpClusterInfo->mClusterId;
            mOutputPath.mAttributeId = mpClusterInfo->mFieldId;

            // Prepare for next iteration
            (mpClusterInfo = mpClusterInfo->mpNext, mEndpointIndex = UINT16_MAX);
            return true;
        }

        if (mEndpointIndex == UINT16_MAX)
        {
            PrepareEndpointIndexRange(*mpClusterInfo);
            // If we have not started iterating over the endpoints yet.
            mEndpointIndex = mBeginEndpointIndex;
            mClusterIndex  = UINT8_MAX;
        }

        for (; mEndpointIndex < mEndEndpointIndex; (mEndpointIndex++, mClusterIndex = UINT8_MAX, mAttributeIndex = UINT16_MAX))
        {
            EndpointId endpointId = emberAfEndpointFromIndex(mEndpointIndex);

            if (mClusterIndex == UINT8_MAX)
            {
                PrepareClusterIndexRange(*mpClusterInfo, endpointId);
                // If we have not started iterating over the clusters yet.
                mClusterIndex   = mBeginClusterIndex;
                mAttributeIndex = UINT16_MAX;
            }

            for (; mClusterIndex < mEndClusterIndex; (mClusterIndex++, mAttributeIndex = UINT16_MAX))
            {
                // emberAfGetNthClusterId must return a valid cluster id here since we have verified the mClusterIndex does
                // not exceed the mEndClusterIndex.
                ClusterId clusterId = emberAfGetNthClusterId(endpointId, mClusterIndex, true /* server */).Value();
                if (mAttributeIndex == UINT16_MAX)
                {
                    PrepareAttributeIndexRange(*mpClusterInfo, endpointId, clusterId);
                    // If we have not started iterating over the attributes yet.
                    mAttributeIndex = mBeginAttributeIndex;
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
                    // Return true will skip the increment of mClusterIndex, mEndpointIndex and mpClusterInfo.
                    return true;
                }
                // We have exhausted all attributes of this cluster, continue iterating over attributes of next cluster.
            }
            // We have exhausted all clusters of this endpoint, continue iterating over clusters of next endpoint.
        }
        // We have exhausted all endpoints in this cluster info, continue iterating over next cluster info item.
    }

    // Reset to default, invalid value.
    mOutputPath = ConcreteAttributePath();
    return false;
}
} // namespace app
} // namespace chip
