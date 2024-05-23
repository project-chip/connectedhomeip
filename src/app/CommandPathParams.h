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
#include <lib/core/GroupId.h>
#include <lib/support/BitFlags.h>

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

    CommandPathParams(uint16_t aId, ClusterId aClusterId, CommandId aCommandId, const BitFlags<CommandPathFlags> & aFlags) :
        mClusterId(aClusterId), mCommandId(aCommandId), mFlags(aFlags)
    {
        if (aFlags == CommandPathFlags::kEndpointIdValid)
        {
            mEndpointId = aId;
        }
        else if (aFlags == CommandPathFlags::kGroupIdValid)
        {
            mGroupId = aId;
        }
    }

    bool IsSamePath(const CommandPathParams & other) const
    {
        if (other.mClusterId != mClusterId || other.mCommandId != mCommandId)
        {
            return false;
        }
        if (mFlags != other.mFlags)
        {
            return false;
        }
        if (mFlags == CommandPathFlags::kEndpointIdValid && other.mEndpointId != mEndpointId)
        {
            return false;
        }
        if (mFlags == CommandPathFlags::kGroupIdValid && other.mGroupId != mGroupId)
        {
            return false;
        }
        return true;
    }
    EndpointId mEndpointId = 0;
    GroupId mGroupId       = 0;
    ClusterId mClusterId   = 0;
    CommandId mCommandId   = 0;
    BitFlags<CommandPathFlags> mFlags;
};
} // namespace app
} // namespace chip
