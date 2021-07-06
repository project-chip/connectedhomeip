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

namespace chip {
namespace app {
struct EventPathParams
{
    EventPathParams(NodeId aNodeId, EndpointId aEndpointId, ClusterId aClusterId, EventId aEventId, bool aIsUrgent) :
        mNodeId(aNodeId), mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId), mIsUrgent(aIsUrgent)
    {}
    EventPathParams() {}
    bool IsSamePath(const EventPathParams & other) const
    {
        return other.mNodeId == mNodeId && other.mEndpointId == mEndpointId && other.mClusterId == mClusterId &&
            other.mEventId == mEventId;
    }
    NodeId mNodeId         = 0;
    EndpointId mEndpointId = 0;
    ClusterId mClusterId   = 0;
    EventId mEventId       = 0;
    bool mIsUrgent         = false;
};
} // namespace app
} // namespace chip
