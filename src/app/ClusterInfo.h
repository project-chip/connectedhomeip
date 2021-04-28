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

/**
 *
 * @brief This is used to record the cluster's attribute path in server side read or subscription client interested in, for read interaction,
 * read handler would mark those interested clusterInfo as dirty when receiving read request, then reporting engine would send out those
 * dirty changes to client per interested clusterInfo, considering IM timed interaction, reporting engine would send out the dirty changes with delay.
 * For subscription, in server side, if user modifies some of interested clusters, and those corresponding clusterInfo would be marked dirty,
 * reporting engine would send them out.
 *
 */
struct ClusterInfo
{
    ClusterInfo(const AttributePathParams & aAttributePathParams, bool aDirty) :
        mAttributePathParams(aAttributePathParams), mDirty(aDirty)
    {}
    bool IsDirty() { return mDirty; }
    void SetDirty() { mDirty = true; }
    void ClearDirty() { mDirty = false; }
    bool IsSamePath(const ClusterInfo & other) const { return other.mAttributePathParams.IsSamePath(mAttributePathParams); }
    AttributePathParams mAttributePathParams;
    bool mDirty = false;
};
} // namespace app
} // namespace chip
