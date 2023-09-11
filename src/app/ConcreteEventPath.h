/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
