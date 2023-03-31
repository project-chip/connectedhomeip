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

/**
 * @file
 *   Defines an iterator for iterating all possible paths from a list of AttributePathParams-s according to spec section 8.9.2.2
 * (Valid Attribute Paths)
 */

#pragma once

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

/**
 * AttributePathExpandIterator is used to iterate over a linked list of AttributePathParams-s.
 * The AttributePathExpandIterator is copiable, however, the given cluster info must be valid when calling Next().
 *
 * AttributePathExpandIterator will expand attribute paths with wildcards, and only emit existing paths for AttributePathParams with
 * wildcards. For AttributePathParams with a concrete path (i.e. does not contain wildcards), AttributePathExpandIterator will emit
 * them as-is.
 *
 * The typical use of AttributePathExpandIterator may look like:
 * ConcreteAttributePath path;
 * for (AttributePathExpandIterator iterator(AttributePathParams); iterator.Get(path); iterator.Next()) {...}
 *
 * The iterator does not copy the given AttributePathParams, The given AttributePathParams must be valid when using the iterator.
 * If the set of endpoints, clusters, or attributes that are supported changes, AttributePathExpandIterator must be reinitialized.
 *
 * A initialized iterator will return the first valid path, no need to call Next() before calling Get() for the first time.
 *
 * Note: The Next() and Get() are two separate operations by design since a possible call of this iterator might be:
 * - Get()
 * - Chunk full, return
 * - In a new chunk, Get()
 *
 * TODO: The AttributePathParams may support a group id, the iterator should be able to call group data provider to expand the group
 * id.
 */
class AttributePathExpandIterator
{
public:
    AttributePathExpandIterator(ObjectList<AttributePathParams> * aAttributePath);

    /**
     * Proceed the iterator to the next attribute path in the given cluster info.
     *
     * Returns false if AttributePathExpandIterator has exhausted all paths in the given AttributePathParams list.
     */
    bool Next();

    /**
     * Fills the aPath with the path the iterator currently points to.
     * Returns false if the iterator is not pointing to a valid path (i.e. it has exhausted the cluster info).
     */
    bool Get(ConcreteAttributePath & aPath)
    {
        aPath = mOutputPath;
        return Valid();
    }

    /**
     * Reset the iterator to the beginning of current cluster if we are in the middle of expanding a wildcard attribute id for some
     * cluster.
     *
     * When attributes are changed in the middle of expanding a wildcard attribute, we need to reset the iterator, to provide the
     * client with a consistent state of the cluster.
     */
    void ResetCurrentCluster();

    /**
     * Returns if the iterator is valid (not exhausted). An iterator is exhausted if and only if:
     * - Next() is called after iterating last path.
     * - Iterator is initialized with a null AttributePathParams.
     */
    inline bool Valid() const { return mpAttributePath != nullptr; }

private:
    ObjectList<AttributePathParams> * mpAttributePath;

    ConcreteAttributePath mOutputPath;

    uint16_t mEndpointIndex, mEndEndpointIndex;
    uint16_t mAttributeIndex, mEndAttributeIndex;

    // Note: should use decltype(EmberAfEndpointType::clusterCount) here, but af-types is including app specific generated files.
    uint8_t mClusterIndex, mEndClusterIndex;
    // For dealing with global attributes that are not part of the attribute
    // metadata.
    uint8_t mGlobalAttributeIndex, mGlobalAttributeEndIndex;

    /**
     * Prepare*IndexRange will update mBegin*Index and mEnd*Index variables.
     * If AttributePathParams contains a wildcard field, it will set mBegin*Index to 0 and mEnd*Index to count.
     * Or it will set mBegin*Index to the index of the Endpoint/Cluster/Attribute, and mEnd*Index to mBegin*Index + 1.
     *
     * If the Endpoint/Cluster/Attribute does not exist, mBegin*Index will be UINT*_MAX, and mEnd*Inde will be 0.
     *
     * The index can be used with emberAfEndpointFromIndex, emberAfGetNthClusterId and emberAfGetServerAttributeIdByIndex.
     */
    void PrepareEndpointIndexRange(const AttributePathParams & aAttributePath);
    void PrepareClusterIndexRange(const AttributePathParams & aAttributePath, EndpointId aEndpointId);
    void PrepareAttributeIndexRange(const AttributePathParams & aAttributePath, EndpointId aEndpointId, ClusterId aClusterId);
};
} // namespace app
} // namespace chip
