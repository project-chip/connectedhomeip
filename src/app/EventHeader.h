/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ConcreteEventPath.h"
#include "EventLoggingTypes.h"
#include <app/util/basic-types.h>

namespace chip {
namespace app {
struct EventHeader
{
    ConcreteEventPath mPath;
    EventNumber mEventNumber     = 0;
    PriorityLevel mPriorityLevel = PriorityLevel::Invalid;
    Timestamp mTimestamp;
};
} // namespace app
} // namespace chip
