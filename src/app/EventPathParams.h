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

#include <app/ConcreteEventPath.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>

namespace chip {
namespace app {
struct EventPathParams
{
    EventPathParams(EndpointId aEndpointId, ClusterId aClusterId, EventId aEventId, bool aUrgentEvent = false) :
        mClusterId(aClusterId), mEventId(aEventId), mEndpointId(aEndpointId), mIsUrgentEvent(aUrgentEvent)
    {}
    EventPathParams() {}
    bool IsSamePath(const EventPathParams & other) const
    {
        return other.mEndpointId == mEndpointId && other.mClusterId == mClusterId && other.mEventId == mEventId;
    }

    bool HasEventWildcard() const { return HasWildcardEndpointId() || HasWildcardClusterId() || HasWildcardEventId(); }

    // For event, an event id can only be interpreted if the cluster id is known.
    bool IsValidEventPath() const { return !(HasWildcardClusterId() && !HasWildcardEventId()); }

    inline bool HasWildcardEndpointId() const { return mEndpointId == kInvalidEndpointId; }
    inline bool HasWildcardClusterId() const { return mClusterId == kInvalidClusterId; }
    inline bool HasWildcardEventId() const { return mEventId == kInvalidEventId; }
    inline void SetWildcardEndpointId() { mEndpointId = kInvalidEndpointId; }
    inline void SetWildcardClusterId() { mClusterId = kInvalidClusterId; }
    inline void SetWildcardEventId() { mEventId = kInvalidEventId; }

    bool IsEventPathSupersetOf(const ConcreteEventPath & other) const
    {
        VerifyOrReturnError(HasWildcardEndpointId() || mEndpointId == other.mEndpointId, false);
        VerifyOrReturnError(HasWildcardClusterId() || mClusterId == other.mClusterId, false);
        VerifyOrReturnError(HasWildcardEventId() || mEventId == other.mEventId, false);

        return true;
    }

    ClusterId mClusterId   = kInvalidClusterId;  // uint32
    EventId mEventId       = kInvalidEventId;    // uint32
    EndpointId mEndpointId = kInvalidEndpointId; // uint16
    bool mIsUrgentEvent    = false;              // uint8
};
} // namespace app
} // namespace chip
