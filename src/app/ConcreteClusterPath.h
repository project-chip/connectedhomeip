/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    ConcreteClusterPath(const ConcreteClusterPath & aOther) = default;
    ConcreteClusterPath & operator=(const ConcreteClusterPath & aOther) = default;

    bool IsValidConcreteClusterPath() const { return !(mEndpointId == kInvalidEndpointId || mClusterId == kInvalidClusterId); }

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
    bool mExpanded; // NOTE: in between larger members
    ClusterId mClusterId = 0;
};

} // namespace app
} // namespace chip
