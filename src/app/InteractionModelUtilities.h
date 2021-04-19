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

enum class CommandPathFlags : uint8_t
{
    kEndpointIdValid = 0x01,
    kGroupIdValid    = 0x02,
};

struct CommandPathParams
{
    CommandPathParams(EndpointId aEndpointId, GroupId aGroupId, ClusterId aClusterId, CommandId aCommandId,
                      const BitFlags<CommandPathFlags> & aFlags) :
        mEndpointId(aEndpointId),
        mGroupId(aGroupId), mClusterId(aClusterId), mCommandId(aCommandId), mFlags(aFlags)
    {}
    bool operator==(const CommandPathParams & aCommandPathParams) const
    {
        return ((aCommandPathParams.mClusterId == mClusterId) && (aCommandPathParams.mCommandId == mCommandId) &&
                (mFlags.Has(aCommandPathParams.mFlags)) &&
                (mFlags.Has(CommandPathFlags::kEndpointIdValid) ? aCommandPathParams.mEndpointId == mEndpointId : false) &&
                (mFlags.Has(CommandPathFlags::kGroupIdValid) ? aCommandPathParams.mGroupId == mGroupId : false));
    }
    EndpointId mEndpointId            = 0;
    GroupId mGroupId                  = 0;
    ClusterId mClusterId              = 0;
    CommandId mCommandId              = 0;
    BitFlags<CommandPathFlags> mFlags = CommandPathFlags::kEndpointIdValid;
};

enum class AttributePathFlags : uint8_t
{
    kFieldIdValid   = 0x01,
    kListIndexValid = 0x02,
};

struct AttributePathParams
{
    AttributePathParams(chip::NodeId aNodeId, chip::EndpointId aEndpointId, chip::ClusterId aClusterId, FieldId aFieldId,
                        ListIndex aListIndex, const BitFlags<AttributePathFlags> & aFlags) :
        mNodeId(aNodeId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mFieldId(aFieldId), mListIndex(aListIndex), mFlags(aFlags)
    {}

    bool operator==(const AttributePathParams & aAttributePathParams) const
    {
        return ((aAttributePathParams.mNodeId == mNodeId) && (aAttributePathParams.mEndpointId == mEndpointId) &&
                (aAttributePathParams.mClusterId == mClusterId) && (mFlags.Has(aAttributePathParams.mFlags)) &&
                (mFlags.Has(AttributePathFlags::kFieldIdValid) ? aAttributePathParams.mFieldId == mFieldId : false) &&
                (mFlags.Has(AttributePathFlags::kListIndexValid) ? aAttributePathParams.mListIndex == mListIndex : false));
    }
    chip::NodeId mNodeId                = 0;
    chip::EndpointId mEndpointId        = 0;
    chip::ClusterId mClusterId          = 0;
    chip::FieldId mFieldId              = 0;
    chip::ListIndex mListIndex          = 0;
    BitFlags<AttributePathFlags> mFlags = AttributePathFlags::kFieldIdValid;
};

struct EventPathParams
{
    EventPathParams(chip::NodeId aNodeId, chip::EndpointId aEndpointId, chip::ClusterId aClusterId, chip::EventId aEventId,
                    bool aIsUrgent) :
        mNodeId(aNodeId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId), mIsUrgent(aIsUrgent)
    {}
    bool operator==(const EventPathParams & aEventPathParams) const
    {
        return ((aEventPathParams.mNodeId == mNodeId) && (aEventPathParams.mEndpointId == mEndpointId) &&
                (aEventPathParams.mClusterId == mClusterId) && (aEventPathParams.mEventId == mEventId));
    }
    NodeId mNodeId         = 0;
    EndpointId mEndpointId = 0;
    ClusterId mClusterId   = 0;
    EventId mEventId       = 0;
    bool mIsUrgent         = false;
};

struct ClusterInfo
{
    bool IsDirty(void) { return mDirty; }
    void SetDirty(void) { mDirty = true; }
    void ClearDirty(void) { mDirty = false; }
    bool operator==(const ClusterInfo & aClusterInfo) const
    {
        return ((aClusterInfo.mAttributePathParams == mAttributePathParams));
    }
    AttributePathParams mAttributePathParams;
    bool mDirty = false;
};
} // namespace app
} // namespace chip
