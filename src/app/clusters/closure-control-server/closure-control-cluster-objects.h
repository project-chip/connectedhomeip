/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 * Structure represents the overall state of a closure control cluster derivation instance.
 */
struct GenericOverallState : public Structs::OverallStateStruct::Type
{
    GenericOverallState(Optional<DataModel::Nullable<PositioningEnum>> positioningValue       = NullOptional,
                        Optional<DataModel::Nullable<LatchingEnum>> latchingValue             = NullOptional,
                        Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>> speedValue = NullOptional,
                        Optional<DataModel::Nullable<uint32_t>> extraInfoValue                = NullOptional)
    {
        Set(positioningValue, latchingValue, speedValue, extraInfoValue);
    }

    GenericOverallState(const GenericOverallState & overallState) { *this = overallState; }

    GenericOverallState & operator=(const GenericOverallState & overallState)
    {
        Set(overallState.positioning, overallState.latching, overallState.speed, overallState.extraInfo);
        return *this;
    }

    void Set(Optional<DataModel::Nullable<PositioningEnum>> positioningValue       = NullOptional,
             Optional<DataModel::Nullable<LatchingEnum>> latchingValue             = NullOptional,
             Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>> speedValue = NullOptional,
             Optional<DataModel::Nullable<uint32_t>> extraInfoValue                = NullOptional)
    {
        positioning = positioningValue;
        latching    = latchingValue;
        speed       = speedValue;
        extraInfo   = extraInfoValue;
    }

    bool operator==(const Structs::OverallStateStruct::Type & rhs) const
    {
        return positioning == rhs.positioning && latching == rhs.latching && speed == rhs.speed && extraInfo == rhs.extraInfo;
    }
};

/**
 * Structure represents the overall target state of a closure control cluster derivation instance.
 */
struct GenericOverallTarget : public Structs::OverallTargetStruct::Type
{
    GenericOverallTarget(Optional<TargetPositionEnum> targetPositionValue = NullOptional,
                         Optional<TargetLatchEnum> targetLatchValue       = NullOptional,
                         Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        Set(targetPositionValue, targetLatchValue, speedValue);
    }

    GenericOverallTarget(const GenericOverallTarget & overallTarget) { *this = overallTarget; }

    GenericOverallTarget & operator=(const GenericOverallTarget & overallTarget)
    {
        Set(overallTarget.position, overallTarget.latch, overallTarget.speed);
        return *this;
    }

    void Set(Optional<TargetPositionEnum> targetPositionValue = NullOptional,
             Optional<TargetLatchEnum> targetLatchValue       = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        position = targetPositionValue;
        latch    = targetLatchValue;
        speed    = speedValue;
    }

    bool operator==(const Structs::OverallTargetStruct::Type & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed;
    }
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
