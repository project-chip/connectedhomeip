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

/**
 * A representation of a concrete cluster path.  This identifies a specific
 * cluster instance.
 */
struct ConcreteClusterPath
{
    ConcreteClusterPath(EndpointId aEndpointId, ClusterId aClusterId) : mEndpointId(aEndpointId), mClusterId(aClusterId) {}

    ConcreteClusterPath() {}

    ConcreteClusterPath(const ConcreteClusterPath & aOther)             = default;
    ConcreteClusterPath & operator=(const ConcreteClusterPath & aOther) = default;

    bool IsValidConcreteClusterPath() const { return !(mEndpointId == kInvalidEndpointId || mClusterId == kInvalidClusterId); }

    bool HasValidIds() const { return IsValidEndpointId(mEndpointId) && IsValidClusterId(mClusterId); }

    bool operator==(const ConcreteClusterPath & aOther) const
    {
        return mEndpointId == aOther.mEndpointId && mClusterId == aOther.mClusterId;
    }

    bool operator!=(const ConcreteClusterPath & aOther) const { return !(*this == aOther); }

    EndpointId mEndpointId = 0;
    // Note: not all subclasses of ConcreteClusterPath need mExpanded, but due
    // to alignment requirements it's "free" in the sense of not needing more
    // memory to put it here.  But we don't initialize it, because that
    // increases codesize for the non-consumers.
    bool mExpanded; // NOTE: in between larger members, NOT initialized (see above)
    ClusterId mClusterId = 0;
};

} // namespace app
} // namespace chip
