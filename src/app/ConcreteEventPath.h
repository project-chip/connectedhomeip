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

#include <app/ConcreteClusterPath.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {

/**
 * A representation of a concrete event path.
 */
struct ConcreteEventPath : public ConcreteClusterPath
{
    ConcreteEventPath(EndpointId aEndpointId, ClusterId aClusterId, EventId aEventId) :
        ConcreteClusterPath(aEndpointId, aClusterId), mEventId(aEventId)
    {}

    ConcreteEventPath() {}

    ConcreteEventPath(const ConcreteEventPath & aOther) = default;
    ConcreteEventPath & operator=(const ConcreteEventPath & aOther) = default;

    bool operator==(const ConcreteEventPath & aOther) const
    {
        return ConcreteClusterPath::operator==(aOther) && (mEventId == aOther.mEventId);
    }

    bool operator!=(const ConcreteEventPath & aOther) const { return !(*this == aOther); }

    bool operator<(const ConcreteEventPath & path) const
    {
        return (mEndpointId < path.mEndpointId) || ((mEndpointId == path.mEndpointId) && (mClusterId < path.mClusterId)) ||
            ((mEndpointId == path.mEndpointId) && (mClusterId == path.mClusterId) && (mEventId < path.mEventId));
    }

    EventId mEventId = 0;
};
} // namespace app
} // namespace chip
