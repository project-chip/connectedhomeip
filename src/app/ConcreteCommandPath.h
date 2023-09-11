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
 * A representation of a concrete invoke path.
 */
struct ConcreteCommandPath : public ConcreteClusterPath
{
    ConcreteCommandPath(EndpointId aEndpointId, ClusterId aClusterId, CommandId aCommandId) :
        ConcreteClusterPath(aEndpointId, aClusterId), mCommandId(aCommandId)
    {}

    bool operator==(const ConcreteCommandPath & aOther) const
    {
        return ConcreteClusterPath::operator==(aOther) && (mCommandId == aOther.mCommandId);
    }

    bool operator!=(const ConcreteCommandPath & aOther) const { return !(*this == aOther); }

    CommandId mCommandId = 0;
};
} // namespace app
} // namespace chip
