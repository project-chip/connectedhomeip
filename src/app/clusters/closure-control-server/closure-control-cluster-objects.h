/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>

#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * A class which represents the overall state of an closure control cluster derivation instance.
 */
struct GenericOverallState : public Structs::OverallStateStruct::Type
{
    GenericOverallState(Optional<PositioningEnum> positioningValue = NullOptional,
                        Optional<LatchingEnum> latchingValue = NullOptional,
                        Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional,
                        Optional<uint32_t> extraInfoValue = NullOptional)
    {
        Set(positioningValue, latchingValue, speedValue, extraInfoValue);
    }

    GenericOverallState(const GenericOverallState & overallState) { *this = overallState; }

    GenericOverallState & operator=(const GenericOverallState & overallState)
    {
        Set(overallState.positioning, overallState.latching, overallState.speed, overallState.extraInfo);
        return *this;
    }

    void Set(Optional<PositioningEnum> positioningValue = NullOptional,
                Optional<LatchingEnum> latchingValue = NullOptional,
                Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional,
                Optional<uint32_t> extraInfoValue = NullOptional)
    {
        if (positioningValue.HasValue()) {
            positioning = positioningValue;
        }
        if (latchingValue.HasValue()) {
            latching = latchingValue;
        }
        if (speedValue.HasValue()) {
            speed = speedValue;
        }
        if (extraInfoValue.HasValue()) {
            extraInfo = extraInfoValue;
        }
    }

    bool IsEqual(const Structs::OverallStateStruct::Type & rhs) const
    {
        if (positioning.HasValue() != rhs.positioning.HasValue() ||
            latching.HasValue() != rhs.latching.HasValue() ||
            speed.HasValue() != rhs.speed.HasValue() ||
            extraInfo.HasValue() != rhs.extraInfo.HasValue())
        {
            return false;
        }

        if (positioning.HasValue() && positioning.Value() != rhs.positioning.Value())
        {
            return false;
        }
        if (latching.HasValue() && latching.Value() != rhs.latching.Value())
        {
            return false;
        }
        if (speed.HasValue() && speed.Value() != rhs.speed.Value())
        {
            return false;
        }
        if (extraInfo.HasValue() && extraInfo.Value() != rhs.extraInfo.Value())
        {
            return false;
        }

        return true;
    }
};

/**
 * A class which represents the overall target state of an closure control cluster derivation instance.
 */
struct GenericOverallTarget : public Structs::OverallTargetStruct::Type
{
    GenericOverallTarget(Optional<TagPositionEnum> tagPositionValue = NullOptional,
                            Optional<TagLatchEnum> tagLatchValue = NullOptional,
                            Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        Set(tagPositionValue, tagLatchValue, speedValue);
    }

    GenericOverallTarget(const GenericOverallTarget & overallTarget) { *this = overallTarget; }

    GenericOverallTarget & operator=(const GenericOverallTarget & overallTarget)
    {
        Set(overallTarget.tagPosition, overallTarget.tagLatch, overallTarget.speed);
        return *this;
    }

    void Set(Optional<TagPositionEnum> tagPositionValue = NullOptional,
                Optional<TagLatchEnum> tagLatchValue = NullOptional,
                Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        if (tagPositionValue.HasValue()) {
            tagPosition = tagPositionValue;
        }
        if (tagLatchValue.HasValue()) {
            tagLatch = tagLatchValue;
        }
        if (speedValue.HasValue()) {
            speed = speedValue;
        }
    }

    bool IsEqual(const Structs::OverallTargetStruct::Type & rhs) const
    {
        if (tagPosition.HasValue() != rhs.tagPosition.HasValue() ||
            tagLatch.HasValue() != rhs.tagLatch.HasValue() ||
            speed.HasValue() != rhs.speed.HasValue())
        {
            return false;
        }

        if (tagPosition.HasValue() && tagPosition.Value() != rhs.tagPosition.Value())
        {
            return false;
        }
        if (tagLatch.HasValue() && tagLatch.Value() != rhs.tagLatch.Value())
        {
            return false;
        }
        if (speed.HasValue() && speed.Value() != rhs.speed.Value())
        {
            return false;
        }

        return true;
    }
}; 

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
