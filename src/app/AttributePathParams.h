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

#include <app/util/basic-types.h>
#include <support/BitFlags.h>

namespace chip {
namespace app {
enum class AttributePathFlags : uint8_t
{
    kFieldIdValid   = 0x01,
    kListIndexValid = 0x02,
};

struct AttributePathParams
{
    AttributePathParams(NodeId aNodeId, EndpointId aEndpointId, ClusterId aClusterId, FieldId aFieldId, ListIndex aListIndex,
                        const BitFlags<AttributePathFlags> aFlags) :
        mNodeId(aNodeId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mFieldId(aFieldId), mListIndex(aListIndex), mFlags(aFlags)
    {}
    AttributePathParams(const AttributePathParams & aAttributePathParams)
    {
        mNodeId     = aAttributePathParams.mNodeId;
        mEndpointId = aAttributePathParams.mEndpointId;
        mClusterId  = aAttributePathParams.mClusterId;
        mFieldId    = aAttributePathParams.mFieldId;
        mListIndex  = aAttributePathParams.mListIndex;
        mFlags      = aAttributePathParams.mFlags;
    }
    bool IsSamePath(const AttributePathParams & other) const
    {
        if (other.mNodeId != mNodeId || other.mEndpointId != mEndpointId || other.mClusterId != mClusterId)
        {
            return false;
        }
        if (mFlags != other.mFlags)
        {
            return false;
        }
        if (mFlags == AttributePathFlags::kFieldIdValid && other.mFieldId != mFieldId)
        {
            return false;
        }
        if (mFlags == AttributePathFlags::kListIndexValid && other.mListIndex != mListIndex)
        {
            return false;
        }
        return true;
    }
    chip::NodeId mNodeId         = 0;
    chip::EndpointId mEndpointId = 0;
    chip::ClusterId mClusterId   = 0;
    chip::FieldId mFieldId       = 0;
    chip::ListIndex mListIndex   = 0;
    BitFlags<AttributePathFlags> mFlags;
};
} // namespace app
} // namespace chip
