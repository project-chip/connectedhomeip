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

#include <app/AttributePathParams.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
struct ClusterInfo
{
    ClusterInfo() {}
    bool IsDirty() { return mDirty; }
    void SetDirty() { mDirty = true; }
    void ClearDirty() { mDirty = false; }

    CHIP_ERROR PushAttributePathSelectorHead();
    void PopAllAttributePathSelector();

    NodeId mNodeId         = 0;
    ClusterId mClusterId   = 0;
    EventId mEventId       = 0;
    AttributePathSelector * mpAttributePathSelector = nullptr;
    ClusterInfo * mpNext   = nullptr;
    EndpointId mEndpointId = 0;
    bool mDirty            = false;

    /* For better structure alignment
     * Above ordering is by bit-size to ensure least amount of memory alignment padding.
     * Changing order to something more natural (e.g. clusterid before nodeid) will result
     * in extra memory alignment padding.
     * uint64 mNodeId
     * uint16_t mClusterId
     * uint16_t EventId
     * uint32_t mAttributeSelectors
     * uint32_t mpNext
     * uint8_t EndpointId
     * uint8_t mDirty
     * padding 1bytes
     */
};
} // namespace app
} // namespace chip
