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
