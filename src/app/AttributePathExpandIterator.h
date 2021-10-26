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

#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
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
 * AttributePathExpandIterator is used for iterate over a linked list of ClusterInfo-s.
 * The AttributePathExpandIterator is copiable, however, the given cluster info must be valid when calling proceed.
 *
 * AttributePathExpandIterator will expand attribute paths with wildcards, and only emit exist paths for ClusterInfo with
 * wildcards. For ClusterInfo with a concrete path (i.e. does not contain wildcards), AttributePathExpandIterator will emit them
 * as-is.
 *
 * The typical use of AttributePathExpandIterator may look like:
 * ConcreteAttributePath path;
 * for (AttributePathExpandIterator iterator(clusterInfo); iterator.Get(path); iterator.Proceed()) {...}
 *
 * If AttributePathExpandIterator is called with a invalid ClusterInfo (e.g. a freed ClusterInfo), the program may misbehave.
 * If there are new endpoints, clusters or attributes adds, AttributePathExpandIterator must be reseted or it may emit unexpected
 * value.
 */
class AttributePathExpandIterator
{
public:
    AttributePathExpandIterator(ClusterInfo * aClusterInfo) { Reset(aClusterInfo); }

    /**
     * Initialize / reset the state of the path iterator.
     * Get() will return the first valid path in the first ClusterInfo.
     * Passing a nullptr to Reset will clear the state of the AttributePathExpandIterator.
     */
    void Reset(ClusterInfo * aClusterInfo);

    /**
     * Proceed the iterator to the next attribute path in the given cluster info.
     *
     * Feturns false if AttributePathExpandIterator has exhausted all paths in the given ClusterInfo list.
     */
    bool Proceed();

    /**
     * Fills the aPath with the path the iterator current points to.
     * Return false if the iterator is not pointing a valid path (i.e. it has exhausted the cluster info).
     */
    bool Get(ConcreteAttributePath & aPath)
    {
        aPath = mOutputPath;
        return mOutputPath.mEndpointId != ConcreteAttributePath::kInvalidEndpointId;
    }

    bool Valid() { return mpClusterInfo != nullptr; }

private:
    ClusterInfo * mpClusterInfo;

    uint16_t mEndpointIndex, mBeginEndpointIndex, mEndEndpointIndex;
    uint8_t mClusterIndex, mBeginClusterIndex, mEndClusterIndex;
    uint16_t mAttributeIndex, mBeginAttributeIndex, mEndAttributeIndex;

    ConcreteAttributePath mOutputPath = ConcreteAttributePath();

    void PrepareEndpointIndexRange(const ClusterInfo & aClusterInfo);
    void PrepareClusterIndexRange(const ClusterInfo & aClusterInfo, EndpointId aEndpointId);
    void PrepareAttributeIndexRange(const ClusterInfo & aClusterInfo, EndpointId aEndpointId, ClusterId aClusterId);
};
} // namespace app
} // namespace chip
