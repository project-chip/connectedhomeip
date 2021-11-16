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
#include <app/util/basic-types.h>

namespace chip {
namespace app {

/**
 * A representation of a concrete event path.
 */
struct ConcreteEventPath
{
    ConcreteEventPath(EndpointId aEndpointId, ClusterId aClusterId, EventId aEventId) :
        mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId)
    {}

    ConcreteEventPath() {}

    ConcreteEventPath & operator=(ConcreteEventPath && other)
    {
        if (&other == this)
            return *this;

        mEndpointId = other.mEndpointId;
        mClusterId  = other.mClusterId;
        mEventId    = other.mEventId;
        return *this;
    }

    bool operator==(const ConcreteEventPath & other) const
    {
        return mEndpointId == other.mEndpointId && mClusterId == other.mClusterId && mEventId == other.mEventId;
    }

    bool IsValidEventPath() const { return !HasWildcardEventId(); }

    inline bool HasWildcardEventId() const { return mEventId == ClusterInfo::kInvalidEventId; }

    EndpointId mEndpointId = 0;
    ClusterId mClusterId   = 0;
    EventId mEventId       = 0;
};
} // namespace app
} // namespace chip
